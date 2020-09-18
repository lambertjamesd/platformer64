
#include "player.h"

struct Player gPlayer;

void updatePlayer(struct Player* player, OSContPad* controller, struct Quaternion* cameraRotation, float dt) {
    if (controller->button & A_BUTTON) {
      player->velocity.y += 10.0f;
    }

    struct Vector3 horzVel = {0.0f, 0.0f, 0.0f};

    if (controller->stick_y != 0) {
      struct Vector3 offset;
      quatMultVector(cameraRotation, &gForward, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &horzVel, (float)controller->stick_y * -0.03f);
    }

    if (controller->stick_x != 0) {
      struct Vector3 offset;
      quatMultVector(cameraRotation, &gRight, &offset);
      offset.y = 0.0f;
      vector3Normalize(&offset, &offset);

      vector3Scale(&offset, &offset, (float)controller->stick_x * 0.03f);
      vector3Add(&horzVel, &offset, &horzVel);
    }

    player->velocity.x = horzVel.x;
    player->velocity.z = horzVel.z;

    player->velocity.y += -9.8f / 30.0f;
}