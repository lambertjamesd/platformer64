
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

struct SceneMaterialBatcher {
    struct SceneRenderObject objectsToRender[MAX_SCENE_OBJECTS];
    Gfx* materials[MAX_MATERIAL_BATCHES];
    struct SceneRenderObject* objectsByMaterial[MAX_MATERIAL_BATCHES];
    u16 usedMaterials;
    u16 usedObjects;
};

void materialBatcherInit(struct SceneMaterialBatcher* materialBatcher);
void materialBatcherPreFrame(struct SceneMaterialBatcher* materialBatcher);
MaterialId materialBatcherMatId(struct SceneMaterialBatcher* materialBatcher, Gfx* material);
void materialBatcherDrawDynamic(struct SceneMaterialBatcher* materialBatcher, Gfx* geometry, Mtx* mtx, MaterialId material);
Gfx* materialBatcherGenDL(struct SceneMaterialBatcher* materialBatcher, Gfx* dl);

#endif