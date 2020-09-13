
#ifndef _COLLISION_MESH_CAPSULE_COLLISION_H
#define _COLLISION_MESH_CAPSULE_COLLISION_H

#include "collisionmesh.h"
#include "collisioncapusle.h"
#include "contactpoint.h"

int meshFaceCapsuleContactPoint(struct CollisionFace* face, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint);

#endif