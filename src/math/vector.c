
#include "vector.h"
#include "fastsqrt.h"

struct Vector3 gRight = {1.0f, 0.0f, 0.0f};
struct Vector3 gUp = {0.0f, 1.0f, 0.0f};
struct Vector3 gForward = {0.0f, 0.0f, 1.0f};

void vector3Negate(struct Vector3* in, struct Vector3* out) {
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
}

void vector3Scale(struct Vector3* in, struct Vector3* out, float scale) {
    out->x = in->x * scale;
    out->y = in->y * scale;
    out->z = in->z * scale;
}

void vector3Add(struct Vector3* a, struct Vector3* b, struct Vector3* out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;   
}

void vector3Sub(struct Vector3* a, struct Vector3* b, struct Vector3* out) {
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
}

void vector3Normalize(struct Vector3* in, struct Vector3* out) {
    float invSqrt = fastInvSqrt(in->x * in->x + in->y * in->y + in->z * in->z);
    vector3Scale(in, out, invSqrt);
}

void vector3Lerp(struct Vector3* a, struct Vector3* b, float t, struct Vector3* out) {
    float tFlip = 1.0f - t;
    out->x = a->x * tFlip + b->x * t;
    out->y = a->y * tFlip + b->y * t;
    out->z = a->z * tFlip + b->z * t;
}

float vector3Dot(struct Vector3* a, struct Vector3* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vector3MagSqrd(struct Vector3* a) {
    return a->x * a->x + a->y * a->y + a->z * a->z;
}

float vector3DistSqrd(struct Vector3* a, struct Vector3* b) {
    float x = a->x - b->x;
    float y = a->y - b->y;
    float z = a->z - b->z;

    return x * x + y * y + z * z;
}

void vector3Cross(struct Vector3* a, struct Vector3* b, struct Vector3* out) {
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
}

void vector3Project(struct Vector3* in, struct Vector3* normal, struct Vector3* out) {
    float mag = vector3Dot(in, normal);
    out->x = normal->x * mag;
    out->y = normal->y * mag;
    out->z = normal->z * mag;
}

void vector3ProjectPlane(struct Vector3* in, struct Vector3* normal, struct Vector3* out) {
    float mag = vector3Dot(in, normal);
    out->x = in->x - normal->x * mag;
    out->y = in->y - normal->y * mag;
    out->z = in->z - normal->z * mag;
}