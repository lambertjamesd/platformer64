
#include "player.h"
#include "controller.h"
#include "cameraman.h"
#include "src/scene/scene.h"
#include "src/collision/meshcapsulecollision.h"
#include "src/system/time.h"

void updatePlayerFreefall(struct Player* player);
void updatePlayerJump(struct Player* player);

#define JUMP_IMPULSE        12.0f
#define JUMP_ACCEL          20.0f
#define DOWNWARD_GRAVITY    40.0f
#define JUMP_HOLD_TIME      0.5f

struct Player gPlayer = {
  updatePlayerFreefall,
  {0.0f, 10.0f, 0.0f},
};

void updatePlayerWalk(struct Player* player) {
    if (contGetButtonDown(A_BUTTON, 0)) {
        player->velocity.y += JUMP_IMPULSE;
        player->currentState = updatePlayerJump;
    }

    struct Vector3 horzVel = {0.0f, 0.0f, 0.0f};

    if (gControllerState[0].stick_y != 0) {
        struct Vector3 offset;
        quatMultVector(&gCameraMan.camera.rotation, &gForward, &offset);
        offset.y = 0.0f;
        vector3Normalize(&offset, &offset);

        vector3Scale(&offset, &horzVel, (float)gControllerState[0].stick_y * -0.1f);
    }

    if (gControllerState[0].stick_x != 0) {
        struct Vector3 offset;
        quatMultVector(&gCameraMan.camera.rotation, &gRight, &offset);
        offset.y = 0.0f;
        vector3Normalize(&offset, &offset);

        vector3Scale(&offset, &offset, (float)gControllerState[0].stick_x * 0.1f);
        vector3Add(&horzVel, &offset, &horzVel);
    }

    player->velocity.x = horzVel.x;
    player->velocity.z = horzVel.z;

    player->position.x += player->velocity.x * gTimeDelta;
    player->position.z += player->velocity.z * gTimeDelta;
}

void updatePlayerFreefall(struct Player* player) {
    player->velocity.y -= DOWNWARD_GRAVITY * gTimeDelta;

    struct Vector3 moveOffset;

    struct CollisionCapsule capsule;
    vector3Scale(&player->velocity, &moveOffset, gTimeDelta);
    vector3Add(&player->position, &moveOffset, &player->position);

    capsule.center = player->position;
    capsule.center.y += 0.5f;
    capsule.innerHeight = 0.5f;
    capsule.radius = 0.25f;

    if (meshCapsuleContactPoint(gScene.staticCollision, &capsule, &player->lastContact)) {
        capsule.center.x += player->lastContact.normal.x * player->lastContact.overlapDistance;
        capsule.center.y += player->lastContact.normal.y * player->lastContact.overlapDistance;
        capsule.center.z += player->lastContact.normal.z * player->lastContact.overlapDistance;

        struct Vector3 projectedVel;
        vector3Project(&player->velocity, &player->lastContact.normal, &projectedVel);
        vector3Sub(&player->velocity, &projectedVel, &player->velocity);

        player->position = capsule.center;
        player->position.y -= 0.5f;

        player->currentState = updatePlayerWalk;
    }
}

void updatePlayerJump(struct Player* player) {
    if (contGetButton(A_BUTTON, 0)) {
        player->velocity.y += JUMP_ACCEL * gTimeDelta;
    } else {
        player->currentState = updatePlayerFreefall;
    }

    updatePlayerFreefall(player);
}

void updatePlayer(struct Player* player) {
    player->currentState(player);
}