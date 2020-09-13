
#ifndef _MATH_PLANE_H
#define _MATH_PLANE_H

#include "vector.h"

/**
 * ax + by + cz + d = 0
 */
struct Plane {
    float a, b, c, d;
};

void planeFromNormalPoint(struct Vector3* normal, struct Vector3* point, struct Plane* out);
void planeProjectOnto(struct Plane* plane, struct Vector3* point, struct Vector3* out);
float planeDistanceFromPoint(struct Plane* plane, struct Vector3* point);

#endif