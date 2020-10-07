
#include "cameraman.h"
#include "controller.h"
#include "player.h"
#include "src/collision/meshcapsulecollision.h"
#include "src/collision/meshslide.h"
#include "src/scene/scene.h"
#include "src/system/time.h"

void updatePlayerFreefall(struct Player* player);
void updatePlayerJump(struct Player* player);
void updatePlayerPause(struct Player* player);

#define JUMP_IMPULSE        12.0f
#define JUMP_ACCEL          20.0f
#define DOWNWARD_GRAVITY    40.0f
#define JUMP_HOLD_TIME      0.5f
#define FLOAT_ACCEL         80.0f

struct Player gPlayer = {
  updatePlayerPause,
  {-3.0f, 10.0f, 3.0f},
};

void playerGetSurfaceDirection(struct Player* player, struct Vector3* normal, struct Vector3* out) {
    if (gControllerState[0].stick_y != 0) {
        struct Vector3 offset;
        quatMultVector(&gCameraMan.camera.rotation, &gForward, &offset);
        vector3ProjectPlane(&offset, normal, &offset);
        vector3Normalize(&offset, &offset);

        vector3Scale(&offset, out, (float)gControllerState[0].stick_y / -127.0f);
    } else {
        out->x = 0.0f;
        out->y = 0.0f;
        out->z = 0.0f;
    }

    if (gControllerState[0].stick_x != 0) {
        struct Vector3 offset;
        quatMultVector(&gCameraMan.camera.rotation, &gRight, &offset);
        vector3ProjectPlane(&offset, normal, &offset);
        vector3Normalize(&offset, &offset);

        vector3Scale(&offset, &offset, (float)gControllerState[0].stick_x / 127.0f);
        vector3Add(out, &offset, out);
    }
}

void updatePlayerWalk(struct Player* player) {
    if (contGetButtonDown(A_BUTTON, 0)) {
        player->velocity.y += JUMP_IMPULSE;
        player->currentState = updatePlayerJump;
    }

    struct Vector3 horzVel;
    struct Vector3 dir;
    playerGetSurfaceDirection(player, &player->lastContact.normal, &horzVel);

    player->velocity.x = horzVel.x * 5.0f;
    player->velocity.z = horzVel.z * 5.0f;

    vector3ProjectPlane(&player->velocity, &player->lastContact.normal, &horzVel);

    float velocitySq = vector3MagSqrd(&horzVel);

    if (velocitySq != 0.0f) {
        vector3Scale(&horzVel, &dir, 1.0f / sqrtf(velocitySq));
        
        struct SlideResult slide = slideContactPoint(
            &player->lastContact, 
            0.25f, 
            &dir, 
            vector3Dot(&dir, &horzVel) * gTimeDelta
        );
        player->position = player->lastContact.contact;
        if (slide.type == SlideResultComplete) {
        }
    }

}

void updatePlayerFreefall(struct Player* player) {
    player->velocity.y -= DOWNWARD_GRAVITY * gTimeDelta;

    struct Vector3 moveOffset;

    struct Vector3 horzVel;
    playerGetSurfaceDirection(player, &gUp, &horzVel);

    horzVel.x = horzVel.x * 10.0f + player->velocity.x;
    horzVel.y = player->velocity.y;
    horzVel.z = horzVel.z * 10.0f + player->velocity.z;

    vector3MoveTowards(&player->velocity, &horzVel, FLOAT_ACCEL * gTimeDelta, &player->velocity);

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

void updatePlayerPause(struct Player* player) {
    if (contGetButton(B_BUTTON, 0)) {
        player->currentState = updatePlayerFreefall;
    }
}

void updatePlayer(struct Player* player) {
    player->currentState(player);
}

void initPlayer(struct Player* player) {
    player->lastContact.normal = gUp;
}