
#ifndef _PLAYER_PLAYER_H
#define _PLAYER_PLAYER_H

#include <ultra64.h>
#include "src/math/vector.h"
#include "src/math/quaternion.h"

struct Player {
    struct Vector3 position;
    struct Vector3 velocity;
};

extern struct Player gPlayer;

void updatePlayer(struct Player* player, OSContPad* controller, struct Quaternion* cameraRotation, float dt);

#endif