
#include "plane.h"

void planeFromNormalPoint(struct Vector3* normal, struct Vector3* point, struct Plane* out) {
    out->a = normal->x;
    out->b = normal->y;
    out->c = normal->z;
    out->d = -(normal->x * point->x + normal->y * point->y + normal->z * point->z);
}