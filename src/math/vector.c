
#include "vector.h"

struct Vector3 gRight = {1.0f, 0.0f, 0.0f};
struct Vector3 gUp = {0.0f, 1.0f, 0.0f};
struct Vector3 gForward = {0.0f, 0.0f, 1.0f};

void vector3Negate(struct Vector3* in, struct Vector3* out) {
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
}