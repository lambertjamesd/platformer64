
#include <math.h>
#include "meshslide.h"
#include "collisionmesh.h"
#include "meshraycast.h"
#include "src/math/ray.h"

#define ZERO_LIKE_TOLERANCE 0.000001f

#define TIME_TILL_BARY_EDGE(baryCoord, vel) (fabs(vel) < ZERO_LIKE_TOLERANCE ? -1.0f : (-baryCoord / vel))

struct SlideResult slideContactPointFace(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    struct Vector3 projectedPoint;
    struct Vector3 dirBary;
    struct Vector3 startingPointBary;
    struct Vector3 sliderCenter;
    struct CollisionFace* face = point->target;

    struct ContactPoint nextContact = *point;
    int i;

    planeProjectOnto(&face->plane, dir, &projectedPoint);
    collisionFaceBaryCoord(face, &projectedPoint, &dirBary);

    collisionFaceBaryCoord(face, &point->contact, &startingPointBary);

    float moveDistance = distance;

    vector3Scale(&point->normal, &sliderCenter, sliderRadius);
    vector3Add(&sliderCenter, &point->contact, &sliderCenter);

    for (i = 0; i < 3; ++i) {
        if (dirBary.el[i] < 0.0f) {
            int edgeIndex = vertexIndexToEdgeIndex(i);
            struct CollisionFace* otherFace = collisionGetAdjacentFace(face, edgeIndex);
            float hitDistance;
            if (
                otherFace && 
                (hitDistance = spherecastPlane(&sliderCenter, dir, &otherFace->plane, sliderRadius)) >= -ZERO_LIKE_TOLERANCE &&
                hitDistance < moveDistance) {
                struct Vector3 otherBaryCoord;
                struct Vector3 centerAtHit;
                rayPointAtDistance(&sliderCenter, dir, hitDistance, &centerAtHit);
                collisionFaceBaryCoord(otherFace, &centerAtHit, &otherBaryCoord);

            } else {
                float timeTillEdge = TIME_TILL_BARY_EDGE(startingPointBary.el[i], dirBary.el[i]);

                if (timeTillEdge >= -ZERO_LIKE_TOLERANCE && timeTillEdge < moveDistance) {
                    nextContact.target = face->edges[edgeIndex];
                    nextContact.type = ColliderTypeMeshEdge;
                    moveDistance = timeTillEdge;
                }
            }
        }
    }

    struct SlideResult result;
    result.type = (point->target == nextContact.target) ? SlideResultComplete : SlideResultNewContact;
    result.moveDistance = moveDistance;

    struct Vector3 newPos;
    vector3Scale(dir, &nextContact.contact, moveDistance);
    vector3Add(&nextContact.contact, &newPos, &nextContact.contact);

    point->target = nextContact.target;
    point->type = nextContact.type;
    point->normal = nextContact.normal;

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