
#ifndef _PLAYER_H
#define _PLAYER_H

#include <ultra64.h>
#include "src/math/vector.h"

struct Player {
    struct Vector3 position;
};

extern struct Player gPlayer;

#endif