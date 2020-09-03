
#ifndef _SCENE_RENDER_H
#define _SCENE_RENDER_H

#include <ultra64.h>

#define MAX_DL_LENGTH           1024
#define MAX_SCENE_OBJECTS       256
#define MAX_MATERIAL_BATCHES    32

#define NO_MATERIAL             -1

typedef long MaterialId;

struct SceneRenderObject {
    Mtx matrix;
    Gfx* geometry;
    struct SceneRenderObject* next;
};

struct SceneRenderState {
    Mtx viewMtx;
    Mtx perspectiveMtx;
    struct SceneRenderObject objectsToRender[MAX_SCENE_OBJECTS];
    Gfx* materials[MAX_MATERIAL_BATCHES];
    struct SceneRenderObject* objectsByMaterial[MAX_MATERIAL_BATCHES];
    Gfx dl[MAX_DL_LENGTH];
    u16 usedMaterials;
    u16 usedObjects;
    u16 perspectiveNorm;
};

void sceneRenderInit(struct SceneRenderState* sceneRender);
void sceneRenderPreFrame(struct SceneRenderState* sceneRender);
MaterialId sceneRenderMatId(struct SceneRenderState* sceneRender, Gfx* material);
void sceneRenderDrawDynamic(struct SceneRenderState* sceneRender, Gfx* geometry, Mtx* mtx, MaterialId material);
Gfx* sceneRenderGenDL(struct SceneRenderState* sceneRender);

#endif