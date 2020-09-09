
#include "cameraman.h"
#include "src/config.h"

struct CameraMan gCameraMan = {
    {},
    {0.0f, 0.0f, 0.0f},
    {5.0f, 2.0f, 0.0},
};

void cameraManUpdate(struct CameraMan* cameraMan, struct Vector3* target) {
    struct Vector3 targetPos;
    struct Vector3 offset;
    vector3Add(target, &cameraMan->targetOffset, &targetPos);

    vector3Sub(&targetPos, &cameraMan->camera.position, &offset);
    vector3Lerp(&cameraMan->velocity, &offset, 0.1f, &cameraMan->velocity);
    vector3Scale(&cameraMan->velocity, &cameraMan->velocity, 0.25f);

    vector3Scale(&cameraMan->velocity, &offset, FRAME_TIME);

    vector3Add(&cameraMan->camera.position, &offset, &cameraMan->camera.position);

    vector3Sub(target, &cameraMan->camera.position, &offset);

    offset.y = 0.0f;

    vector3Normalize(&offset, &offset);
    offset.x += 1.0f;
    vector3Normalize(&offset, &offset);

    cameraMan->camera.rotation.x = 0.0f;
    cameraMan->camera.rotation.y = offset.z;
    cameraMan->camera.rotation.z = 0.0f;
    cameraMan->camera.rotation.w = offset.x;
}