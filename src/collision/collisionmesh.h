
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
    float barycentricDenom;
    struct Vector3 dotCompare1;
    struct Vector3 dotCompare2;
};

struct CollisionEdge {
    struct CollisionFace* faces[2];
    unsigned short edgeIndex[2];
    struct Vector3* endpoints[2];
};

struct CollisionMesh {
    struct CollisionFace* faces;
    struct CollisionEdge* edges;
    struct Vector3* vertices;
    unsigned short faceCount;
    unsigned short edgeCount;
    unsigned short vertexCount;
};

void collisionFaceBaryCoord(struct CollisionFace* face, struct Vector3* in, struct Vector3* baryCoord);
void collisionFaceFromBaryCoord(struct CollisionFace* face, struct Vector3* baryCoord, struct Vector3* out);
void collisionFaceBaryDistanceToEdge(struct CollisionFace* face, struct Vector3* origin, struct Vector3* dir, struct Vector3* baryCoord);

void collisionFillDebugShape(struct CollisionMesh* target, struct Vector3* from, int fromCount);

int vertexIndexToEdgeIndex(int vertexIndex);
struct CollisionFace* collisionGetAdjacentFace(struct CollisionFace* face, int edgeIndex);

#endif