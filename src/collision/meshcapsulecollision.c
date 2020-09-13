
#include "meshcapsulecollision.h"

#include <ultra64.h>
#include <math.h>

int meshFaceCapsuleContactPoint(struct CollisionFace* face, struct CollisionCapsule* capsule, struct ContactPoint* contactPoint) {
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
        return 0;
    } else if (overlapDistance > capsule->radius * 2.0f + fabs(capsule->innerHeight * face->plane.b)) {
        return 0;
    }

    struct Vector3 baryCoord;

    collisionFaceBaryCoord(face, &center, &baryCoord);

    // check if capsule is withing the bounds of the triangle
    if (baryCoord.x < -0.00001f || baryCoord.y < -0.00001f || baryCoord.z < -0.00001f) {
        return 0;
    }

    contactPoint->overlapDistance = overlapDistance;

    contactPoint->normal.x = face->plane.a;
    contactPoint->normal.y = face->plane.b;
    contactPoint->normal.z = face->plane.c;

    contactPoint->contact.x = center.x + face->plane.a * overlapDistance;
    contactPoint->contact.y = center.y + face->plane.b * overlapDistance;
    contactPoint->contact.z = center.z + face->plane.c * overlapDistance;

    return 1;
}