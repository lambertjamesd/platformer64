/*---------------------------------------------------------------------
  Copyright (C) 1999 Nintendo.

  File   		main.c
  Comment	   	1M FLASH TEST(ver 1.25 for flash library 7.6)
  ---------------------------------------------------------------------*/
#include <ultra64.h>
#include  <string.h>
#include "nu64sys.h"
#include "thread.h"
#include "graph.h"
#include "debugger/debugger.h"
#include "render.h"
#include "src/render/sceneview.h"
#include "src/player/cameraman.h"
#include "src/player/player.h"
#include "src/player/controller.h"

u16	*cfb_tbl[2];

#define USE_DEBUGGER  1

/*
 *  Handler
 */
extern OSMesgQueue  n_dmaMessageQ;
OSPiHandle          *handler;

static long long __align;
char gTmpBuffer[0x1000];

extern OSThread mainThread;

public	void	mainproc(void *arg)
{
  u16	trig, hold;
  int	frame;

  handler = osCartRomInit();
  frame = 0;

  cfb_tbl[1] = (u16*)OS_PHYSICAL_TO_K0(osMemSize - sizeof(u16) * SCREEN_WD * SCREEN_HT);
  cfb_tbl[0] = cfb_tbl[1] - sizeof(u16) * SCREEN_WD * SCREEN_HT;

  osViBlack(1);
  osViSwapBuffer( cfb_tbl[frame] );

  osViBlack(0);
  n_WaitMesg(retrace);

#if USE_DEBUGGER

  OSThread* threadPtr = &mainThread;
  enum GDBError err = gdbInitDebugger(handler, &n_dmaMessageQ, &threadPtr, 1);
#else
  enum GDBError err = gdbSerialInit(handler, &n_dmaMessageQ);
#endif

  contInit();
  initRenderScene();

  while(1) {
    contReadData();

    struct Quaternion qRotate;
    struct Quaternion rotateByFrame;

    if (contGetButton(L_CBUTTONS | R_CBUTTONS, 0)) {
      quatAxisAngle(&gUp, contGetButton(L_CBUTTONS, 0) ? -0.01f : 0.01f, &rotateByFrame);
      quatMultiply(&gCameraMan.camera.rotation, &rotateByFrame, &qRotate);
      gCameraMan.camera.rotation = qRotate;
    }

    updatePlayer(&gPlayer, &gCameraMan.camera.rotation, 1.0f / 30.0f);

    renderScene( cfb_tbl[frame] );
    frame ^= 1;
  }
}  
