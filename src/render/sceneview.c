
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

void calcTransform(struct Vector3* pos, struct Quaternion* rot, float scale, float result[4][4]) {
    quatToMatrix(rot, result);
    result[0][0] *= scale;
    result[0][1] *= scale;
    result[0][2] *= scale;
    result[1][0] *= scale;
    result[1][1] *= scale;
    result[1][2] *= scale;
    result[2][0] *= scale;
    result[2][1] *= scale;
    result[2][2] *= scale;

    result[3][0] += pos->x;
    result[3][1] += pos->y;
    result[3][2] += pos->z;
}