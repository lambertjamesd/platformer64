
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
    baryCoord->y = vector3Dot(&pointOffset, &face->dotCompare1) * face->barycentricDenom;
    baryCoord->z = vector3Dot(&pointOffset, &face->dotCompare2) * face->barycentricDenom;
    baryCoord->x = 1.0f - baryCoord->y - baryCoord->z;
}

void collisionFaceBaryDistanceToEdge(struct CollisionFace* face, struct Vector3* origin, struct Vector3* dir, struct Vector3* baryCoord) {
    struct Vector3 dotCompare0;
    struct Vector3 relativeOrigin;

    vector3Add(&face->dotCompare1, &face->dotCompare2, &dotCompare0);

    float timeDenom1 = vector3Dot(dir, &face->dotCompare1);
    float timeDenom2 = vector3Dot(dir, &face->dotCompare2);
    float timeDenom0 = timeDenom1 + timeDenom2;

    vector3Sub(origin, face->edges[0]->endpoints[face->edgeIndices[0]], &relativeOrigin);

    if (timeDenom0 < ZERO_TOLERANCE) {
        baryCoord->x = FLT_MAX;
    } else {
        baryCoord->x = (1.0f / face->barycentricDenom - vector3Dot(&relativeOrigin, &dotCompare0)) / timeDenom0;
    }

    if (timeDenom1 > -ZERO_TOLERANCE) {
        baryCoord->y = FLT_MAX;
    } else {
        baryCoord->y = -vector3Dot(&relativeOrigin, &face->dotCompare1) / timeDenom1;
    }

    if (timeDenom2 > -ZERO_TOLERANCE) {
        baryCoord->z = FLT_MAX;
    } else {
        baryCoord->z = -vector3Dot(&relativeOrigin, &face->dotCompare2) / timeDenom2;
    }
}

float collisionFaceBaryDir(struct CollisionFace* face, struct Vector3* dir, struct Vector3* baryCoord) {
    baryCoord->y = vector3Dot(dir, &face->dotCompare1);
    baryCoord->z = vector3Dot(dir, &face->dotCompare2);
    baryCoord->x = -(baryCoord->y + baryCoord->z);
}

struct CollisionEdge* collisionNextEdge(struct CollisionEdge* edge, int endpointIndex, int* nextEndpointIndex) {
    struct CollisionFace* face = edge->faces[endpointIndex];

    if (!face) {
        return NULL;
    }

    int edgeIndex = edge->edgeIndex[endpointIndex];

    if (edgeIndex == 0) {
        edgeIndex = 2;
    } else {
        edgeIndex -= 1;
    }

    *nextEndpointIndex = face->edgeIndices[edgeIndex] ^ 0x1;
    return face->edges[edgeIndex];
}

struct CollisionEdge* collisionPrevEdge(struct CollisionEdge* edge, int endpointIndex, int *nextEndpointIndex) {
    struct CollisionFace* face = edge->faces[endpointIndex ^ 0x1];

    if (!face) {
        return NULL;
    }

    int edgeIndex = edge->edgeIndex[endpointIndex];

    if (edgeIndex == 2) {
        edgeIndex = 0;
    } else {
        edgeIndex += 1;
    }

    *nextEndpointIndex = face->edgeIndices[edgeIndex];
    return face->edges[edgeIndex];
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
                firstEdge->faces[1]->edgeIndices[firstEdge->edgeIndex[1]] = 1;
                secondEdge->endpoints[0] = NULL;
                secondEdge->endpoints[1] = NULL;
            }
        }
    }

    secondEdgeIndex = 0;
    for (firstEdgeIndex = 0; firstEdgeIndex < target->edgeCount; ++firstEdgeIndex) {
        struct CollisionEdge* firstEdge = &target->edges[firstEdgeIndex];

        if (firstEdge->endpoints[0] && firstEdge->endpoints[1]) {
            if (firstEdgeIndex != secondEdgeIndex) {
                collisionReplaceEdges(firstEdge, &target->edges[secondEdgeIndex]);
                target->edges[secondEdgeIndex] = *firstEdge;
            }

            ++secondEdgeIndex;
        }
    }

    target->edgeCount = secondEdgeIndex;
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

        struct Vector3 edgeDir0;
        struct Vector3 edgeDir1;
        
        struct CollisionFace* face = &target->faces[faceIndex];

        face->edges[0] = &target->edges[i + 0];
        face->edges[1] = &target->edges[i + 1];
        face->edges[2] = &target->edges[i + 2];

        face->edgeIndices[0] = 0;
        face->edgeIndices[1] = 0;
        face->edgeIndices[2] = 0;

        vector3Sub(v1, v0, &edgeDir0);
        vector3Sub(v2, v0, &edgeDir1);

        float edgeDot = vector3Dot(&edgeDir0, &edgeDir1);
        float edgeLength0 = vector3MagSqrd(&edgeDir0);
        float edgeLength1 = vector3MagSqrd(&edgeDir1);

        face->barycentricDenom = 1.0f / (
            vector3MagSqrd(&edgeDir0) * vector3MagSqrd(&edgeDir1) -
            edgeDot * edgeDot
        );

        face->dotCompare1.x = edgeLength1 * edgeDir0.x - edgeDot * edgeDir1.x;
        face->dotCompare1.y = edgeLength1 * edgeDir0.y - edgeDot * edgeDir1.y;
        face->dotCompare1.z = edgeLength1 * edgeDir0.z - edgeDot * edgeDir1.z;

        face->dotCompare2.x = edgeLength0 * edgeDir1.x - edgeDot * edgeDir0.x;
        face->dotCompare2.y = edgeLength0 * edgeDir1.y - edgeDot * edgeDir0.y;
        face->dotCompare2.z = edgeLength0 * edgeDir1.z - edgeDot * edgeDir0.z;

        struct Vector3 dotCompare2;

        struct Vector3 normal;
        vector3Cross(&edgeDir0, &edgeDir1, &normal);
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

int edgeIndexToVertexIndex(int edgeIndex) {
    if (edgeIndex == 0) {
        return 2;
    } else {
        return edgeIndex - 1;
    }
}

struct CollisionFace* collisionGetAdjacentFace(struct CollisionFace* face, int edgeIndex) {
    return face->edges[edgeIndex]->faces[1 - face->edgeIndices[edgeIndex]];
}