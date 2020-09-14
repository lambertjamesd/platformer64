
#ifndef _COLLISION_CONTACT_POINT_H
#define _COLLISION_CONTACT_POINT_H

#include "src/math/vector.h"
#include "collider.h"

struct ContactPoint {
    void* target;
    enum ColliderType type;
    struct Vector3 contact;
    float overlapDistance;
    struct Vector3 normal;
};

#endif