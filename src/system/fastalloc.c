
#include "fastalloc.h"
#include "assert.h"
#include "align.h"

// 64k of fast alloc memory
static char fastAllocBuffer[0x10000];
static char* currentFastAlloc;

void* fastalloc(int size) {
    if (currentFastAlloc == 0) {
        currentFastAlloc = fastAllocBuffer;
    }
    
    void* result = currentFastAlloc;
    currentFastAlloc = (char*)ALIGN_8_BYTES((int)currentFastAlloc + size);
    assert(currentFastAlloc <= fastAllocBuffer + sizeof(fastAllocBuffer));

    return result;
}

AllocState getFastAllocState() {
    return currentFastAlloc;
}

void restoreFastAllocState(AllocState state) {
    currentFastAlloc = state;
}

void resetFastAlloc() {
    currentFastAlloc = fastAllocBuffer;
}