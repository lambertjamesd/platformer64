
#ifndef _COLLISION_CONTACT_POINT_H
#define _COLLISION_CONTACT_POINT_H

#include "src/math/vector.h"

struct ContactPoint {
    struct Vector3 contact;
    float overlapDistance;
    struct Vector3 normal;
};

#endif