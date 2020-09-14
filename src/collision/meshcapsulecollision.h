
#ifndef _COLLISION_MESH_CAPSULE_COLLISION_H
#define _COLLISION_MESH_CAPSULE_COLLISION_H

#include "collisionmesh.h"
#include "collisioncapusle.h"
#include "contactpoint.h"

enum MeshCollisionResult {
    MeshCollisionResultNone,
    MeshCollisionResultPlane,
    MeshCollisionResultTriangle,
    MeshCollisionResultLineEnd0,
    MeshCollisionResultLineEnd1,
    MeshCollisionResultLineSegment,
};

enum MeshCollisionResult meshFaceCapsuleContactPoint(struct CollisionFace* face, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint, struct Vector3* baryCoord);

#endif