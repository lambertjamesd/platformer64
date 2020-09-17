
#ifndef _COLLISION_COLLIDER_TYPE_H
#define _COLLISION_COLLIDER_TYPE_H

#include "src/math/vector.h"

enum ColliderType {
    ColliderTypeMeshFace,
    ColliderTypeMeshEdge,
    ColliderTypeMeshEdgeEnd0,
    ColliderTypeMeshEdgeEnd1,
    ColliderTypePoint,
};

enum ColliderMoveResult {
    ColliderMoveResultSameFace,
    ColliderMoveResultHitEdge,
};

#endif