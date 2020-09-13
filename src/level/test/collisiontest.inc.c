
#include "src/collision/collisionmesh.h"

Vtx test_CollisionTest_vtx[4] = {
	{{{-256, 256, -256},0, {0, 0},{0xFF, 0, 0, 0xFF}}},
	{{{256, 256, -256},0, {1, 0},{0xFF, 0, 0, 0xFF}}},
	{{{256, 256, 256},0, {1, 1},{0xFF, 0, 0, 0xFF}}},
	{{{-256, 256, 256},0, {0, 1},{0xFF, 0, 0, 0xFF}}},
};

Gfx test_CollisionTest_dl[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_ADDRGB, G_CC_PRIMITIVE),
    gsDPSetPrimColor(0, 0, 0xff, 0, 0, 0xff),
    gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),
	gsSPVertex(test_CollisionTest_vtx, 4, 0),
    gsSP1Triangle(0, 1, 2, 0),
    gsDPSetPrimColor(0, 0, 0, 0xff, 0, 0xff),
    gsSP1Triangle(0, 2, 3, 0),
    gsSPEndDisplayList(),
};
