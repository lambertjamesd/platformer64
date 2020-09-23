
#ifndef _PLAYER_PLAYER_H
#define _PLAYER_PLAYER_H

#include <ultra64.h>
#include "src/math/vector.h"
#include "src/math/quaternion.h"
#include "src/collision/contactpoint.h"

struct Player;

typedef void (*PlayerState)(struct Player* player);

struct Player {
    PlayerState currentState;
    struct Vector3 position;
    struct Vector3 velocity;
    struct ContactPoint lastContact;
};

extern struct Player gPlayer;

void updatePlayer(struct Player* player);

#endif