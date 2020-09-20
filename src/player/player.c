
#include "player.h"
#include "controller.h"
#include "cameraman.h"
#include "src/scene/scene.h"
#include "src/collision/meshcapsulecollision.h"

struct Player gPlayer;

void updatePlayer(struct Player* player) {
    if (contGetButtonDown(A_BUTTON, 0)) {
      player->velocity.y += 10.0f;
    }

    struct Vector3 horzVel = {0.0f, 0.0f, 0.0f};

    if (gControllerState[0].stick_y != 0) {
      struct Vector3 offset;
      quatMultVector(&gCameraMan.camera.rotation, &gForward, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &horzVel, (float)gControllerState[0].stick_y * -0.03f);
    }

    if (gControllerState[0].stick_x != 0) {
      struct Vector3 offset;
      quatMultVector(&gCameraMan.camera.rotation, &gRight, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &offset, (float)gControllerState[0].stick_x * 0.03f);
      vector3Add(&horzVel, &offset, &horzVel);
    }

    player->velocity.x = horzVel.x;
    player->velocity.z = horzVel.z;

    player->velocity.y += -9.8f / 30.0f;

    struct ContactPoint contactPoint;
    struct Vector3 moveOffset;

    struct CollisionCapsule capsule;
    vector3Scale(&gPlayer.velocity, &moveOffset, 1.0f / 30.0f);
    vector3Add(&gPlayer.position, &moveOffset, &gPlayer.position);

    capsule.center = gPlayer.position;
    capsule.center.y += 0.5f;
    capsule.innerHeight = 0.5f;
    capsule.radius = 0.25f;

    if (meshCapsuleContactPoint(gScene.staticCollision, &capsule, &contactPoint)) {
        capsule.center.x += contactPoint.normal.x * contactPoint.overlapDistance;
        capsule.center.y += contactPoint.normal.y * contactPoint.overlapDistance;
        capsule.center.z += contactPoint.normal.z * contactPoint.overlapDistance;

        struct Vector3 projectedVel;
        vector3Project(&gPlayer.velocity, &contactPoint.normal, &projectedVel);
        vector3Sub(&gPlayer.velocity, &projectedVel, &gPlayer.velocity);

        gPlayer.position = capsule.center;
        gPlayer.position.y -= 0.5f;
    }
}