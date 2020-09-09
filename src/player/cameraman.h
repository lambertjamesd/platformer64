
#ifndef _CAMERA_MAN_H
#define _CAMERA_MAN_H

#include "src/render/sceneview.h"
#include "src/math/vector.h"

struct CameraMan {
    struct Camera camera;
    struct Vector3 velocity;
    struct Vector3 targetOffset;
};

void cameraManUpdate(struct CameraMan* cameraMan, struct Vector3* target);

extern struct CameraMan gCameraMan;

#endif