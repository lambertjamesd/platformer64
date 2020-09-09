
#ifndef _SCENE_VIEW_H
#define _SCENE_VIEW_H

#include <ultra64.h>
#include "src/math/vector.h"
#include "src/math/quaternion.h"

struct Camera {
    struct Vector3 position;
    struct Quaternion rotation;
    float fov;
    float near;
    float far;
};

struct SceneView {
    struct Camera camera;
};

void sceneViewInit(struct SceneView* scene);

void cameraCalcView(struct Camera* camera, float result[4][4]);
void calcTransform(struct Vector3* pos, struct Quaternion* rot, float scale, float result[4][4]);

#endif