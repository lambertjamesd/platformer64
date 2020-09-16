
#include "meshcapsulecollision.h"

#include <ultra64.h>
#include <math.h>

#include "src/system/fastalloc.h"
#include "src/math/fastsqrt.h"

enum MeshCollisionResult meshFaceCapsuleContactPoint(struct CollisionFace* face, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint, struct Vector3* baryCoord) {
    struct Vector3 center = capsule->center;

    if (face->plane.b > 0.0f) {
        center.y -= capsule->innerHeight * 0.5f;
    } else if (face->plane.b < 0.0f) {
        center.y += capsule->innerHeight * 0.5f;
    }

    float offset = planeDistanceFromPoint(&face->plane, &center);

    float overlapDistance = capsule->radius - offset;

    // Check if the capsule touches the plane of the triangle
    if (overlapDistance < 0.0f) {
        return MeshCollisionResultNone;
    } else if (overlapDistance > capsule->radius * 2.0f + fabs(capsule->innerHeight * face->plane.b)) {
        return MeshCollisionResultNone;
    }

    collisionFaceBaryCoord(face, &center, baryCoord);

    // check if capsule is withing the bounds of the triangle
    if (baryCoord->x < 0.0f || baryCoord->y < 0.0f || baryCoord->z < 0.0f) {
        return MeshCollisionResultPlane;
    }

    contactPoint->overlapDistance = overlapDistance;

    contactPoint->normal.x = face->plane.a;
    contactPoint->normal.y = face->plane.b;
    contactPoint->normal.z = face->plane.c;

    contactPoint->contact.x = center.x + face->plane.a * overlapDistance;
    contactPoint->contact.y = center.y + face->plane.b * overlapDistance;
    contactPoint->contact.z = center.z + face->plane.c * overlapDistance;
    contactPoint->target = face;
    contactPoint->type = ColliderTypeMeshFace;

    return MeshCollisionResultTriangle;
}

enum MeshCollisionResult meshEdgeSphereConcatPoint(struct CollisionEdge* edge, struct Vector3* pos, float radius, struct ContactPoint* contactPoint) {
    struct Vector3 edgeDir;
    struct Vector3 projectedPoint;

    struct Vector3 offset;
    vector3Sub(pos, edge->endpoints[0], &offset);
    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeDir);
    float scalar = vector3Dot(&offset, &edgeDir) / vector3MagSqrd(&edgeDir);

    if (scalar < 0.0f) {
        return MeshCollisionResultLineEnd0;
    } else if (scalar > 1.0f) {
        return MeshCollisionResultLineEnd1;
    }

    contactPoint->contact.x = edge->endpoints[0]->x + edgeDir.x * scalar;
    contactPoint->contact.y = edge->endpoints[0]->y + edgeDir.y * scalar;
    contactPoint->contact.z = edge->endpoints[0]->z + edgeDir.z * scalar;

    vector3Sub(pos, &contactPoint->contact, &offset);

    float magSqrd = vector3MagSqrd(&offset) > radius * radius;

    if (magSqrd) {
        return MeshCollisionResultNone;
    }

    vector3Scale(&offset, &contactPoint->normal, fastInvSqrt(magSqrd));
    contactPoint->overlapDistance = radius - vector3Dot(&offset, &contactPoint->normal);
    contactPoint->target = edge;
    contactPoint->type = ColliderTypeMeshEdge;
}

enum MeshCollisionResult meshEdgeCapsuleContactPoint(struct CollisionEdge* edge, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint) {
    struct Vector3 edgeDir;
    struct Vector3 originOffset;

    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeDir);

    float denom = edgeDir.x * edgeDir.x + edgeDir.z * edgeDir.z;

    if (fabs(denom) < 0.00001f) {
        int minIndex = edge->endpoints[1]->y < edge->endpoints[0]->y;

        if (edge->endpoints[minIndex]->y > capsule->center.y + capsule->innerHeight * 0.5f) {
            return MeshCollisionResultLineEnd0 + minIndex;
        } else if (edge->endpoints[1 - minIndex]->y < capsule->center.y - capsule->innerHeight * 0.5f) {
            return MeshCollisionResultLineEnd0 + (1 - minIndex);
        } else {
            struct Vector3 referencePoint = capsule->center;

            if (edge->endpoints[minIndex]->y > referencePoint.y) {
                referencePoint.y = edge->endpoints[minIndex]->y;
            } else if (edge->endpoints[1 - minIndex]->y < referencePoint.y) {
                referencePoint.y = edge->endpoints[1 - minIndex]->y;
            }

            return meshEdgeSphereConcatPoint(edge, &referencePoint, capsule->radius, contactPoint);
        }
    } else {
        vector3Sub(edge->endpoints[0], &capsule->center, &originOffset);
        float partialEdgeDot = edgeDir.x * originOffset.x + edgeDir.z * originOffset.z;

        float capsuleLerp = 
            (denom + edgeDir.y * edgeDir.y) * originOffset.y
            - 
            edgeDir.y * (partialEdgeDot + edgeDir.y * originOffset.y);

        denom = 1.0f / denom;

        capsuleLerp *= denom;

        if (capsuleLerp < capsule->innerHeight * 0.5f) {
            originOffset = capsule->center;
            originOffset.y += capsule->innerHeight * 0.5f;
            return meshEdgeSphereConcatPoint(edge, &originOffset, capsule->radius, contactPoint);
        } else if (capsuleLerp < capsule->innerHeight * -0.5f) {
            originOffset = capsule->center;
            originOffset.y += capsule->innerHeight * -0.5f;
            return meshEdgeSphereConcatPoint(edge, &originOffset, capsule->radius, contactPoint);
        } else {
            float edgeLerp = partialEdgeDot * denom;

            if (edgeLerp > 1.0f) {
                return MeshCollisionResultLineEnd1;
            } else if (edgeLerp < 0.0f) {
                return MeshCollisionResultLineEnd0;
            } else {
                contactPoint->contact.x = edge->endpoints[0]->x + edgeDir.x * edgeLerp;
                contactPoint->contact.y = edge->endpoints[0]->y + edgeDir.y * edgeLerp;
                contactPoint->contact.z = edge->endpoints[0]->z + edgeDir.z * edgeLerp;

                struct Vector3 capsulePoint;

                capsulePoint.x = capsule->center.x;
                capsulePoint.y = capsule->center.y + capsuleLerp;
                capsulePoint.z = capsule->center.z;

                struct Vector3 offset;

                vector3Sub(&capsulePoint, &contactPoint->contact, &offset);

                float distSqrd = vector3MagSqrd(&offset);

                if (distSqrd > capsule->radius * capsule->radius) {
                    return MeshCollisionResultNone;
                }

                vector3Scale(&offset, &contactPoint->normal, fastInvSqrt(distSqrd));
                contactPoint->overlapDistance = capsule->radius - vector3Dot(&offset, &contactPoint->normal);
                contactPoint->target = edge;
                contactPoint->type = ColliderTypeMeshEdge;

                return MeshCollisionResultLineSegment;
            }
        }
    }
}

int meshCapsuleContactPoint(struct CollisionMesh* mesh, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint) {
    int i;

    AllocState prevState = getFastAllocState();

    struct CollisionEdge** edges = prevState;
    int edgeCount = 0;
    int edgeArraySize = 0;

    for (i = 0; i < mesh->faceCount; ++i) {
        struct CollisionFace* face = &mesh->faces[i];
        struct Vector3 baryCoord;
        enum MeshCollisionResult result = meshFaceCapsuleContactPoint(face, capsule, contactPoint, &baryCoord);
        if (result == MeshCollisionResultTriangle) {
            restoreFastAllocState(prevState);
            return 1;
        } else if (result == MeshCollisionResultPlane) {
            if (edgeCount == edgeArraySize) {
                // allocate 2 slots since fast alloc aligns to 8 bytes
                fastalloc(sizeof(struct CollisionEdge*) * 2);
                edgeArraySize += 2;
            }

            if (baryCoord.x < 0.0f) {
                edges[edgeCount++] = face->edges[1];
            }

            if (baryCoord.y < 0.0f) {
                edges[edgeCount++] = face->edges[2];
            }

            if (baryCoord.z < 0.0f) {
                edges[edgeCount++] = face->edges[0];
            }
        }
    }

    int pointCount = 0;
    int pointArraySize = 0;

    for (i = 0; i < edgeCount; ++i) {
        enum MeshCollisionResult result = meshEdgeCapsuleContactPoint(edges[i], capsule, contactPoint);

        if (result == MeshCollisionResultLineSegment) {
            restoreFastAllocState(prevState);
            return 1;
        } else if (result == MeshCollisionResultLineEnd0) {
            if (pointCount == pointArraySize) {
                // allocate 2 slots since fast alloc aligns to 8 bytes
                fastalloc(sizeof(struct CollisionEdge*) * 2);
                pointArraySize += 2;
            }
            edges[pointCount++] = edges[i];
        } else if (result == MeshCollisionResultLineEnd1) {
            if (pointCount == pointArraySize) {
                // allocate 2 slots since fast alloc aligns to 8 bytes
                fastalloc(sizeof(struct CollisionEdge*) * 2);
                pointArraySize += 2;
            }
            edges[pointCount++] = (struct CollisionEdge*)((u32)edges[i] | 0x1);
        }
    }

    restoreFastAllocState(prevState);

    return 0;
}