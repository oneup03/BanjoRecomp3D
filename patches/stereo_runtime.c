// Auto-convergence runtime tick. Once per frame, examines BK's current state
// (game mode, current map, first-person camera state) and pushes a bool to
// the host indicating whether the scene should use reduced stereo convergence.
// The host owns the actual scaling — this patch just classifies scenes.

#include "patches.h"
#include "graphics.h"

extern s32 getGameMode(void);

// Avoid calling map_get() / ncFirstPersonCamera_getState() here — they live in
// core2's overlay and may not be loaded when this tick fires from core1's
// mainLoop on the very first frames after boot, which crashes. Read the
// underlying BSS fields directly: before core2 initializes them they're just
// zeros (not in the low-convergence list, state==0 means "not in FP"), so
// the early-boot reads are safe.
extern struct {
    s32 unk0;
    s32 map_4;
    s32 unk8;
} D_803835D0;

// First-person camera struct lives at 0x8037DC60 in BSS. The state byte sits
// at offset 76 (6 × f32[3] = 72 bytes of vectors, then a f32 timer = 76).
extern struct {
    f32 position[3];
    f32 rotation[3];
    f32 zoomed_in_position[3];
    f32 zoomed_in_rotation[3];
    f32 zoomed_out_position[3];
    f32 zoomed_out_rotation[3];
    f32 transition_timer;
    u8  state;
} D_8037DC60;

// In BK's NTSC US 1.0 build, GAME_MODE_3_NORMAL covers far more than just
// gameplay: opening FMV, file select, and game-over FMV all run under it.
// The map ID is what distinguishes those from the playable world. We treat
// menu shells (file select, Dingpot) and any cutscene map as "low convergence"
// while leaving normal world maps alone.
// Match this list to pillarbox_patches.c's pillarbox_active() exactly, plus the
// file-select map. Anything outside that list can be repurposed by mods for
// real gameplay, and we don't want auto-conv quietly dimming the depth on a
// mod-authored level just because it happens to live on (e.g.) MAP_94's slot.
// File select (0x91) is intrinsically a menu — mods don't put gameplay there.
static int map_is_low_convergence(enum map_e map) {
    switch (map) {
        case MAP_91_FILE_SELECT:
        // "_CS_" (cutscene) maps — kept in sync with pillarbox_active().
        case MAP_1E_CS_START_NINTENDO:
        case MAP_1F_CS_START_RAREWARE:
        case MAP_20_CS_END_NOT_100:
        case MAP_7B_CS_INTRO_GL_DINGPOT_1:
        case MAP_7C_CS_INTRO_BANJOS_HOUSE_1:
        case MAP_7D_CS_SPIRAL_MOUNTAIN_1:
        case MAP_7E_CS_SPIRAL_MOUNTAIN_2:
        case MAP_81_CS_INTRO_GL_DINGPOT_2:
        case MAP_82_CS_ENTERING_GL_MACHINE_ROOM:
        case MAP_83_CS_GAME_OVER_MACHINE_ROOM:
        case MAP_84_CS_UNUSED_MACHINE_ROOM:
        case MAP_85_CS_SPIRAL_MOUNTAIN_3:
        case MAP_86_CS_SPIRAL_MOUNTAIN_4:
        case MAP_87_CS_SPIRAL_MOUNTAIN_5:
        case MAP_88_CS_SPIRAL_MOUNTAIN_6:
        case MAP_89_CS_INTRO_BANJOS_HOUSE_2:
        case MAP_8A_CS_INTRO_BANJOS_HOUSE_3:
        case MAP_94_CS_INTRO_SPIRAL_7:
        case MAP_95_CS_END_ALL_100:
        case MAP_96_CS_END_BEACH_1:
        case MAP_97_CS_END_BEACH_2:
        case MAP_98_CS_END_SPIRAL_MOUNTAIN_1:
        case MAP_99_CS_END_SPIRAL_MOUNTAIN_2:
            return 1;
        default:
            return 0;
    }
}

static int stereo_scene_wants_low_convergence(void) {
    const s32 mode = getGameMode();
    // Modes that are entirely menu/FMV regardless of map.
    if (mode == GAME_MODE_7_ATTRACT_DEMO ||
        mode == GAME_MODE_6_FILE_PLAYBACK ||
        mode == GAME_MODE_8_BOTTLES_BONUS ||
        mode == GAME_MODE_9_BANJO_AND_KAZOOIE ||
        mode == GAME_MODE_A_SNS_PICTURE) {
        return 1;
    }
    // First-person camera: state 1 (entering) and 2 (idle / zoomed in) both
    // frame Banjo's body right at the camera plane, so the gameplay
    // convergence reads as too strong. State 3/4 are exit/done — let those
    // pop back to full convergence. State 0 (uninitialized BSS or never
    // entered FP) is also full convergence.
    const u8 fpState = D_8037DC60.state;
    if (fpState == FIRSTPERSON_STATE_1_ENTER ||
        fpState == FIRSTPERSON_STATE_2_IDLE) {
        return 1;
    }
    // Mode 3 (and others) — fall back to per-map classification so file-
    // select and cutscene maps still get the reduced convergence.
    return map_is_low_convergence((enum map_e)D_803835D0.map_4);
}

void stereo_runtime_tick(void) {
    recomp_stereo_set_low_convergence_scene(stereo_scene_wants_low_convergence());
}

// Patch the tiny per-frame helper that mainLoop calls unconditionally on the
// first line of every frame. We hook here (not at func_802E39D0 / game_draw)
// because game_draw only fires during normal gameplay state — FMVs, file
// select, and game-over cutscenes never reach it, so the auto-convergence
// signal would never update for those scenes. mainLoop runs unconditionally
// across every game mode, so this catches them all.
extern void func_8033BD6C(void);
extern void func_80255198(void);

RECOMP_PATCH void func_8023DA74(void) {
    stereo_runtime_tick();
    func_8033BD6C();
    func_80255198();
}
