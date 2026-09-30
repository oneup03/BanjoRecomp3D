// Banjo: Recompiled, Nintendo 3DS entry point.
#include <cinttypes>
#include <cstdio>
#include <string>

extern "C" {
#include <3ds/types.h>
#include <3ds/services/hid.h>
}

#include "recomp3ds.h"
#include "ovl_patches.hpp"
#include "banjo_game.h"
#include "recomp_data.h"
#include "bk_settings.h"
#include "librecomp/rsp.hpp"
#include "librecomp/overlays.hpp"
#include "librecomp/game.hpp"
#include "ultramodern/ultra64.h"

extern "C" void recomp_entrypoint(uint8_t* rdram, recomp_context* ctx);
gpr get_entrypoint_address();
extern RspUcodeFunc n_aspMain;

namespace {

// A=A  B=B  L=Z  R=R  START=START; the C buttons on X (up), Y (down),
// ZL (left), ZR (right), the D-pad and the C-Stick. Banjo has no use for the
// N64 D-pad or L.
using namespace recomp3ds;
const ButtonMap kButtons[] = {
    { KEY_A, N64_A }, { KEY_B, N64_B }, { KEY_L, N64_Z }, { KEY_R, N64_R }, { KEY_START, N64_START },
    { KEY_X | KEY_DUP, N64_CUP }, { KEY_Y | KEY_DDOWN, N64_CDOWN },
    { KEY_ZL | KEY_DLEFT, N64_CLEFT }, { KEY_ZR | KEY_DRIGHT, N64_CRIGHT },
};

void analog_cam_changed(int on) {
    // The analog camera reads the C-Stick itself; its C buttons would also
    // turn and zoom the camera.
    input_set_cstick_buttons(on == 0);
}

const char* const kSavingNames[] = { "Off", "Notes", "Jinjos", "Both" };
const char* const kInvertNames[] = { "None", "X", "Y", "Both" };
const char* const kOffOnNames[] = { "Off", "On" };
// The desktop's options on the 3DS menu's pages, with the desktop's defaults.
const MenuOption kOptions[] = {
    { "analog_cam", "Analog camera", 0, 1, &g_bk_analog_cam, kOffOnNames, analog_cam_changed, PageControls },
    { "analog_cam_speed", "Analog speed", 1, 10, &g_bk_analog_cam_sensitivity, nullptr, nullptr, PageControls },
    { "camera_invert", "Camera invert", 0, 3, &g_bk_camera_invert, kInvertNames, nullptr, PageControls },
    { "first_person_invert", "Aim invert", 0, 3, &g_bk_first_person_invert, kInvertNames, nullptr, PageControls },
    { "fly_swim_invert", "Fly/swim inv.", 0, 3, &g_bk_fly_swim_invert, kInvertNames, nullptr, PageControls },
    { "note_saving", "Note saving", 0, 3, &g_bk_note_saving, kSavingNames, nullptr, PageGame },
    { "bgm_volume", "Music volume", 0, 100, &g_bk_bgm_volume, nullptr, nullptr, PageGame, 5, "%" },
};

// Defaults for the library's rows; settings.ini, once written, keeps what the
// player chose.
const MenuDefault kDefaults[] = {
    { "separation", 50 },               // 3D depth
    { "convergence_tenths", 100 },      // 10.0
    { "stereo_ghost_contrast", 80 },    // percent
    { "stick_deadzone", 15 },           // percent
    { "cstick_deadzone", 20 },          // percent
};

RspUcodeFunc* get_rsp_microcode(const OSTask* task) {
    switch (task->t.type) {
        case M_AUDTASK:
            return n_aspMain;
        default:
            fprintf(stderr, "Unknown RSP task: %" PRIu32 "\n", task->t.type);
            return nullptr;
    }
}

std::string get_game_thread_name(const OSThread* t) {
    std::string name = "[Game] ";
    switch (t->id) {
        case 0:  name += (t->priority == 150) ? "PIMGR" : (t->priority == 80) ? "VIMGR" : std::to_string(t->id); break;
        case 1:  name += "INIT"; break;
        case 2:  name += "DEFRAG"; break;
        case 4:  name += "AUDIO"; break;
        case 5:  name += "RESET"; break;
        case 6:  name += "MAIN"; break;
        case 7:  name += "CONT"; break;
        case 8:  name += "RUMBLE"; break;
        default: name += std::to_string(t->id); break;
    }
    return name;
}

// Projection-group ids the Banjo patches use (patches/transform_ids.h) and
// the owner's stereo classification of them (rt64-3D Stereo3D branch,
// rt64_projection_processor.cpp).
constexpr uint32_t PROJECTION_GAMEPLAY = 0x1000;
constexpr uint32_t PROJECTION_SKYBOX = 0x1001;
constexpr uint32_t PROJECTION_TRANSITION = 0x1002;
constexpr uint32_t PROJECTION_HUD_FIRST = 0x1003;     // press start .. the end
constexpr uint32_t PROJECTION_HUD_LAST = 0x1008;
constexpr uint32_t PROJECTION_PILLARBOX = 0x1009;
constexpr uint32_t PROJECTION_DIALOG_BUBBLE = 0x100A;
constexpr uint32_t PROJECTION_PORTRAIT_FIRST = 0x1100;
constexpr uint32_t PROJECTION_PORTRAIT_LAST = 0x11FF;

using rt64_3ds::ProjKind;
using rt64_3ds::StereoClass;
const rt64_3ds::StereoRule bk_stereo_rules[] = {
    { PROJECTION_GAMEPLAY,       PROJECTION_GAMEPLAY,       ProjKind::Perspective, StereoClass::World },
    { PROJECTION_SKYBOX,         PROJECTION_SKYBOX,         ProjKind::Perspective, StereoClass::Infinity },
    { PROJECTION_TRANSITION,     PROJECTION_TRANSITION,     ProjKind::Perspective, StereoClass::None },
    { PROJECTION_HUD_FIRST,      PROJECTION_HUD_LAST,       ProjKind::Any,         StereoClass::Hud },
    { PROJECTION_PILLARBOX,      PROJECTION_PILLARBOX,      ProjKind::Any,         StereoClass::Hud },
    { PROJECTION_DIALOG_BUBBLE,  PROJECTION_DIALOG_BUBBLE,  ProjKind::Perspective, StereoClass::HudBubble },
    { PROJECTION_PORTRAIT_FIRST, PROJECTION_PORTRAIT_LAST,  ProjKind::Any,         StereoClass::Hud },
};

}   // namespace

#define REGISTER_FUNC(name) recomp::overlays::register_base_export(#name, name)
extern "C" {
    void recomp_get_window_resolution(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_target_aspect_ratio(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_target_framerate(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_cutscene_aspect_ratio(uint8_t* rdram, recomp_context* ctx);
    void recomp_stereo_set_low_convergence_scene(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_analog_cam_enabled(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_right_analog_inputs(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_bgm_volume(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_inverted_axes(uint8_t* rdram, recomp_context* ctx);
    void recomp_get_analog_inverted_axes(uint8_t* rdram, recomp_context* ctx);
}

// RecompiledMods/static_mods.cpp (rt64-3ds tools/mod_bake): the mods built into the executable.
void recomp_register_static_mods();

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    // The host functions the desktop exports to mods (src/main/main.cpp).
    REGISTER_FUNC(recomp_get_window_resolution);
    REGISTER_FUNC(recomp_get_target_aspect_ratio);
    REGISTER_FUNC(recomp_get_target_framerate);
    REGISTER_FUNC(recomp_get_cutscene_aspect_ratio);
    REGISTER_FUNC(recomp_stereo_set_low_convergence_scene);
    REGISTER_FUNC(recomp_get_analog_cam_enabled);
    REGISTER_FUNC(recomp_get_right_analog_inputs);
    REGISTER_FUNC(recomp_get_bgm_volume);
    REGISTER_FUNC(recomp_get_inverted_axes);
    REGISTER_FUNC(recomp_get_analog_inverted_axes);
    recomputil::register_data_api_exports();
    // Extension data for two object types: Props and ActorMarkers.
    recomputil::init_extended_object_data(2);
    recomp_register_static_mods();

    recomp3ds::GameDesc desc{};
    desc.game_id = u8"BK";
    // The desktop's version (src/main/main.cpp): mods check it.
    recomp::Version::from_string("1.0.2", desc.project_version);
    desc.sd_dir = "BanjoRecomp";
    desc.entry = recomp::GameEntry{
        .rom_hash = 0x1B67585D56E07F8CULL,
        .internal_name = "Banjo-Kazooie",
        .display_name = "Banjo-Kazooie",
        .game_id = u8"BK",
        .mod_game_id = "bk",               // mods baked into the executable (BAKE_MODS)
        // Eep16k rather than Eep4k: room for the note and jinjo saving data.
        .save_type = recomp::SaveType::Eep16k,
        .thumbnail_bytes = {},
        .is_enabled = false,
        .decompression_routine = nullptr,  // only mod hooking needs it
        .has_compressed_code = true,
        .entrypoint_address = get_entrypoint_address(),
        .entrypoint = recomp_entrypoint,
        .on_init_callback = banjo::bk_on_init,
    };
    desc.rsp.get_rsp_microcode = get_rsp_microcode;
    desc.register_overlays = banjo::register_bk_overlays;
    desc.register_patches = banjo::register_bk_patches;
    desc.get_game_thread_name = get_game_thread_name;

    desc.render.game_name = "Banjo-Kazooie";
    desc.render.gbi = rt64_3ds::Gbi::F3DEX;
    // The game's own frame (DEFAULT_FRAMEBUFFER_WIDTH x HEIGHT), placed on the
    // top screen as RT64 places it; the patches widen it to the screen's 5:3.
    desc.render.vi_width = 292;
    desc.render.vi_height = 216;
    desc.render.rules = bk_stereo_rules;
    desc.render.rule_count = sizeof(bk_stereo_rules) / sizeof(bk_stereo_rules[0]);
    desc.render.unclassified_persp = StereoClass::None;
    desc.render.unclassified_ortho = StereoClass::Hud;
    desc.render.frame_head_proj_id = PROJECTION_GAMEPLAY;
    // The jiggy transition and the picture puzzles render the world into an
    // image of their own and sample it; the pause copies the screen.
    desc.render.render_to_texture = true;
    desc.render.hud_k_persp = 1.5455f;     // the desktop's perspective HUD shift for Banjo
    // The original game's level of detail (gSPBranchLessZ), not the desktop's
    // always-near models: the far ones save the console work. The patches
    // keep the original culling and model LOD too (BK_3DS).
    desc.render.force_lod_branch = false;
    // Auto convergence that keeps up with cutscenes: close-ups often sit low
    // in the frame and cut often, at 20 fps. Sample most of the frame's
    // height, filter over fewer frames and follow faster than DK64 does.
    desc.render.autoconv.region_bottom = 0.9f;
    desc.render.autoconv.history = 5;
    desc.render.autoconv.approach = 0.4f;
    desc.render.autoconv.retreat = 0.08f;
    desc.render.autoconv.cut_ratio = 1.8f;
    desc.render.autoconv.ease_in = 0.25f;
    desc.render.autoconv.ease_out = 0.08f;
    desc.audio_hle = true;      // n_aspMain interpreted on the CPU (rt64-3ds naudio_hle.cpp)
    desc.audio_hle_variant = recomp3ds::NaudioVariant::BanjoKazooie;   // no phase-inverted voices in this build
    desc.button_map = kButtons;
    desc.button_map_count = sizeof(kButtons) / sizeof(kButtons[0]);
    desc.menu_options = kOptions;
    desc.menu_option_count = sizeof(kOptions) / sizeof(kOptions[0]);
    desc.menu_defaults = kDefaults;
    desc.menu_default_count = sizeof(kDefaults) / sizeof(kDefaults[0]);
#ifdef BK_NULL_RENDERER
    desc.null_renderer = true;
#endif

    return recomp3ds::run(desc);
}
