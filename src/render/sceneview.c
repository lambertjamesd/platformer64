
#include "sceneview.h"

void sceneViewInit(struct SceneView* scene) {

}

void cameraCalcView(struct Camera* camera, float result[4][4]) {
    struct Vector3 offset;
    struct Quaternion invRotate;

    quatConjugate(&camera->rotation, &invRotate);
    quatMultVector(&invRotate, &camera->position, &offset);

    quatToMatrix(&invRotate, result);

    result[0][3] += offset.x;
    result[1][3] += offset.y;
    result[2][3] += offset.z;
}