
#ifndef _COLLISION_MESHSLIDE_H
#define _COLLISION_MESHSLIDE_H

#include "collisionmesh.h"
#include "contactpoint.h"

enum SlideResultType {
    SlideResultComplete,
    SlideResultNewContact,   
    SlideResultError,   
};

struct SlideResult {
    enum SlideResultType type;
    float moveDistance;
};

struct SlideResult slideContactPoint(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance);

#endif