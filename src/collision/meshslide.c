
#include <math.h>
#include "meshslide.h"
#include "collisionmesh.h"

#define ZERO_LIKE_TOLERANCE 0.000001f

struct SlideResult slideContactPointFace(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    struct Vector3 projectedPoint;
    struct Vector3 baryCoord;
    struct CollisionFace* face = point->target;
    vector3Scale(dir, &projectedPoint, distance);
    vector3Add(&point->contact, &projectedPoint, &projectedPoint);
    planeProjectOnto(&face->plane, &projectedPoint, &projectedPoint);

    collisionFaceBaryCoord(face, &projectedPoint, &baryCoord);

    if (baryCoord.x > 0.0f && baryCoord.y > 0.0f && baryCoord.z > 0.0f) {
        struct SlideResult result;
        point->contact = projectedPoint;
        result.type = SlideResultComplete;
        result.moveDistance = distance;
        return result;
    }

    struct Vector3 startingPoint;

    collisionFaceBaryCoord(face, &point->contact, &startingPoint);

    struct Vector3 offset;

    vector3Sub(&baryCoord, &startingPoint, &offset);

    float edgeTime = fabs(offset.x) < ZERO_LIKE_TOLERANCE ? 10.0f : -startingPoint.x / offset.x;
    int edgeIndex = 1;

    float edgeCheck = fabs(offset.y) < ZERO_LIKE_TOLERANCE ? 10.0f : -startingPoint.y / offset.y;

    if (edgeCheck < edgeTime) {
        edgeTime = edgeTime;
        edgeIndex = 2;
    }

    edgeCheck = fabs(offset.z) < ZERO_LIKE_TOLERANCE ? 10.0f : -startingPoint.z / offset.z;

    if (edgeCheck < edgeTime) {
        edgeTime = edgeTime;
        edgeIndex = 0;
    }



    struct SlideResult result;
    result.type = SlideResultError;
    result.moveDistance = 0.0f;
    return result;
}

struct SlideResult slideContactPoint(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    switch (point->type) {
        case ColliderTypeMeshFace:
            return slideContactPointFace(point, sliderRadius, dir, distance);
    }

    struct SlideResult result;
    result.type = SlideResultError;
    result.moveDistance = 0.0f;
    return result;
}