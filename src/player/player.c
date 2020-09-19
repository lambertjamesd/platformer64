
#include "player.h"
#include "controller.h"

struct Player gPlayer;

void updatePlayer(struct Player* player, struct Quaternion* cameraRotation, float dt) {
    if (contGetButtonDown(A_BUTTON, 0)) {
      player->velocity.y += 10.0f;
    }

    struct Vector3 horzVel = {0.0f, 0.0f, 0.0f};

    if (gControllerState[0].stick_y != 0) {
      struct Vector3 offset;
      quatMultVector(cameraRotation, &gForward, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &horzVel, (float)gControllerState[0].stick_y * -0.03f);
    }

    if (gControllerState[0].stick_x != 0) {
      struct Vector3 offset;
      quatMultVector(cameraRotation, &gRight, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &offset, (float)gControllerState[0].stick_x * 0.03f);
      vector3Add(&horzVel, &offset, &horzVel);
    }

    player->velocity.x = horzVel.x;
    player->velocity.z = horzVel.z;

    player->velocity.y += -9.8f / 30.0f;
}