
#include "quaternion.h"
#include <ultra64.h>

void quatIdent(struct Quaternion* q) {
    q->x = 0.0f;
    q->y = 0.0f;
    q->z = 0.0f;
    q->w = 1.0f;
}

void quatAxisAngle(struct Vector3* axis, float angle, struct Quaternion* out) {
    float sinTheta = sinf(angle * 0.5f);
    float cosTheta = cosf(angle * 0.5f);

    out->x = axis->x * sinTheta;
    out->y = axis->y * sinTheta;
    out->z = axis->z * sinTheta;
    out->w = cosTheta;
}

void quatConjugate(struct Quaternion* in, struct Quaternion* out) {
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
    out->w = in->w;
}

void quatMultVector(struct Quaternion* q, struct Vector3* a, struct Vector3* out) {
    struct Quaternion tmp;
    struct Quaternion asQuat;
    struct Quaternion conj;
    asQuat.x = a->x;
    asQuat.y = a->y;
    asQuat.z = a->z;
    asQuat.w = 0;

    quatMultiply(q, &asQuat, &tmp);
    quatConjugate(q, &conj);
    quatMultiply(&tmp, &conj, &asQuat);
    
    out->x = asQuat.x;
    out->y = asQuat.y;
    out->z = asQuat.z;
}

void quatMultiply(struct Quaternion* a, struct Quaternion* b, struct Quaternion* out) {
    out->x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
    out->y = a->w*b->y + a->y*b->w + a->z*b->x - a->x*b->z;
    out->z = a->w*b->z + a->z*b->w + a->x*b->y - a->y*b->x;
    out->w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
}

void quatToMatrix(struct Quaternion* q, float out[4][4]) {
    float xx = q->x*q->x;
    float yy = q->y*q->y;
    float zz = q->z*q->z;

    float xy = q->x*q->y;
    float yz = q->y*q->z;
    float xz = q->x*q->z;

    float xw = q->x*q->w;
    float yw = q->y*q->w;
    float zw = q->z*q->w;

    out[0][0] = 1.0f - 2.0f * (yy + zz);
    out[0][1] = 2.0f * (xy - zw);
    out[0][2] = 2.0f * (xz + yw);
    out[0][3] = 0.0f;
    out[1][0] = 2.0f * (xy + zw);
    out[1][1] = 1.0f - 2.0f * (xx + zz);
    out[1][2] = 2.0f * (yz - xw);
    out[1][3] = 0.0f;
    out[2][0] = 2.0f * (xz - yw);
    out[2][1] = 2.0f * (yz + xw);
    out[2][2] = 1.0f - 2.0f * (xx + yy);
    out[2][3] = 0.0f;
    out[3][0] = 0.0f;
    out[3][1] = 0.0f;
    out[3][2] = 0.0f;
    out[3][3] = 1.0f;
}