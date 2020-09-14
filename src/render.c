
#include <ultra64.h>
#include <string.h>

#include "nu64sys.h"
#include "render.h"
#include "debugger/serial.h"
#include "src/math/quaternion.h"
#include "src/render/sceneview.h"
#include "src/level/test/header.h"
#include "src/system/align.h"
#include "src/player/cameraman.h"
#include "src/player/geo/header.h"
#include "src/system/assert.h"
#include "src/collision/collisionmesh.h"
#include "src/collision/meshcapsulecollision.h"

#define SP_UCODE_SIZE		4096
#define SP_UCODE_DATA_SIZE	2048

#define	SP_DRAM_STACK_SIZE8	(1024)
#define	SP_DRAM_STACK_SIZE64	(SP_DRAM_STACK_SIZE8 >> 3)

#define RDP_OUTPUT_LEN	(4096*16)

u64 dram_stack[SP_DRAM_STACK_SIZE64];
u64 rdp_output[RDP_OUTPUT_LEN];

#define GLOBAL_DL_SIZE      1024

Gfx globalDL[GLOBAL_DL_SIZE];

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

static Vtx tex_vtx[] =  {
    { -64,  64, -5, 0, (0 << 6), (0 << 6), 0xff, 0xff, 0xff, 0xff},
    {  64,  64, -5, 0, (4 << 6), (0 << 6), 0xff, 0xff, 0xff, 0xff},
    {  64, -64, -5, 0, (4 << 6), (4 << 6), 0xff, 0xff, 0xff, 0xff},
    { -64, -64, -5, 0, (0 << 6), (4 << 6), 0xff, 0xff, 0xff, 0xff},
};

static Vp vp = {
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* scale */
	SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,	/* translate */
};

static float theta;
Mtx projection;
Mtx modeling;
Mtx worldScale;
Mtx playerMtx;
u16 perspectiveCorrect;

struct Vector3 target = {0.0f, 0.0f, 0.0f};

extern u16 zbuffer[];

struct CollisionFace gDebugFaces[10];
struct CollisionEdge gDebugEdges[30];
struct CollisionMesh gDebugMesh = {
    gDebugFaces,
    gDebugEdges,
    0,
    0,
};

struct Vector3 gDebugMeshData[] = {
    {-1.0f, 0.5f, -1.0f},
    {1.0f, 0.5f, 1.0f},
    {1.0f, 0.5f, -1.0f},

    {1.0f, 0.5f, 1.0f},
    {-1.0f, 0.5f, -1.0f},
    {-1.0f, 0.0f, 1.0f},
};

Vtx gDebugVertices[32];

void checkIsTouching(struct CollisionMesh* mesh, struct Vector3* pos, float radius, float innerHeight, char* isTouchingOut) {
    int i;

    for (i = 0; i < mesh->faceCount; ++i) {
        struct Vector3 baryCoords;
        struct Vector3 projectedPoint;
        planeProjectOnto(&mesh->faces[i].plane, pos, &projectedPoint);
        collisionFaceBaryCoord(&mesh->faces[i], &projectedPoint, &baryCoords);
        isTouchingOut[i] = baryCoords.x >= 0 && baryCoords.y >= 0 &&  baryCoords.z >= 0;
    }
}

Gfx* renderDebugCollision(Gfx* dl, struct CollisionMesh* mesh, char* isTouching) {
    int vertexIndex = 0;

    gDPPipeSync(dl++);
    gDPSetCycleType(dl++, G_CYC_1CYCLE);
    gDPSetRenderMode(dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineMode(dl++, G_CC_ADDRGB, G_CC_PRIMITIVE);
    gSPSetGeometryMode(dl++, G_SHADE | G_SHADING_SMOOTH);

    int i;
    int edge;
    int pass;

    for (pass = 0; pass < 2; ++pass) {
        if (pass == 0) {
            gDPSetPrimColor(dl++, 0, 0, 0xff, 0, 0, 0xff);
        } else {
            gDPSetPrimColor(dl++, 0, 0, 0, 0xff, 0, 0xff);
        }

        gDPPipeSync(dl++);

        for (i = 0; i < mesh->faceCount; ++i) {
            if (isTouching[i] == pass) {
                struct CollisionFace* face = &mesh->faces[i];
                gSPVertex(dl++, &gDebugVertices[vertexIndex], 3, 0);
                for (edge = 0; edge < 3; ++edge) {
                    struct Vector3* point = face->edges[edge]->endpoints[0];
                    gDebugVertices[vertexIndex].v.ob[0] = (short)(point->x * 256.0f);
                    gDebugVertices[vertexIndex].v.ob[1] = (short)(point->y * 256.0f);
                    gDebugVertices[vertexIndex].v.ob[2] = (short)(point->z * 256.0f);
                    gDebugVertices[vertexIndex].v.cn[3] = 0xff;
                    ++vertexIndex;
                }
                gSP1Triangle(dl++, 0, 1, 2, 0);
            }
        }
    }

    osWritebackDCache(gDebugVertices, sizeof(gDebugVertices));

    return dl;
}

Gfx* clear(u16* cfb) {
    guPerspective(&projection, &perspectiveCorrect, 70.0f, 4.0f / 3.0f, 1.0f, 128.0f, 1.0f);
    Mtx cameraView;
    Mtx rotate;

    struct Quaternion qRotate;
    struct Vector3 offset;

    float fMtx[4][4];
    char isTouching[32];

    cameraCalcView(&gCameraMan.camera, fMtx);
    guMtxF2L(fMtx, &cameraView);

    quatAxisAngle(&gRight, theta, &qRotate);
    quatToMatrix(&qRotate, fMtx);
    guMtxF2L(fMtx, &rotate);
    guMtxIdent(&rotate);

    guMtxCatL(&rotate, &cameraView, &modeling);


    guScale(&worldScale, 1.0f / 256.0f, 1.0f / 256.0f, 1.0f / 256.0f);

    struct Vector3 movedTarget;
    struct ContactPoint contactPoint;
    struct CollisionCapsule capsule;
    struct Vector3 baryCoords;

    capsule.center = target;
    capsule.center.y += 0.5f;
    capsule.innerHeight = 0.5f;
    capsule.radius = 0.25f;

    int index;

    for (index = 0; index < gDebugMesh.faceCount; ++index) {
        if (meshFaceCapsuleContactPoint(&gDebugMesh.faces[index], &capsule, &contactPoint, &baryCoords)) {
            capsule.center.x += contactPoint.normal.x * contactPoint.overlapDistance;
            capsule.center.y += contactPoint.normal.y * contactPoint.overlapDistance;
            capsule.center.z += contactPoint.normal.z * contactPoint.overlapDistance;
        }
    }

    guTranslate(&rotate, capsule.center.x, capsule.center.y - 0.5f, capsule.center.z);
    guMtxCatL(&worldScale, &rotate, &playerMtx);

    osWritebackDCache(&projection, sizeof(Mtx));
    osWritebackDCache(&modeling, sizeof(Mtx));
    osWritebackDCache(&worldScale, sizeof(Mtx));
    osWritebackDCache(&playerMtx, sizeof(Mtx));

    u16* color = cfb;
    u16* zbuf = (u16*)ALIGN_64_BYTES((u32)zbuffer);

    theta += 0.01f;

    Gfx* dl = globalDL;
    gSPSegment(dl++, 0, 0x0);
    gSPSegment(dl++, 8, 0x0);
    gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);

    gDPSetCycleType(dl++, G_CYC_FILL);

    gDPSetDepthImage(dl++, zbuf);
    gDPSetColorImage(dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, zbuf);
    gDPSetFillColor(dl++, GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0));
    gDPPipeSync(dl++);
    gDPFillRectangle(dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	gDPPipeSync(dl++);

    gDPSetColorImage(dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, cfb);
    gDPSetFillColor(dl++, GPACK_RGBA5551(255,255,1,1) << 16 | 
		     GPACK_RGBA5551(255,255,1,1));
    gDPPipeSync(dl++);
    gDPFillRectangle(dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(dl++);

    gDPSetDepthSource(dl++, G_ZS_PIXEL);
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
    gDPSetRenderMode(dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF);
    gDPSetBlendMask(dl++, 0xff);
    gDPSetColorDither(dl++, G_CD_DISABLE);
    
    gDPPipeSync(dl++);

    gSPViewport(dl++, &vp);
    gSPClearGeometryMode(dl++, G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_CULL_FRONT | G_FOG | G_LIGHTING | G_SHADE);
    gSPSetGeometryMode(dl++, G_ZBUFFER | G_SHADING_SMOOTH | G_CULL_BACK);
    gSPTexture(dl++, 0, 0, 0, 0, G_OFF);

    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&projection), G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&modeling), G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPPerspNormalize(dl++, perspectiveCorrect);
    gDPPipeSync(dl++);

    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&worldScale), G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_PUSH);
    gSPDisplayList(dl++, OS_K0_TO_PHYSICAL(test_TestLayout_mesh));
    // gSPDisplayList(dl++, OS_K0_TO_PHYSICAL(test_CollisionTest_dl));

    checkIsTouching(&gDebugMesh, &target, 0.25f, 0.5f, isTouching);
    dl = renderDebugCollision(dl, &gDebugMesh, isTouching);

    gSPPopMatrix(dl++, G_MTX_MODELVIEW);

    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&playerMtx), G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_PUSH);
    gSPDisplayList(dl++, geo_Player_mesh);
    gSPPopMatrix(dl++, G_MTX_MODELVIEW);

	gDPFullSync(dl++);
	gSPEndDisplayList(dl++);

    assert(dl - globalDL <= GLOBAL_DL_SIZE);

    return dl;
}

void renderScene(u16* cfb) {
    cameraManUpdate(&gCameraMan, &target);

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
    gCameraMan.camera.rotation.w = 1.0f;

    collisionFillDebugShape(&gDebugMesh, gDebugMeshData, sizeof(gDebugMeshData) / sizeof(struct Vector3));
}