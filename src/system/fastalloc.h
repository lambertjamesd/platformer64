
#ifndef _FAST_ALLOC_H
#define _FAST_ALLOC_H

typedef void* AllocState;

void* fastalloc(int size);
AllocState getFastAllocState();
void restoreFastAllocState(AllocState state);
void resetFastAlloc();

#endif