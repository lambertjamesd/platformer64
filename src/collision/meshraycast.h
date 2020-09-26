
#ifndef _COLLISION_MESHRAYCAST_H
#define _COLLISION_MESHRAYCAST_H

#include "src/math/plane.h"
#include "collisionmesh.h"
#include "contactpoint.h"
#include <float.h>

#define RAYCAST_NO_HIT  FLT_MAX
// used to signal if two shapes overlap all the way
#define RAYCAST_FULL_OVERLAP -FLT_MAX

float raycastPlane(struct Vector3* origin, struct Vector3* dir, struct Plane* plane);
float spherecastPlane(struct Vector3* origin, struct Vector3* dir, struct Plane* plane, float radius);
float spherecastFace(struct Vector3* origin, struct Vector3* dir, struct CollisionFace* face, float radius, struct ContactPoint* contact);
float spherecastLineOverlap(struct Vector3* origin, struct Vector3* dir, struct CollisionEdge* edge, float radius, struct ContactPoint* contact, float* edgeLerp);
float spherecastEdge(struct Vector3* origin, struct Vector3* dir, struct CollisionEdge* edge, float radius, struct ContactPoint* contact);
float spherecastPoint(struct Vector3* origin, struct Vector3* dir, struct Vector3* point, float radius, struct ContactPoint* contact);

#endif