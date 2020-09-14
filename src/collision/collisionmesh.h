
#ifndef _COLLISION_MESH_H
#define _COLLISION_MESH_H

#include "src/math/plane.h"
#include "src/math/vector.h"

struct CollisionEdge;
struct CollisionFace;

struct CollisionFace {
    struct CollisionEdge* edges[3];
    unsigned char edgeIndices[3];
    unsigned char unused;
    struct Plane plane;
    struct Vector3 edgeDir[2];
    float barycentricDenom;
};

struct CollisionEdge {
    struct CollisionFace* faces[2];
    unsigned short edgeIndex[2];
    struct Vector3* endpoints[2];
};

struct CollisionMesh {
    struct CollisionFace* faces;
    struct CollisionEdge* edges;
    unsigned short faceCount;
    unsigned short edgeCount;
};

void collisionFaceBaryCoord(struct CollisionFace* face, struct Vector3* in, struct Vector3* baryCoord);
void collisionFaceFromBaryCoord(struct CollisionFace* face, struct Vector3* baryCoord, struct Vector3* out);

void collisionFillDebugShape(struct CollisionMesh* target, struct Vector3* from, int fromCount);

#endif