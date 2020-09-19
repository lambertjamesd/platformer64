
#ifndef _PLAYER_CONTROLLER_H
#define _PLAYER_CONTROLLER_H

#include <ultra64.h>

extern OSContStatus gControllerStatus[MAXCONTROLLERS];
extern OSContPad gControllerState[MAXCONTROLLERS];
extern u16 gControllerLastButton[MAXCONTROLLERS];

extern void contInit();

extern void contReadData();

extern int contGetButton(u16 buttonMask, int index);
extern int contGetButtonDown(u16 buttonMask, int index);
extern int contGetButtonUp(u16 buttonMask, int index);

#endif