
#include "meshcapsulecollision.h"

#include <ultra64.h>
#include <math.h>

#include "src/system/fastalloc.h"

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

    return MeshCollisionResultTriangle;
}

enum MeshCollisionResult meshEdgeSphereConcatPoint(struct CollisionEdge* edge, struct Vector3* pos, float radius, struct ContactPoint* contactPoint) {

}

enum MeshCollisionResult meshEdgeCapsuleContactPoint(struct CollisionEdge* edge, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint) {
    struct Vector3 edgeDir;
    struct Vector3 originOffset;

    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeDir);

    float denom = edgeDir.x * edgeDir.x + edgeDir.z * edgeDir.z;

    if (fabs(denom) < 0.00001f) {

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
                contactPoint->target = edge;
                contactPoint->type = ColliderTypeMeshEdge;

                contactPoint->contact.x = edge->endpoints[0]->x + edgeDir.x * edgeLerp;
                contactPoint->contact.y = edge->endpoints[0]->y + edgeDir.y * edgeLerp;
                contactPoint->contact.z = edge->endpoints[0]->z + edgeDir.z * edgeLerp;

                struct Vector3 capsulePoint;

                capsulePoint.x = capsule->center.x;
                capsulePoint.y = capsule->center.y + capsuleLerp;
                capsulePoint.z = capsule->center.z;

                vector3Sub(&capsulePoint, &contactPoint->contact, &contactPoint->normal);
                vector3Normalize(&contactPoint->normal, &contactPoint->normal);

                return MeshCollisionResultLineSegment;
            }
        }
    }
}

int meshCapsuleContactPoint(struct CollisionMesh* mesh, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint) {
    int i;

    AllocState prevState = getFastAllocState();

    // TODO a more reasonable size
    struct CollisionEdge** edges = fastalloc(sizeof(struct CollisionEdge*) * mesh->faceCount * 2);
    int edgeCount = 0;

    for (i = 0; i < mesh->faceCount; ++i) {
        struct CollisionFace* face = &mesh->faces[i];
        struct Vector3 baryCoord;
        enum MeshCollisionResult result = meshFaceCapsuleContactPoint(face, capsule, contactPoint, &baryCoord);
        if (result == MeshCollisionResultTriangle) {
            contactPoint->target = &mesh->faces[i];
            contactPoint->type = ColliderTypeMeshFace;
            restoreFastAllocState(prevState);
            return 1;
        } else if (result == MeshCollisionResultPlane) {
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

    restoreFastAllocState(prevState);

    return 0;
}