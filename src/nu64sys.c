
#include	<ultra64.h>
#include	<ramrom.h>
#include	"nu64sys.h"
#include	"thread.h"

/*
 *		BOOT section
 */
public	u64		bootStack[STACKSIZE/8];

/*
 *		IDLE thread
 */
private	void		idle(void *);
private	OSThread	idleThread;
private	u64		idleThreadStack[STACKSIZE/8];

/*
 *		MAIN thread
 */
extern	void		mainproc(void *);
OSThread	mainThread;
private	u64		mainThreadStack[STACKSIZE/8];

/*
 *		RMON thread
 */
#ifdef	RMON
private	OSThread	rmonThread;
private	u64		rmonThreadStack[RMON_STACKSIZE/8];
#endif

/*
 *		Pi message queue
 */
#define	NUM_PI_MSGS     8
private	OSMesg		PiMessages[NUM_PI_MSGS];
private	OSMesgQueue	PiMessageQ;

/*
 *		Si message queue
 */
public  OSMesgQueue	n_siMessageQ;
private OSMesg		n_siMessage;


/*
 *		Other message queues
 */
public	OSMesgQueue	n_dmaMessageQ,	n_rdpMessageQ;
public	OSMesgQueue	n_rspMessageQ,	n_retraceMessageQ;
private	OSMesg		dmaMessageBuf,	rdpMessageBuf;
private	OSMesg		rspMessageBuf,	retraceMessageBuf;

/*
 *		Cartridge message queues
 */
public	OSMesgQueue	cartMessageQ;
private	OSMesg		cartMessageBuf;


extern  u16   zbuffer[];


/*---------------------------------------------------------------------*
 *	IDLE THREAD (Pri.= 0)
 *---------------------------------------------------------------------*/
private	void	idle(void *arg)
{
  osCreateViManager(OS_PRIORITY_VIMGR);

#if	SCREEN_LOW
  osViSetMode(&osViModeTable[OS_VI_NTSC_LPN1]);
#else
  osViSetMode(&osViModeTable[OS_VI_NTSC_HPF1]);
#endif
  
  /*
   *		Initialize Pi manager
   */
  osCreatePiManager((OSPri)OS_PRIORITY_PIMGR,
		    &PiMessageQ, PiMessages, NUM_PI_MSGS);
  
  /*
   *		Setup message queue
   */
  osCreateMesgQueue(&n_dmaMessageQ, &dmaMessageBuf, 1);		/* DMA */

  osCreateMesgQueue(&n_rspMessageQ, &rspMessageBuf, 1);
  osSetEventMesg(OS_EVENT_SP , &n_rspMessageQ, NULL);		/* RSP */

  osCreateMesgQueue(&n_rdpMessageQ, &rdpMessageBuf, 1);
  osSetEventMesg(OS_EVENT_DP, &n_rdpMessageQ, NULL);		/* RDP */
  
  osCreateMesgQueue(&n_retraceMessageQ, &retraceMessageBuf, 1);
  osViSetEvent(&n_retraceMessageQ, NULL, 1);			/* retrace */
  
  osCreateMesgQueue(&n_siMessageQ, &n_siMessage, 1);
  osSetEventMesg(OS_EVENT_SI, &n_siMessageQ, (OSMesg)1);	/* SI */

  osCreateMesgQueue(&cartMessageQ, &cartMessageBuf, 1);
  osSetEventMesg(OS_EVENT_CART, &cartMessageQ, (OSMesg)1);
    

  /*
   *		Create & start MAINPROC thread
   */  
  osCreateThread(&mainThread, TID_MAINPROC, mainproc, (void *)0,
		 (void *)(mainThreadStack+STACKSIZE/8), 10);
  
  osStartThread(&mainThread);
  
  /*
   *		Now idling
   */
  osSetThreadPri(0, 0);
    while(1){
    }
}

/*---------------------------------------------------------------------*
 *	BOOT PART
 *---------------------------------------------------------------------*/
public void boot(void)
{  
  /*
   *		Initialize OS
   */

  osInitialize();
    
  /*
   *		Create idle thread & start it
   */
  osCreateThread(&idleThread, TID_IDLE, idle, (void*)0,
		 idleThreadStack+STACKSIZE/8, 10);
  osStartThread(&idleThread);
}

