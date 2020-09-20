
#ifndef _SCENE_H
#define _SCENE_H

#include "src/collision/collisionmesh.h"

struct Scene {
    struct CollisionMesh* staticCollision;
};

extern struct Scene gScene;

#endif