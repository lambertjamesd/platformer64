
#include "collisionmesh.h"
#include "src/math/vector.h"


// // Compute barycentric coordinates (u, v, w) for
// // point p with respect to triangle (a, b, c)
// void Barycentric(Point p, Point a, Point b, Point c, float &u, float &v, float &w)
// {
//     Vector v0 = b - a, v1 = c - a, v2 = p - a;
//     float d00 = Dot(v0, v0);
//     float d01 = Dot(v0, v1);
//     float d11 = Dot(v1, v1);
//     float d20 = Dot(v2, v0);
//     float d21 = Dot(v2, v1);
//     float denom = d00 * d11 - d01 * d01;
//     v = (d11 * d20 - d01 * d21) / denom;
//     w = (d00 * d21 - d01 * d20) / denom;
//     u = 1.0f - v - w;
// }


void collisionFaceBaryCoord(struct CollisionFace* face, struct Vector3* in, struct Vector3* baryCoord) {
    struct Vector3 pointOffset;

    vector3Sub(in, face->edges[0]->endpoints[face->edgeIndices[0]], &pointOffset);

    float d01 = vector3Dot(&face->edgeDir[0], &face->edgeDir[1]);
    float d20 = vector3Dot(face->edges[0]->endpoints[face->edgeIndices[0]], &pointOffset);
    float d21 = vector3Dot(face->edges[1]->endpoints[face->edgeIndices[1]], &pointOffset);

    baryCoord->y = (vector3Dot(&face->edgeDir[1], &face->edgeDir[1]) * d20 - d01 * d21) * face->barycentricDenom;
    baryCoord->z = (vector3Dot(&face->edgeDir[0], &face->edgeDir[0]) * d21 - d01 * d20) * face->barycentricDenom;
    baryCoord->x = 1.0f - baryCoord->y - baryCoord->z;
}

void collisionFaceFromBaryCoord(struct CollisionFace* face, struct Vector3* baryCoord, struct Vector3* out) {
    struct Vector3* a = face->edges[0]->endpoints[face->edgeIndices[0]];
    struct Vector3* b = face->edges[1]->endpoints[face->edgeIndices[1]];
    struct Vector3* c = face->edges[2]->endpoints[face->edgeIndices[2]];

    out->x = baryCoord->x * a->x + baryCoord->y * b->x + baryCoord->z * c->x;
    out->y = baryCoord->x * a->y + baryCoord->y * b->y + baryCoord->z * c->y;
    out->z = baryCoord->x * a->z + baryCoord->y * b->z + baryCoord->z * c->z;
}

void collisionMoveSphereOverMesh(struct CollisionLocation* location, struct Vector3* dir) {
    switch (location->collisionType) {
        case ColliderTypeMeshFace:
            break;
        case ColliderTypeMeshEdge:
            break;
        case ColliderTypeMeshEdgeEnd0:
            break;
        case ColliderTypeMeshEdgeEnd1:
            break;
    };
}