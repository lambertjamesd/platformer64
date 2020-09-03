
#include "scenerender.h"

void materialBatcherInit(struct SceneMaterialBatcher* materialBatcher) {
    materialBatcher->usedMaterials = 0;
    materialBatcherPreFrame(materialBatcher);
}

void materialBatcherPreFrame(struct SceneMaterialBatcher* materialBatcher) {
    materialBatcher->usedObjects = 0;

    int i;
    for (i = 0; i < MAX_MATERIAL_BATCHES; ++i) {
        materialBatcher->objectsByMaterial[i] = NULL;
    }
}

MaterialId materialBatcherMatId(struct SceneMaterialBatcher* materialBatcher, Gfx* material) {
    int i;

    for (i = 0; i < materialBatcher->usedMaterials; ++i) {
        if (materialBatcher->materials[i] == material) {
            return i;
        }   
    }

    if (materialBatcher->usedMaterials == MAX_MATERIAL_BATCHES) {
        return NO_MATERIAL;
    } else {
        materialBatcher->materials[i] = material;
        return materialBatcher->usedMaterials++;
    }
}

void materialBatcherDrawDynamic(struct SceneMaterialBatcher* materialBatcher, Gfx* geometry, Mtx* mtx, MaterialId material) {
    if (materialBatcher->usedObjects != MAX_SCENE_OBJECTS && material != NO_MATERIAL) {
        struct SceneRenderObject* next = &materialBatcher->objectsToRender[materialBatcher->usedObjects];

        next->matrix = *mtx;
        next->geometry = geometry;
        next->next = materialBatcher->objectsByMaterial[material];
        materialBatcher->objectsByMaterial[material] = next;

        ++materialBatcher->usedObjects;
    }
}

Gfx* materialBatcherGenDL(struct SceneMaterialBatcher* materialBatcher, Gfx* dl) {
    int i;
    for (i = 0; i < materialBatcher->usedMaterials; ++i) {
        if (materialBatcher->objectsByMaterial[i]) {
            gSPDisplayList(dl++, materialBatcher->materials[i]);

            struct SceneRenderObject* curr = materialBatcher->objectsByMaterial[i];

            while (curr) {
                gSPMatrix(dl++, &curr->matrix, G_MTX_MODELVIEW|G_MTX_PUSH|G_MTX_MUL);
                gSPDisplayList(dl++, curr->geometry);
                gSPPopMatrix(dl++, G_MTX_MODELVIEW);

                curr = curr->next;
            }
        }
    }

    return dl;
}