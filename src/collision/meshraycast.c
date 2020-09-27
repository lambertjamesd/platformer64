
#include "meshraycast.h"
#include <math.h>

float raycastPlane(struct Vector3* origin, struct Vector3* dir, struct Plane* plane) {
    float denom = vector3Dot(dir, &plane->normal);

    if (denom > -0.0001f) {
        return RAYCAST_NO_HIT;
    }
    
    float result = (-plane->d - vector3Dot(origin, &plane->normal)) / denom;

    if (result >= 0.0f) {
        return result;
    } else {
        return RAYCAST_NO_HIT;
    }
}

float spherecastPlane(struct Vector3* origin, struct Vector3* dir, struct Plane* plane, float radius) {
    struct Vector3 modifiedOrigin;
    modifiedOrigin.x = origin->x - plane->a * radius;
    modifiedOrigin.x = origin->y - plane->b * radius;
    modifiedOrigin.x = origin->z - plane->c * radius;

    float denom = vector3Dot(dir, &plane->normal);
    float originDistance = vector3Dot(&modifiedOrigin, &plane->normal) + plane->d;

    if (denom > -0.0001f) {
        if (fabs(originDistance) <= radius) {
            return RAYCAST_FULL_OVERLAP;
        } else {
            return RAYCAST_NO_HIT;
        }
    }
    
    float result = -originDistance / denom;

    if (result >= 0.0f) {
        return result;
    } else {
        return RAYCAST_NO_HIT;
    }
}

float spherecastFace(struct Vector3* origin, struct Vector3* dir, struct CollisionFace* face, float radius, struct ContactPoint* contact) {
    struct Vector3 normal;
    struct Vector3 faceOffset;

    return RAYCAST_NO_HIT;
}

float spherecastLineOverlap(struct Vector3* origin, struct Vector3* dir, struct CollisionEdge* edge, float radius, struct ContactPoint* contact, float* edgeLerp) {
    struct Vector3 edgeOffset;
    struct Vector3 dirDiff;
    struct Vector3 originDiff;
    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeOffset);

    float dirDot = vector3Dot(&edgeOffset, dir);

    float edgeLenSq = 1.0f / vector3MagSqrd(&edgeOffset);
    vector3Scale(&edgeOffset, &dirDiff, dirDot * edgeLenSq);
    vector3Sub(dir, &dirDiff, &dirDiff);

    float dirDiffMagSq = vector3MagSqrd(&dirDiff);
    float rSqrd = radius * radius;

    vector3Scale(&edgeOffset, &originDiff, vector3Dot(&edgeOffset, origin) * edgeLenSq);
    vector3Sub(origin, &originDiff, &originDiff);

    // check if ray and edge are in the same direction
    if (dirDiffMagSq < 0.00001f) {
        // line is fully enclosed by spherecast
        if (vector3MagSqrd(&originDiff) < rSqrd) {
            *edgeLerp = dirDot;
            return RAYCAST_FULL_OVERLAP;
        } else {
            return RAYCAST_NO_HIT;
        }
    }

    float originDiffMagSq = vector3MagSqrd(&originDiff);
    float edgeDot = vector3Dot(&dirDiff, &originDiff);

    float overlapCheck = edgeDot * edgeDot - dirDiffMagSq * (originDiffMagSq - rSqrd);

    // ray and line don't get close enough to intersect
    if (overlapCheck < 0.0f) {
        return RAYCAST_NO_HIT;
    }

    // check if line is behind ray
    if (edgeDot > 0.0f) {
        return RAYCAST_NO_HIT;
    }

    float result = (-edgeDot - sqrtf(overlapCheck)) /  dirDiffMagSq;

    contact->target = edge;
    contact->type = ColliderTypeMeshEdge;
    
    struct Vector3 posAtContact;

    posAtContact.x = origin->x + dir->x * result;
    posAtContact.y = origin->y + dir->y * result;
    posAtContact.z = origin->z + dir->z * result;

    *edgeLerp = vector3Dot(&edgeOffset, &contact->contact) * edgeLenSq;

    vector3Sub(&posAtContact, edge->endpoints[0], &contact->contact);
    vector3Scale(&edgeOffset, &contact->contact, *edgeLerp);
    vector3Add(&contact->contact, edge->endpoints[0], &contact->contact);
    vector3Sub(&posAtContact, &contact->contact, &contact->normal);
    contact->overlapDistance = 0.0f;

    return result;
}

float spherecastPoint(struct Vector3* origin, struct Vector3* dir, struct Vector3* point, float radius, struct ContactPoint* contact) {
    struct Vector3 originOffset;
    vector3Sub(origin, point, &originOffset);

    float dirdot = vector3Dot(&originOffset, dir);

    if (dirdot > 0.0f) {
        // the point is behind the ray
        return RAYCAST_NO_HIT;
    }

    float offsetMagSqr = vector3MagSqrd(&originOffset);

    float sqrtDenom = dirdot * dirdot - offsetMagSqr - radius - radius;

    if (sqrtDenom >= 0.0f) {
        return -dirdot - sqrtf(sqrtDenom);
    } else {
        return RAYCAST_NO_HIT;
    }
    
}