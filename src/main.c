/*---------------------------------------------------------------------
  Copyright (C) 1999 Nintendo.

  File   		main.c
  Comment	   	1M FLASH TEST(ver 1.25 for flash library 7.6)
  ---------------------------------------------------------------------*/
#include	<ultra64.h>
#include  <string.h>
#include	"nu64sys.h"
#include	"thread.h"
#include	"graph.h"
#include  "debugger/debugger.h"
#include  "render.h"
#include  "src/render/sceneview.h"
#include  "src/player/cameraman.h"

u16	*cfb_tbl[2];

#define NUM_LINES 24
#define NUM_COLS  70

#define USE_DEBUGGER  1

char textGrid[NUM_LINES][NUM_COLS + 1];
u8 nextLineIndex;

/*
 *  Controller globals
 */
static OSMesgQueue contMessageQ;
static OSMesg   dummyMessage;
static OSContStatus statusdata[MAXCONTROLLERS];
static OSContPad controllerdata[MAXCONTROLLERS];
static int      controller;
static int      lastx;
static int      lasty;
static int      lastbutton = 0;
static u32 	seed =1;

/*
 *  Handler
 */
extern OSMesgQueue  n_dmaMessageQ;
OSPiHandle          *handler;

extern struct Vector3 target;

void println(char* text)
{
  char* nextLine = textGrid[nextLineIndex];
  char* lineEnd = nextLine + NUM_COLS;

  while (*text && nextLine < lineEnd)
  {
    *nextLine = *text;
    ++nextLine;
    ++text;
  }

  while (nextLine < lineEnd)
  {
    *nextLine = ' ';
    ++nextLine;
  }
  
  if (nextLineIndex == 0)
  {
    nextLineIndex = NUM_LINES - 1;
  }
  else
  {
    --nextLineIndex;
  }
}

int     initcontroller(void)
{
  int             i;
  u8          pattern;

  osCreateMesgQueue(&contMessageQ, &dummyMessage, 1);
  osSetEventMesg(OS_EVENT_SI, &contMessageQ, (OSMesg) 0);

  osContInit(&contMessageQ, &pattern, &statusdata[0]);

  for (i = 0; i < MAXCONTROLLERS; i++) {
    if ((pattern & (1 << i)) &&
	!(statusdata[i].errno & CONT_NO_RESPONSE_ERROR)) {
      osContStartReadData(&contMessageQ);
      controller = i;
      return i;
    }
  }
  return -1;
}

static void     readControllers(void)
{
  OSContPad      *pad;

  if (osRecvMesg(&contMessageQ, &dummyMessage, OS_MESG_NOBLOCK) == 0) {
    osContGetReadData(controllerdata);
    osContStartReadData(&contMessageQ);
  }

  pad = &controllerdata[controller];

  lastbutton = pad->button;
  lastx = pad->stick_x;
  lasty = pad->stick_y;
}

u32 __gdbGetWatch();

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
  initcontroller();

#if USE_DEBUGGER

  OSThread* threadPtr = &mainThread;
  enum GDBError err = gdbInitDebugger(handler, &n_dmaMessageQ, &threadPtr, 1);
  
  if (err != GDBErrorNone) {
    sprintf(gTmpBuffer, "Error initializing debugger %d", err);
    println(gTmpBuffer);
  }
#else
  enum GDBError err = gdbSerialInit(handler, &n_dmaMessageQ);

  if (err != GDBErrorNone) {
    sprintf(gTmpBuffer, "Error initializing debugger %d", err);
    println(gTmpBuffer);
  }
#endif

  initRenderScene();
  println("Start polling");
  int healthcheck = 0;

  lastbutton = 0;

  while(1) {
    trig = lastbutton;
    readControllers();
    trig = lastbutton & (lastbutton & ~trig);
    hold = lastbutton;

    struct Quaternion qRotate;
    struct Quaternion rotateByFrame;

    if (hold & (L_CBUTTONS | R_CBUTTONS)) {
      quatAxisAngle(&gUp, (hold & L_CBUTTONS) ? -0.01f : 0.01f, &rotateByFrame);
      quatMultiply(&gCameraMan.camera.rotation, &rotateByFrame, &qRotate);
      gCameraMan.camera.rotation = qRotate;
    }

    // quatAxisAngle(&gRight, (float)lasty * 0.0001f, &rotateByFrame);
    // quatMultiply(&gCameraMan.camera.rotation, &rotateByFrame, &qRotate);

    if (lasty != 0) {
      struct Vector3 offset;
      quatMultVector(&gCameraMan.camera.rotation, &gForward, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &offset, (float)lasty * -0.001f);
      vector3Add(&target, &offset, &target);
    }

    if (lastx != 0) {
      struct Vector3 offset;
      quatMultVector(&gCameraMan.camera.rotation, &gRight, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &offset, (float)lastx * 0.001f);
      vector3Add(&target, &offset, &target);
    }

    renderScene( cfb_tbl[frame] );
    frame ^= 1;
  }
}  
