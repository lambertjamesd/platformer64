
#include "vector.h"

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