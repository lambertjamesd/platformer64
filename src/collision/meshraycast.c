
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

    return raycastPlane(&modifiedOrigin, dir, plane);
}

float spherecastFace(struct Vector3* origin, struct Vector3* dir, struct CollisionFace* face, float radius, struct ContactPoint* contact) {
    struct Vector3 normal;
    struct Vector3 faceOffset;
    vector3Sub(dir, )
    vector3Cross(dir, )

    return RAYCAST_NO_HIT;
}

float spherecastLineOverlap(struct Vector3* origin, struct Vector3* dir, struct CollisionEdge* edge, float radius, struct ContactPoint* contact, float* edgeLerp) {
    struct Vector3 edgeDir;
    struct Vector3 originOffset;
    struct Vector3 normal;
    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeDir);
    vector3Cross(dir, &edgeDir, &normal);
    vector3Sub(edge->endpoints[0], origin, &originOffset);

    float normalDistSqrd = vector3MagSqrd(&normal);

    if (fabs(normalDistSqrd) < 0.000001f) {
        vector3Project(&originOffset, dir, &edgeDir);
        vector3Add(edge->endpoints[0], &edgeDir, &edgeDir);
        if (vector3DistSqrd(&edgeDir, edge->endpoints[0]) <= radius * radius) {
            contact->contact = edgeDir;
            return RAYCAST_FULL_OVERLAP;
        } else {
            return RAYCAST_NO_HIT;
        }
    } else {
        struct Vector3 lineOffset;
        vector3Scale(&normal, &lineOffset, vector3Dot(&normal, &originOffset) / normalDistSqrd);

        normalDistSqrd = vector3MagSqrd(&lineOffset);
        float overlapOffset = radius * radius - normalDistSqrd;

        if (overlapOffset < 0.0f) {
            return RAYCAST_NO_HIT;
        }

        float xyDenom = dir->x * edgeDir.y - dir->y * edgeDir.x;
        float yzDenom = dir->y * edgeDir.z - dir->z * edgeDir.y;

        if (xyDenom > yzDenom) {
            *edgeLerp = (
                dir->y * (originOffset.x - lineOffset.x) - 
                dir->x * (originOffset.y - lineOffset.y)
            ) / xyDenom;
        } else {
            *edgeLerp = (
                dir->z * (originOffset.y - lineOffset.y) - 
                dir->y * (originOffset.z - lineOffset.z)
            ) / yzDenom;
        }

        vector3Scale(&edgeDir, *edgeLerp, contact->contact);
        vector3Add(edge->endpoints[0], &edgeDir, contact->contact);

        float result = (contact->contact.x - origin->x) * dir->x + 
            (contact->contact.y - origin->y) * dir->y + 
            (contact->contac.z - origin->z) * dir->z;
            
        contact->overlapDistance = overlapOffset;

        return result;
    }
}