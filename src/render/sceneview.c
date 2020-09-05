
#include "sceneview.h"

void sceneViewInit(struct SceneView* scene) {

}

void cameraCalcView(struct Camera* camera, float result[4][4]) {
    // guTranslateF(result, 0.0f, 0.0f, -5.0f);
    // struct Vector3 offset;
    struct Quaternion invRotate;

    // quatMultVector(&invRotate, &camera->position, &offset);

    float rotate[4][4];
    float offsetMat[4][4];

    quatConjugate(&camera->rotation, &invRotate);
    quatToMatrix(&invRotate, rotate);

    guTranslateF(offsetMat, -camera->position.x, -camera->position.y, -camera->position.z);

    guMtxCatF(offsetMat, rotate, result);
}