
#include <ultra64.h>
#include "time.h"

float gTimeDelta;
OSTime gLastTime;

void timeUpdate() {
    OSTime currTime = osGetTime();

    OSTime timeDelta = currTime - gLastTime;
    gTimeDelta = (float)OS_CYCLES_TO_USEC(timeDelta) / 1000000.0f;

    gLastTime = currTime;
}
