
#ifndef _PLAYER_PLAYER_H
#define _PLAYER_PLAYER_H

#include <ultra64.h>
#include "src/math/vector.h"
#include "src/math/quaternion.h"

struct Player;

typedef void (*PlayerState)(struct Player* player);

struct Player {
    PlayerState currentState;
    struct Vector3 position;
    struct Vector3 velocity;
};

extern struct Player gPlayer;

void updatePlayer(struct Player* player);

#endif