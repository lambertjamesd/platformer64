
#include "sceneview.h"

void sceneViewInit(struct SceneView* scene) {

}

void cameraCalcView(struct Camera* camera, float result[4][4]) {
    // guTranslateF(result, 0.0f, 0.0f, -5.0f);
    struct Vector3 offset;
    struct Quaternion invRotate;

    vector3Negate(&camera->position, &offset);

    quatConjugate(&camera->rotation, &invRotate);
    quatMultVector(&invRotate, &offset, &offset);

    quatToMatrix(&invRotate, result);

    result[3][0] += offset.x;
    result[3][1] += offset.y;
    result[3][2] += offset.z;
}