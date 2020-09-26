
#include <math.h>
#include "meshslide.h"
#include "collisionmesh.h"
#include "meshraycast.h"
#include "src/math/ray.h"
#include "src/system/fastalloc.h"

#define ZERO_LIKE_TOLERANCE 0.000001f

#define TIME_TILL_BARY_EDGE(baryCoord, vel) (fabs(vel) < ZERO_LIKE_TOLERANCE ? -1.0f : (-baryCoord / vel))

int slideStateHasFace(struct SlideRaycastState* state, struct CollisionFace* face) {
    int i;
    for (i = 0; i < state->checkedFaceCount; ++i) {
        if (state->checkedFaces[i] == face) {
            return 1;
        }
    }
    return 0;
}

void slideStateMarkFace(struct SlideRaycastState* state, struct CollisionFace* face) {
    if (state->checkedFaceCount < MAX_EDGE_CHECK) {
        state->checkedFaces[state->checkedFaceCount] = face;
        ++state->checkedFaceCount;
    }
}

void slideStateQueueEdge(struct SlideRaycastState* state, struct CollisionEdge* edge) {
    if (state->edgesToCheckCount < MAX_EDGE_CHECK) {
        int i;
        for (i = 0; i < state->edgesToCheckCount; ++i) {
            if (state->edgesToCheck[i] == edge) {
                return;
            }
        }

        state->edgesToCheck[state->edgesToCheckCount] = edge;
        ++state->edgesToCheckCount;
    }
}

float slideSpherecastEdges(struct Vector3* origin, struct Vector3* dir, float radius, struct SlideRaycastState* castState, struct ContactPoint* contact) {
    int pointCheckIndex = 0;
    int edgeIndex;
    for (edgeIndex = 0; edgeIndex < castState->edgesToCheckCount; ++edgeIndex) {
        float edgeLerp;
        float result = spherecastLineOverlap(origin, dir, castState->edgesToCheck[edgeIndex], radius, contact, &edgeLerp);

        if (result > -ZERO_LIKE_TOLERANCE && result != RAYCAST_NO_HIT) {
            if (edgeLerp < 0.0f) {
                castState->edgesToCheck[++pointCheckIndex] = castState->edgesToCheck[edgeIndex];
            } else if (edgeLerp > 1.0f) {
                castState->edgesToCheck[++pointCheckIndex] = (struct CollisionEdge*)((int)castState->edgesToCheck[edgeIndex] | 0x1);
            } else {
                castState->target = castState->edgesToCheck[edgeIndex];
                castState->targetType = ColliderTypeMeshEdge;
                return result;
            }
        }
    }

    for (edgeIndex = 0; edgeIndex < pointCheckIndex; ++edgeIndex) {
        struct CollisionEdge* edge = (struct CollisionEdge*)((int)castState->edgesToCheck[edgeIndex] & ~0x1);
        int pointIndex = (int)castState->edgesToCheck[edgeIndex] & 0x1;

        float result = spherecastPoint(origin, dir, edge->endpoints[pointIndex], radius, contact);

        if (result > -ZERO_LIKE_TOLERANCE && result != RAYCAST_NO_HIT) {
            castState->target = edge;
            castState->targetType = ColliderTypeMeshEdgeEnd0 + pointIndex;
            return result;
        }
    }

    return RAYCAST_NO_HIT;
}

float slideSpherecastFaces(struct Vector3* origin, struct Vector3* dir, float radius, struct SlideRaycastState* castState, struct CollisionFace* checkFace) {
    if (!slideStateHasFace(castState, checkFace)) {
        slideStateMarkFace(castState, checkFace);

        float result = spherecastPlane(origin, dir, &checkFace->plane, radius);

        if (result > -ZERO_LIKE_TOLERANCE && result != RAYCAST_NO_HIT) {
            struct Vector3 baryCoord;
            struct Vector3 centerAtHit;
            int isContained = 1;
            rayPointAtDistance(origin, dir, result, &centerAtHit);
            collisionFaceBaryCoord(checkFace, &centerAtHit, &baryCoord);

            int vertexIndex = 0;

            for (vertexIndex = 0; vertexIndex < 3; ++vertexIndex) {
                if (baryCoord.el[vertexIndex] < 0.0f) {
                    int edgeIndex = vertexIndexToEdgeIndex(vertexIndex);
                    slideStateQueueEdge(castState, checkFace->edges[edgeIndex]);

                    struct CollisionFace* otherFace = checkFace->edges[edgeIndex]->faces[1 - checkFace->edgeIndices[edgeIndex]];
                    slideSpherecastFaces(origin, dir, radius, castState, otherFace);
                    isContained = 0;
                }
            }

            if (isContained) {
                castState->target = checkFace;
                castState->targetType = ColliderTypeMeshFace;
                return result;
            }
        }
    }

    return RAYCAST_NO_HIT;
}

struct SlideResult slideContactPointFace(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    struct Vector3 projectedPoint;
    struct Vector3 dirBary;
    struct Vector3 startingPointBary;
    struct Vector3 sliderCenter;
    struct CollisionFace* face = point->target;

    struct ContactPoint nextContact = *point;
    int pointIndex;

    planeProjectOnto(&face->plane, dir, &projectedPoint);
    collisionFaceBaryCoord(face, &projectedPoint, &dirBary);

    collisionFaceBaryCoord(face, &point->contact, &startingPointBary);

    float moveDistance = distance;

    vector3Scale(&point->normal, &sliderCenter, sliderRadius);
    vector3Add(&sliderCenter, &point->contact, &sliderCenter);

    struct SlideRaycastState raycastState;
    raycastState.checkedFaces[0] = face;
    raycastState.checkedFaceCount = 1;
    raycastState.edgesToCheckCount = 0;
    raycastState.pointsToCheckCount = 0;

    for (pointIndex = 0; pointIndex < 3; ++pointIndex) {
        if (dirBary.el[pointIndex] < 0.0f) {
            int edgeIndex = vertexIndexToEdgeIndex(pointIndex);
            struct CollisionFace* otherFace = collisionGetAdjacentFace(face, edgeIndex);
            float hitDistance;
            if (
                otherFace && 
                (hitDistance = slideSpherecastFaces(&sliderCenter, dir, sliderRadius, &raycastState, otherFace)) >= -ZERO_LIKE_TOLERANCE &&
                hitDistance < moveDistance) {
                nextContact.target = raycastState.target;
                nextContact.type = raycastState.targetType;
                moveDistance = hitDistance;

                nextContact.normal = ((struct CollisionFace*)raycastState.target)->plane.normal;
            } else {
                float timeTillEdge = TIME_TILL_BARY_EDGE(startingPointBary.el[pointIndex], dirBary.el[pointIndex]);

                if (timeTillEdge >= -ZERO_LIKE_TOLERANCE && timeTillEdge < moveDistance) {
                    nextContact.target = face->edges[edgeIndex];
                    nextContact.type = ColliderTypeMeshEdge;
                    moveDistance = timeTillEdge;
                }
            }
        }
    }

    struct ContactPoint edgeCheck;
    float edgeHit = slideSpherecastEdges(&sliderCenter, dir, sliderRadius, &raycastState, &edgeCheck);

    if (edgeHit >= -ZERO_LIKE_TOLERANCE && edgeHit < moveDistance) {
        nextContact = edgeCheck;
        moveDistance = edgeHit;
    }

    struct SlideResult result;
    result.type = (point->target == nextContact.target) ? SlideResultComplete : SlideResultNewContact;
    result.moveDistance = moveDistance;

    struct Vector3 newPos;
    vector3Scale(dir, &nextContact.contact, moveDistance);
    vector3Add(&nextContact.contact, &newPos, &point->contact);

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