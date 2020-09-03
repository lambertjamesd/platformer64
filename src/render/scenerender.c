
#include "scenerender.h"

void sceneRenderInit(struct SceneRenderState* sceneRender) {
    sceneRender->usedMaterials = 0;
    sceneRenderPreFrame(sceneRender);
}

void sceneRenderPreFrame(struct SceneRenderState* sceneRender) {
    sceneRender->usedObjects = 0;

    int i;
    for (i = 0; i < MAX_MATERIAL_BATCHES; ++i) {
        sceneRender->objectsByMaterial[i] = NULL;
    }
}

MaterialId sceneRenderMatId(struct SceneRenderState* sceneRender, Gfx* material) {
    int i;

    for (i = 0; i < sceneRender->usedMaterials; ++i) {
        if (sceneRender->materials[i] == material) {
            return i;
        }   
    }

    if (sceneRender->usedMaterials == MAX_MATERIAL_BATCHES) {
        return NO_MATERIAL;
    } else {
        sceneRender->materials[i] = material;
        return sceneRender->usedMaterials++;
    }
}

void sceneRenderDrawDynamic(struct SceneRenderState* sceneRender, Gfx* geometry, Mtx* mtx, MaterialId material) {
    if (sceneRender->usedObjects != MAX_SCENE_OBJECTS && material != NO_MATERIAL) {
        struct SceneRenderObject* next = &sceneRender->objectsToRender[sceneRender->usedObjects];

        next->matrix = *mtx;
        next->geometry = geometry;
        next->next = sceneRender->objectsByMaterial[material];
        sceneRender->objectsByMaterial[material] = next;

        ++sceneRender->usedObjects;
    }
}

Gfx* sceneRenderGenDL(struct SceneRenderState* sceneRender) {
    Gfx* dl = sceneRender->dl;

    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&sceneRender->perspectiveMtx), G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPPerspNormalize(dl++, sceneRender->perspectiveNorm);
    gSPMatrix(dl++, OS_K0_TO_PHYSICAL(&sceneRender->viewMtx), G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);

    int i;
    for (i = 0; i < sceneRender->usedMaterials; ++i) {
        if (sceneRender->objectsByMaterial[i]) {
            gSPDisplayList(dl++, sceneRender->materials[i]);

            struct SceneRenderObject* curr = sceneRender->objectsByMaterial[i];

            while (curr) {
                gSPMatrix(dl++, &curr->matrix, G_MTX_MODELVIEW|G_MTX_PUSH|G_MTX_MUL);
                gSPDisplayList(dl++, curr->geometry);
                gSPPopMatrix(dl++, G_MTX_MODELVIEW);

                curr = curr->next;
            }
        }
    }

    gSPEndDisplayList(dl);
    return dl;
}