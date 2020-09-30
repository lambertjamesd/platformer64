
#include "collisionmesh.h"
#include "src/math/vector.h"
#include "src/system/assert.h"
#include <float.h>
#include <math.h>

#define ZERO_TOLERANCE 0.00001f

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

void collisionFaceBaryDistanceToEdge(struct CollisionFace* face, struct Vector3* origin, struct Vector3* dir, struct Vector3* baryCoord) {
    struct Vector3 dotCompare0;
    struct Vector3 dotCompare1;
    struct Vector3 dotCompare2;
    struct Vector3 relativeOrigin;

    float edgeDot = vector3Dot(&face->edgeDir[0], &face->edgeDir[1]);
    float edge0MagSqr = vector3MagSqrd(&face->edgeDir[0]);
    float edge1MagSqr = vector3MagSqrd(&face->edgeDir[1]);

    dotCompare1.x = edgeDot * face->edgeDir[1].x - edge1MagSqr * face->edgeDir[0].x;
    dotCompare1.y = edgeDot * face->edgeDir[1].y - edge1MagSqr * face->edgeDir[0].y;
    dotCompare1.z = edgeDot * face->edgeDir[1].z - edge1MagSqr * face->edgeDir[0].z;

    dotCompare2.x = edgeDot * face->edgeDir[0].x - edge0MagSqr * face->edgeDir[1].x;
    dotCompare2.y = edgeDot * face->edgeDir[0].y - edge0MagSqr * face->edgeDir[1].y;
    dotCompare2.z = edgeDot * face->edgeDir[0].z - edge0MagSqr * face->edgeDir[1].z;

    vector3Add(&dotCompare1, &dotCompare2, &dotCompare0);

    float timeDenom0 = vector3Dot(dir, &dotCompare0);
    float timeDenom1 = vector3Dot(dir, &dotCompare1);
    float timeDenom2 = vector3Dot(dir, &dotCompare2);

    vector3Sub(origin, face->edges[0]->endpoints[0], &relativeOrigin);

    if (fabs(timeDenom0) < ZERO_TOLERANCE) {
        baryCoord->x = FLT_MAX;
    } else {
        baryCoord->x = (-vector3Dot(&relativeOrigin, &dotCompare0) - 1.0f / face->barycentricDenom) / timeDenom0;
    }

    if (fabs(timeDenom1) < ZERO_TOLERANCE) {
        baryCoord->y = FLT_MAX;
    } else {
        baryCoord->y = -vector3Dot(&relativeOrigin, &dotCompare1) / timeDenom1;
    }

    if (fabs(timeDenom2) < ZERO_TOLERANCE) {
        baryCoord->z = FLT_MAX;
    } else {
        baryCoord->z = -vector3Dot(&relativeOrigin, &dotCompare2) / timeDenom2;
    }
}

void collisionFaceFromBaryCoord(struct CollisionFace* face, struct Vector3* baryCoord, struct Vector3* out) {
    struct Vector3* a = face->edges[0]->endpoints[face->edgeIndices[0]];
    struct Vector3* b = face->edges[1]->endpoints[face->edgeIndices[1]];
    struct Vector3* c = face->edges[2]->endpoints[face->edgeIndices[2]];

    out->x = baryCoord->x * a->x + baryCoord->y * b->x + baryCoord->z * c->x;
    out->y = baryCoord->x * a->y + baryCoord->y * b->y + baryCoord->z * c->y;
    out->z = baryCoord->x * a->z + baryCoord->y * b->z + baryCoord->z * c->z;
}

void collisionReplaceEdges(struct CollisionEdge* replace, struct CollisionEdge* edge) {
    int i;

    for (i = 0; i < 3; ++i) {
        if (replace->faces[0]->edges[i] == replace) {
            replace->faces[0]->edges[i] = edge;
        }

        if (replace->faces[1] && replace->faces[1]->edges[i] == replace) {
            replace->faces[1]->edges[i] = edge;
        }
    }
}

void collisionJoinDuplicateVertices(struct CollisionMesh* target) {
    int firstVertexIndex;
    int secondVertexIndex;

    for (firstVertexIndex = 0; firstVertexIndex < target->vertexCount; ++firstVertexIndex) {
        for (secondVertexIndex = 0; secondVertexIndex < firstVertexIndex; ++secondVertexIndex) {
            struct Vector3* firstVertex = &target->vertices[firstVertexIndex];
            struct Vector3* secondVertex = &target->vertices[secondVertexIndex];

            if (
                firstVertex->x == secondVertex->x &&
                firstVertex->y == secondVertex->y &&
                firstVertex->z == secondVertex->z
            ) {
                int edgeIndex;

                for (edgeIndex = 0; edgeIndex < target->edgeCount; ++edgeIndex) {
                    if (target->edges[edgeIndex].endpoints[0] == secondVertex) {
                        target->edges[edgeIndex].endpoints[0] = firstVertex;
                    }

                    if (target->edges[edgeIndex].endpoints[1] == secondVertex) {
                        target->edges[edgeIndex].endpoints[1] = firstVertex;
                    }
                }
            }
        }
    }
}

void collisionJoinAdjacentEdges(struct CollisionMesh* target) {
    int firstEdgeIndex;
    int secondEdgeIndex;

    collisionJoinDuplicateVertices(target);

    for (firstEdgeIndex = 0; firstEdgeIndex < target->edgeCount; ++firstEdgeIndex) {
        for (secondEdgeIndex = 0; secondEdgeIndex < firstEdgeIndex; ++secondEdgeIndex) {
            struct CollisionEdge* firstEdge = &target->edges[firstEdgeIndex];
            struct CollisionEdge* secondEdge = &target->edges[secondEdgeIndex];
            struct Vector3* firstA = firstEdge->endpoints[0];
            struct Vector3* firstB = firstEdge->endpoints[1];

            struct Vector3* secondA = secondEdge->endpoints[0];
            struct Vector3* secondB = secondEdge->endpoints[1];

            struct Vector3* tmp;

            if (firstA > firstB) {
                tmp = firstA;
                firstA = firstB;
                firstB = tmp;
            }

            if (secondA > secondB) {
                tmp = secondA;
                secondA = secondB;
                secondB = tmp;
            }

            if (firstA && firstB && firstA == secondA && firstB == secondB) {
                firstEdge->faces[1] = secondEdge->faces[0];
                firstEdge->edgeIndex[1] = secondEdge->edgeIndex[0];
                firstEdge->faces[1]->edges[firstEdge->edgeIndex[1]] = firstEdge;
                secondEdge->endpoints[0] = NULL;
                secondEdge->endpoints[1] = NULL;
            }
        }
    }

    // secondEdgeIndex = 0;
    // for (firstEdgeIndex = 0; firstEdgeIndex < target->edgeCount; ++firstEdgeIndex) {
    //     struct CollisionEdge* firstEdge = &target->edges[firstEdgeIndex];

    //     if (firstEdge->endpoints[0] && firstEdge->endpoints[1]) {
    //         if (firstEdgeIndex != secondEdgeIndex) {
    //             collisionReplaceEdges(firstEdge, &target->edges[secondEdgeIndex]);
    //             target->edges[secondEdgeIndex] = *firstEdge;
    //         }

    //         ++secondEdgeIndex;
    //     }
    // }

    // target->edgeCount = secondEdgeIndex;
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

        target->vertices = from;
        target->vertexCount = fromCount;
    }

    collisionJoinAdjacentEdges(target);
}

int vertexIndexToEdgeIndex(int vertexIndex) {
    if (vertexIndex >= 2) {
        return 0;
    } else {
        return vertexIndex + 1;
    }
}

struct CollisionFace* collisionGetAdjacentFace(struct CollisionFace* face, int edgeIndex) {
    return face->edges[edgeIndex]->faces[1 - face->edgeIndices[edgeIndex]];
}