
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

#define MAX_FACE_CHECK  8
#define MAX_EDGE_CHECK  16

struct SlideRaycastState {
    void* target;
    enum ColliderType targetType;
    struct CollisionFace* checkedFaces[MAX_FACE_CHECK];
    union {
        struct CollisionEdge* edgesToCheck[MAX_EDGE_CHECK];
        struct Vector3* pointsToCheck[MAX_EDGE_CHECK];
    };
    unsigned short checkedFaceCount;
    unsigned short edgesToCheckCount;
    unsigned short pointsToCheckCount;
};

struct SlideResult slideContactPoint(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance);

#endif