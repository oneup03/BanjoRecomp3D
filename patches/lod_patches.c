#include "patches.h"
#include "core1/mlmtx.h"
#include "functions.h"

typedef struct {
    s32 cmd_0;
    s32 size_4;
    f32 max_8;
    f32 min_C;
    f32 unk10[3];
    s32 subgeo_offset_1C;
}GeoCmd8;

extern f32 D_80383C98[3];
extern void func_80339124(Gfx **gfx, Mtx **mtx, BKGeoList *geo_list);
extern float gu_sqrtf(float val);

// @recomp Patched to force all LODs to act as if they were a fixed distance from the camera.
RECOMP_PATCH void func_80338B50(Gfx **gfx, Mtx **mtx, void *arg2) {
    GeoCmd8 *cmd = (GeoCmd8 *)arg2;
    f32 dist;

    if (cmd->subgeo_offset_1C) {
        mlMtx_apply_vec3f(D_80383C98, cmd->unk10);
#ifdef BK_3DS
        // @recomp 3DS: the original distance-based LOD; the far models save the console work.
        dist = gu_sqrtf(D_80383C98[0] * D_80383C98[0] + D_80383C98[1] * D_80383C98[1] + D_80383C98[2] * D_80383C98[2]);
#else
        // @recomp Force distance for LOD to be 50.
        dist = 50.0f; //gu_sqrtf(D_80383C98[0] * D_80383C98[0] + D_80383C98[1] * D_80383C98[1] + D_80383C98[2] * D_80383C98[2]);
#endif
        if (cmd->min_C < dist && dist <= cmd->max_8) {
            func_80339124(gfx, mtx, (BKGeoList *)((s32)cmd + cmd->subgeo_offset_1C));
        }
    }
}
