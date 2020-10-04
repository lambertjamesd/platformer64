
#include <math.h>
#include "meshslide.h"
#include "collisionmesh.h"
#include "meshraycast.h"
#include "src/math/ray.h"
#include "src/system/fastalloc.h"
#include "debugger/serial.h"
#include <string.h>

#define ZERO_LIKE_TOLERANCE 0.000001f

#define TIME_TILL_BARY_EDGE(baryCoord, vel) (fabs(vel) < ZERO_LIKE_TOLERANCE ? -1.0f : (-baryCoord / vel))

char gTmpBufferFoo[1000];

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
    if (checkFace && !slideStateHasFace(castState, checkFace)) {
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
    struct Vector3 sliderCenter;
    struct Vector3 edgeDistance;
    struct CollisionFace* face = point->target;

    struct ContactPoint nextContact = *point;
    int pointIndex;

    float moveDistance = distance;

    vector3Scale(&point->normal, &sliderCenter, sliderRadius);
    vector3Add(&sliderCenter, &point->contact, &sliderCenter);

    collisionFaceBaryDistanceToEdge(face, &point->contact, dir, &edgeDistance);

    struct SlideRaycastState raycastState;
    raycastState.checkedFaces[0] = face;
    raycastState.checkedFaceCount = 1;
    raycastState.edgesToCheckCount = 0;
    raycastState.pointsToCheckCount = 0;

    for (pointIndex = 0; pointIndex < 3; ++pointIndex) {
        if (edgeDistance.el[pointIndex] > -ZERO_LIKE_TOLERANCE) {
            int edgeIndex = vertexIndexToEdgeIndex(pointIndex);
            struct CollisionFace* otherFace = collisionGetAdjacentFace(face, edgeIndex);
            float hitDistance;
            float otherFaceDot = 0.0f;

            if (otherFace) {
                otherFaceDot = vector3Dot(dir, &otherFace->plane.normal);
            }

            if (edgeDistance.el[pointIndex] < moveDistance) {
                struct CollisionEdge* edge = face->edges[edgeIndex];
                moveDistance = edgeDistance.el[pointIndex];

                if (otherFace && (vector3Dot(&face->plane.normal, &otherFace->plane.normal) > 0.9999 || otherFaceDot < 0.0f)) {
                    nextContact.target = otherFace;
                    nextContact.type = ColliderTypeMeshFace;
                    nextContact.normal = otherFace->plane.normal;
                } else {
                    nextContact.target = edge;
                    nextContact.type = ColliderTypeMeshEdge;
                }
            }

            if (
                otherFaceDot < 0.0f &&
                edgeDistance.el[pointIndex] < sliderRadius * 2.0f &&
                (hitDistance = slideSpherecastFaces(&sliderCenter, dir, sliderRadius, &raycastState, otherFace)) >= -ZERO_LIKE_TOLERANCE &&
                hitDistance < moveDistance) {

                nextContact.target = raycastState.target;
                nextContact.type = raycastState.targetType;
                moveDistance = hitDistance;
                nextContact.normal = ((struct CollisionFace*)raycastState.target)->plane.normal;
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

    vector3Scale(dir, &nextContact.contact, moveDistance);
    vector3Add(&nextContact.contact, &point->contact, &point->contact);

    point->target = nextContact.target;
    point->type = nextContact.type;
    point->normal = nextContact.normal;

    return result;
}

struct SlideResult slideContactPointEdgeToEndpoint(struct ContactPoint* point, struct CollisionEdge* edge, int endpointIndex) {
    struct SlideResult result;
    result.type = SlideResultNewContact;
    result.moveDistance = sqrtf(vector3DistSqrd(edge->endpoints[endpointIndex], &point->contact));
    
    point->target = edge;
    point->type = ColliderTypeMeshEdgeEnd0 + endpointIndex;
    point->contact = *edge->endpoints[endpointIndex];

    return result;
}

struct SlideResult slideContactPointEdge(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    struct CollisionEdge* edge = (struct CollisionEdge*)point->target;

    float faceDot[2];
    faceDot[0] = vector3Dot(dir, &edge->faces[0]->plane.normal);
    faceDot[1] = edge->faces[1] ? vector3Dot(dir, &edge->faces[1]->plane.normal) : faceDot[0];

    int faceIndex = faceDot[0] < faceDot[1];

    struct Vector3 moveDir;
    collisionFaceBaryDir(edge->faces[faceIndex], dir, &moveDir);

    struct CollisionFace* face = edge->faces[faceIndex];
    int otherEdgeIndex = edge->edgeIndex[faceIndex];

    if (faceDot[faceIndex] < ZERO_LIKE_TOLERANCE && moveDir.el[edgeIndexToVertexIndex(otherEdgeIndex)] > 0.0f) {
        point->target = face;
        point->type = ColliderTypeMeshFace;
        point->normal = face->plane.normal;

        struct SlideResult result;
        result.type = SlideResultNewContact;
        result.moveDistance = 0.0f;
        return result;
    }
    
    struct Vector3 edgeOffset;
    struct Vector3 movePlaneNormal;

    vector3Sub(edge->endpoints[1], edge->endpoints[0], &edgeOffset);
    vector3Cross(&point->normal, dir, &movePlaneNormal);

    float denom = vector3Dot(&edgeOffset, &movePlaneNormal);
    float edgeLength = vector3MagSqrd(&edgeOffset);

    // check if dir slides parallel to the edge
    if (fabs(denom) < ZERO_LIKE_TOLERANCE || !edge->faces[1]) {
        struct Vector3 offset;
        struct Vector3 finalPos;
        vector3Scale(dir, &offset, distance);
        vector3Add(&point->contact, &offset, &finalPos);

        struct Vector3 pointOffset;
        vector3Sub(&finalPos, edge->endpoints[0], &pointOffset);

        float pointDot = vector3Dot(&edgeOffset, &pointOffset);

        if (pointDot < 0.0f) {
            return slideContactPointEdgeToEndpoint(point, edge, 0);
        } else if (pointDot > edgeLength) {
            return slideContactPointEdgeToEndpoint(point, edge, 1);
        } else {
            if (!edge->faces[1]) {
                // project the final point into the edge
                vector3Scale(&edgeOffset, &offset, pointDot / edgeLength);
                vector3Add(edge->endpoints[0], &offset, &finalPos);
            }

            point->target = edge;
            point->type = ColliderTypeMeshEdge;
            point->contact = finalPos;

            struct SlideResult result;
            result.type = SlideResultComplete;
            result.moveDistance = distance;
            return result;
        }
    } else {
        struct Vector3 normalOffset;
        struct Vector3 pointOffset;

        vector3Sub(&point->normal, &face->plane.normal, &normalOffset);
        vector3Sub(&point->contact, edge->endpoints[0], &pointOffset);

        float edgeLerpDelta = sliderRadius * vector3Dot(&normalOffset, &movePlaneNormal) / denom;

        float edgeLerp = vector3Dot(&pointOffset, &edgeOffset) / edgeLength + edgeLerpDelta;

        if (edgeLerpDelta > 0.0f && edgeLerp > 1.0f) {
            struct SlideResult result = slideContactPointEdgeToEndpoint(point, edge, 0);
            vector3Lerp(&face->plane.normal, &point->normal, (edgeLerp - 1.0f) / edgeLerpDelta, &point->normal);
            vector3Normalize(&point->normal, &point->normal);
            return result;
        } else if (edgeLerpDelta < 0.0f && edgeLerp < 0.0f) {
            struct SlideResult result = slideContactPointEdgeToEndpoint(point, edge, 1);
            vector3Lerp(&face->plane.normal, &point->normal, -edgeLerp / edgeLerpDelta, &point->normal);
            vector3Normalize(&point->normal, &point->normal);
            return result;
        } else {
            struct Vector3 offset;
            struct Vector3 finalPos;
            vector3Scale(&edgeOffset, &offset, edgeLerp);

            point->target = edge;
            point->type = ColliderTypeMeshEdge;
            vector3Add(edge->endpoints[0], &offset, &point->contact);
            point->normal = face->plane.normal;

            struct SlideResult result;
            result.type = SlideResultComplete;
            result.moveDistance = distance;
            return result;
        }

        struct SlideResult result;
        result.type = SlideResultNewContact;
        result.moveDistance = 0.0f;
        return result;
    }
}


struct SlideResult slideContactPoint(struct ContactPoint* point, float sliderRadius, struct Vector3* dir, float distance) {
    switch (point->type) {
        case ColliderTypeMeshFace:
            return slideContactPointFace(point, sliderRadius, dir, distance);
        case ColliderTypeMeshEdge:
            return slideContactPointEdge(point, sliderRadius, dir, distance);
    }

    struct SlideResult result;
    result.type = SlideResultError;
    result.moveDistance = 0.0f;
    return result;
}