
#ifndef _COLLISION_MESH_H
#define _COLLISION_MESH_H

#include "src/math/plane.h"
#include "src/math/vector.h"
#include "collider.h"

struct CollisionEdge;
struct CollisionFace;

struct CollisionFace {
    struct CollisionEdge* edges[3];
    struct Plane edgeTangents[3];
    u8 edgeIndices[3];
    struct Plane plane;
    struct Vector3 edgeDir[2];
    float barycentricDenom;
};

struct CollisionEdge {
    struct CollisionFace* faces[2];
    u16 edgeIndex[2];
    struct Vector3* endpoints[2];
};

struct CollisionMesh {
    struct CollisionEdge* edges;
    u32 edgeCount;
    struct CollisionFace* faces;
    u32 faceCount;
};

void collisionFaceBaryCoord(struct CollisionFace* face, struct Vector3* in, struct Vector3* baryCoord);
void collisionFaceFromBaryCoord(struct CollisionFace* face, struct Vector3* baryCoord, struct Vector3* out);

void collisionMoveSphereOverMesh(struct CollisionLocation* location, struct Vector3* dir);

#endif