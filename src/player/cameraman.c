
#include "cameraman.h"
#include "src/config.h"

struct CameraMan gCameraMan = {
    {},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 2.0f, 5.0},
};

void cameraManUpdate(struct CameraMan* cameraMan, struct Vector3* target) {
    struct Vector3 offset;

    quatMultVector(&cameraMan->camera.rotation, &cameraMan->targetOffset, &offset);
    vector3Add(target, &offset, &cameraMan->camera.position);
}