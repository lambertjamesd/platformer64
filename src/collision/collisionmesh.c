
#include "collisionmesh.h"
#include "src/math/vector.h"
#include "src/system/assert.h"

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
    float d20 = vector3Dot(&face->edgeDir[0], &pointOffset);
    float d21 = vector3Dot(&face->edgeDir[1], &pointOffset);

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

void collisionFillDebugShape(struct CollisionMesh* target, struct Vector3* from, int fromCount) {
    int i;
    int faceIndex = 0;

    target->edgeCount = fromCount;
    target->faceCount = fromCount / 3;

    for (i = 0; i < fromCount; i += 3) {
        struct Vector3* v0 = &from[i + 0];
        struct Vector3* v1 = &from[i + 1];
        struct Vector3* v2 = &from[i + 2];
        
        struct CollisionFace* face = &target->faces[faceIndex];

        face->edges[0] = &target->edges[i + 0];
        face->edges[1] = &target->edges[i + 1];
        face->edges[2] = &target->edges[i + 2];

        face->edgeIndices[0] = 0;
        face->edgeIndices[1] = 0;
        face->edgeIndices[2] = 0;

        vector3Sub(v1, v0, &face->edgeDir[0]);
        vector3Sub(v2, v0, &face->edgeDir[1]);

        float edgeDot = vector3Dot(&face->edgeDir[0], &face->edgeDir[1]);

        face->barycentricDenom = 1.0f / (
            vector3MagSqrd(&face->edgeDir[0]) * vector3MagSqrd(&face->edgeDir[1]) -
            edgeDot * edgeDot
        );

        struct Vector3 normal;
        vector3Cross(&face->edgeDir[0], &face->edgeDir[1], &normal);
        vector3Normalize(&normal, &normal);
        planeFromNormalPoint(&normal, v0, &face->plane);

        int edge;

        for (edge = 0; edge < 3; ++edge) { 
            face->edges[edge]->faces[0] = face;
            face->edges[edge]->faces[1] = 0;
            face->edges[edge]->edgeIndex[0] = edge;
            face->edges[edge]->edgeIndex[1] = 0;
            face->edges[edge]->endpoints[0] = &from[i + edge];
            face->edges[edge]->endpoints[1] = &from[i + (edge + 1) % 3];
        }

        faceIndex++;
    }
}