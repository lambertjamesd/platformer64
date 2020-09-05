
#include <ultra64.h>
#include <string.h>

#include "nu64sys.h"
#include "render.h"
#include "debugger/serial.h"
#include "src/math/quaternion.h"
#include "src/render/sceneview.h"

#define SP_UCODE_SIZE		4096
#define SP_UCODE_DATA_SIZE	2048

#define	SP_DRAM_STACK_SIZE8	(1024)
#define	SP_DRAM_STACK_SIZE64	(SP_DRAM_STACK_SIZE8 >> 3)

#define RDP_OUTPUT_LEN	(4096*16)

u64 dram_stack[SP_DRAM_STACK_SIZE64];
u64 rdp_output[RDP_OUTPUT_LEN];

Gfx globalDL[1024];

static OSTask taskHeader = {
    M_GFXTASK,
    OS_TASK_DP_WAIT,
    rspbootTextStart,
    0, // computed later
    gspF3DEX2_fifoTextStart,
    SP_UCODE_SIZE,
    gspF3DEX2_fifoDataStart,
    SP_UCODE_DATA_SIZE,
    &dram_stack[0],
	SP_DRAM_STACK_SIZE8,
	&rdp_output[0],
	&rdp_output[0]+RDP_OUTPUT_LEN,
	NULL,			/* display list pointer (fill in later) */
	0,			/* display list size (ignored) */
	NULL,			/* yield buffer (used if yield will occur) */
	0			/* yield buffer length */
};

static Vtx vertices[] = {
    { -640, -64,  640, 0, 0, 0, 0, 0xff, 0, 0xff	},
    { 640, -64,  640, 0, 0, 0, 0, 0, 0, 0xff	},
    { 640, -64, -640, 0, 0, 0, 0, 0, 0xff, 0xff	},
    { -640, -64, -640, 0, 0, 0, 0xff, 0, 0, 0xff	},
};

static Vp vp = {
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* translate */
};

static float theta;
Mtx projection;
Mtx modeling;
u16 perspectiveCorrect;
struct Camera camera;

Gfx* clear(u16* cfb) {
    guOrtho(&projection,
		-(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
		-(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
		1.0F, 1024.0F, 1.0F);
    guPerspective(&projection, &perspectiveCorrect, 70.0f, 4.0f / 3.0f, 1.0f, 1024.0f, 1.0f);
    Mtx cameraView;
    Mtx rotate;

    struct Quaternion qRotate;
    struct Vector3 offset;

    float fMtx[4][4];

    cameraCalcView(&camera, fMtx);
    guMtxF2L(fMtx, &cameraView);

    quatAxisAngle(&gRight, theta, &qRotate);
    quatToMatrix(&qRotate, fMtx);
    guMtxF2L(fMtx, &rotate);
    guMtxIdent(&rotate);

    guMtxCatL(&rotate, &cameraView, &modeling);

    osWritebackDCache(&projection, sizeof(Mtx));
    osWritebackDCache(&modeling, sizeof(Mtx));

    theta += 0.01f;

    Gfx* dl = globalDL;
    gSPSegment(dl++, 0, 0x0);
    gDPSetCycleType(dl++, G_CYC_FILL);
    gDPSetColorImage(dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, cfb);
    gDPSetFillColor(dl++, GPACK_RGBA5551(255,1,1,1) << 16 | 
		     GPACK_RGBA5551(255,1,1,1));
    gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
    gDPFillRectangle(dl++, 0, 0, SCREEN_WD, SCREEN_HT);
    gDPPipeSync(dl++);

    gDPPipelineMode(dl++, G_PM_1PRIMITIVE);
    gDPSetCombineMode(dl++, G_CC_SHADE, G_CC_SHADE);
    gDPSetTextureLOD(dl++, G_TL_TILE);
    gDPSetTextureLUT(dl++, G_TT_NONE);
    gDPSetTextureDetail(dl++, G_TD_CLAMP);
    gDPSetTexturePersp(dl++, G_TP_PERSP);
    gDPSetTextureFilter(dl++, G_TF_BILERP);
    gDPSetTextureConvert(dl++, G_TC_FILT);
    gDPSetCycleType(dl++, G_CYC_1CYCLE);
    gDPSetCombineKey(dl++, G_CK_NONE);
    gDPSetAlphaCompare(dl++, G_AC_NONE);
    gDPSetRenderMode(dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetBlendMask(dl++, 0xff);
    gDPSetColorDither(dl++, G_CD_DISABLE);
    
    gDPPipeSync(dl++);

    gSPViewport(dl++, &vp);
    gSPClearGeometryMode(dl++, G_ZBUFFER | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_CULL_FRONT | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADE);
    gSPSetGeometryMode(dl++, G_SHADING_SMOOTH);
    gSPTexture(dl++, 0, 0, 0, 0, G_OFF);

    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&modeling), G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPPerspNormalize(dl++, perspectiveCorrect);
    gDPPipeSync(dl++);

    gDPSetCycleType(dl++, G_CYC_1CYCLE);
    gDPSetRenderMode(dl++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
    gSPSetGeometryMode(dl++, G_SHADE | G_SHADING_SMOOTH);

    gSPVertex(dl++, &vertices, 4, 0);
    gSP1Triangle(dl++, 0, 1, 2, 0);
    gSP1Triangle(dl++, 2, 3, 0, 0);
    // gSP2Triangles(dl++, 0, 1, 2, 0, 2, 3, 0, 0);

	gDPFullSync(dl++);
	gSPEndDisplayList(dl++);
    return dl;
}

void renderScene(u16* cfb) {
    OSMesg dummyMessage;
    OSTask *task = &taskHeader;
    task->t.ucode_boot = rspbootTextStart;
    task->t.ucode_boot_size = (int)rspbootTextEnd - (int)rspbootTextStart;

    Gfx* end = clear(cfb);

    task->t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
    task->t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart;

    task->t.data_ptr = (u64*)globalDL;
    task->t.data_size = (end - globalDL) * sizeof(Gfx);

	osWritebackDCache(&globalDL, sizeof(globalDL));
	osSpTaskStart(task);
    
	(void)osRecvMesg(&n_rdpMessageQ, &dummyMessage, OS_MESG_BLOCK);

    osViSwapBuffer(cfb);
    
	if (MQ_IS_FULL(&n_retraceMessageQ)) {
	    (void)osRecvMesg(&n_retraceMessageQ, &dummyMessage, OS_MESG_BLOCK);
    }
	(void)osRecvMesg(&n_retraceMessageQ, &dummyMessage, OS_MESG_BLOCK);
}

void initRenderScene() {
    camera.rotation.w = 1.0f;
}