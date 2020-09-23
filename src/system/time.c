
#include <ultra64.h>
#include "time.h"

float gTimeDelta = 1.0 / 30.0f;
float gInvTimeDelta = 30.0f;
OSTime gLastTime;

void timeUpdate() {
    OSTime currTime = osGetTime();

    OSTime timeDelta = currTime - gLastTime;
    gTimeDelta = (float)OS_CYCLES_TO_USEC(timeDelta) / 1000000.0f;
    gInvTimeDelta = 1.0f / gTimeDelta;

    gLastTime = currTime;
}
