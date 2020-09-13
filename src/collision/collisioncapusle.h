
#ifndef _COLLISION_COLLISIOIN_CAPSULE_H
#define _COLLISION_COLLISIOIN_CAPSULE_H

#include "src/math/vector.h"

struct CollisionCapsule {
    struct Vector3 center;
    float radius;
    float innerHeight;
};

#endif