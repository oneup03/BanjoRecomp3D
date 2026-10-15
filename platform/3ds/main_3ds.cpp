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
#include "librecomp/mods.hpp"
#include "ultramodern/ultra64.h"

extern "C" void recomp_entrypoint(uint8_t* rdram, recomp_context* ctx);
gpr get_entrypoint_address();
extern RspUcodeFunc n_aspMain;

namespace {

// A=A  B=B  L=Z  R=R  START=START; the C buttons on X (up), Y (down),
// ZL (left), ZR (right), the D-pad and the C-Stick. Banjo has no use for the
// N64 D-pad or L, but some built-in mods do: the D-pad's down for Banjo Bee
// (its bee flight), all four directions for Transform Anywhere, and L for
// Ability Anywhere (L + A), Cheats (hold L to moon jump) and Transform
// Anywhere (its "L + D-pad" mode). While one of them is on, those buttons are
// either on the touch screen (the default: the buttons stay as they are),
// or on the buttons themselves ("Mod buttons" on the Controls page):
// - the D-pad directions a mod reads become the N64 D-pad's;
// - ZL becomes N64 L.
// The C buttons stay on X, Y, ZR and the C-Stick (and ZL or the D-pad where
// those are free). An Old 3DS has no ZL, ZR or C-Stick: with Transform
// Anywhere on the buttons, its camera can't turn with the C buttons.
using namespace recomp3ds;
constexpr const char* kBanjoBee = "bk_recomp_mod_banjo_bee";
constexpr const char* kTransformAnywhere = "ProxyMM_TransformAnywhere";
constexpr const char* kAbilityAnywhere = "ProxyMM_AbilityAnywhere";
constexpr const char* kCheats = "ProxyMM_Cheats";

enum DpadUse { DpadNone, DpadDown, DpadAll };
struct Buttons {
    ButtonMap map[16];
    size_t count = 0;
};
// Every combination, built once before the game reads them.
Buttons g_buttons[3][2];    // [DpadUse][ZL is N64 L]

void build_buttons(Buttons& b, DpadUse d, bool zl_is_l) {
    auto add = [&](uint32_t keys, uint16_t n64) { if (keys != 0) b.map[b.count++] = { keys, n64 }; };
    add(KEY_A, N64_A); add(KEY_B, N64_B); add(KEY_L, N64_Z); add(KEY_R, N64_R); add(KEY_START, N64_START);
    add(KEY_X | (d == DpadAll ? 0 : KEY_DUP), N64_CUP);
    add(KEY_Y | (d == DpadNone ? KEY_DDOWN : 0), N64_CDOWN);
    add((zl_is_l ? 0 : KEY_ZL) | (d == DpadAll ? 0 : KEY_DLEFT), N64_CLEFT);
    add(KEY_ZR | (d == DpadAll ? 0 : KEY_DRIGHT), N64_CRIGHT);
    if (zl_is_l) add(KEY_ZL, N64_L);
    if (d != DpadNone) add(KEY_DDOWN, N64_DDOWN);
    if (d == DpadAll) { add(KEY_DUP, N64_DUP); add(KEY_DLEFT, N64_DLEFT); add(KEY_DRIGHT, N64_DRIGHT); }
}

const char* const kModButtonNames[] = { "Touch screen", "D-pad, ZL" };
int g_mod_buttons = 0;      // 0: on the touch screen, 1: the D-pad and ZL

// The buttons for the mods on now: at the game's start, whenever one is
// switched on the Mods page (it takes effect at once), and when "Mod
// buttons" changes.
void set_buttons() {
    auto on = [](const char* id) { return recomp::mods::is_mod_enabled(id); };
    const bool transform = on(kTransformAnywhere);
    const DpadUse d = transform ? DpadAll : on(kBanjoBee) ? DpadDown : DpadNone;
    const bool need_l = transform || on(kAbilityAnywhere) || on(kCheats);
    if (g_mod_buttons == 0) {
        const Buttons& b = g_buttons[DpadNone][0];
        input_set_map(b.map, b.count);
        const uint16_t dpad = d == DpadAll ? (N64_DUP | N64_DDOWN | N64_DLEFT | N64_DRIGHT) : d == DpadDown ? N64_DDOWN : 0;
        touch_pad_set(dpad | (need_l ? N64_L : 0));
    }
    else {
        const Buttons& b = g_buttons[d][need_l ? 1 : 0];
        input_set_map(b.map, b.count);
        touch_pad_set(0);
    }
}
void mod_buttons_changed(int) { set_buttons(); }
void bk_on_init_3ds(uint8_t* rdram, recomp_context* ctx) {
    banjo::bk_on_init(rdram, ctx);
    set_buttons();
}
void mod_switched(const char*, bool) {
    set_buttons();
}

const char* const kSavingNames[] = { "Off", "Notes", "Jinjos", "Both" };
const char* const kInvertNames[] = { "None", "X", "Y", "Both" };
const char* const kOffOnNames[] = { "Off", "On" };
const char* const kParticleNames[] = { "Full", "Half", "Quarter" };
// The desktop's options on the 3DS menu's pages, with the desktop's defaults.
const MenuOption kOptions[] = {
    // The game turns the C-Stick's C buttons off itself while the analog
    // camera may turn (recomp_set_right_analog_suppressed).
    { "analog_cam", "Analog camera", 0, 1, &g_bk_analog_cam, kOffOnNames, nullptr, PageControls },
    { "analog_cam_speed", "Analog camera sensitivity", 1, 10, &g_bk_analog_cam_sensitivity, nullptr, nullptr, PageControls },
    { "mod_buttons", "Mod buttons", 0, 1, &g_mod_buttons, kModButtonNames, mod_buttons_changed, PageControls },
    { "camera_invert", "Invert camera", 0, 3, &g_bk_camera_invert, kInvertNames, nullptr, PageControls },
    { "first_person_invert", "Invert first person view", 0, 3, &g_bk_first_person_invert, kInvertNames, nullptr, PageControls },
    { "fly_swim_invert", "Invert flying & swimming", 0, 3, &g_bk_fly_swim_invert, kInvertNames, nullptr, PageControls },
    { "note_saving", "Note & Jinjo saving", 0, 3, &g_bk_note_saving, kSavingNames, nullptr, PageGame },
    { "bgm_volume", "Music volume", 0, 100, &g_bk_bgm_volume, nullptr, nullptr, PageGame, 5, "%" },
    // Particle emission divided by 1, 2 or 4 (patches/particle_patches.c):
    // each particle is a draw of its own, which an Old 3DS can't afford by
    // the hundred.
    { "particles", "Particles", 0, 2, &g_bk_particles, kParticleNames, nullptr, PageGame },
};

// Built-in mods left out of the game modes (and off their Mods page): Exit
// to Lair returns to the original game's lair entrances, which the hacks
// rework (Nostalgia 64 replaces 60-odd of the lair's warps). No built-in mod
// replaces a function a game mode replaces (the mode would fail to load).
const char* const kModsNotInGameModes[] = { "ProxyMM_ExitToLair" };

// Defaults for the library's rows; settings.ini, once written, keeps what the
// player chose.
const MenuDefault kDefaults[] = {
    { "separation", 50 },               // 3D depth
    { "convergence_tenths", 100 },      // 10.0
    { "stereo_ghost_contrast", 100 },   // percent: 100 is ghost reduction off
    { "stick_deadzone", 15 },           // percent
    { "cstick_deadzone", 20 },          // percent
    // C-Up is Banjo's first-person view: a nudge of the C-Stick shouldn't
    // enter it (X and D-pad up still press C-Up).
    { "cstick_up", 0 },
    // "60 interp." on a New 3DS; an Old 3DS starts at the game's own rate
    // instead (GameDesc::old3ds_frame_cap below), its cores having no room
    // for in-betweens.
    { "frame_cap", 2 },
    // A quarter of the particles on an Old 3DS (the row above).
    { "particles", 2, MenuDefault::Old3dsOnly },
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
    desc.mods_not_in_game_modes = kModsNotInGameModes;
    desc.on_mod_switched = mod_switched;
    desc.mods_not_in_game_modes_count = sizeof(kModsNotInGameModes) / sizeof(kModsNotInGameModes[0]);
    desc.sd_dir = "BanjoRecomp";
    // The ROM (16 MB) is held in memory on a New 3DS. An Old 3DS streams it
    // from the card: the CIA's heap there is 37.6 MB, and the game peaked at
    // 30.3 MB of it with a 2 MB stream cache (RDRAM 16 MB, about 12 MB for
    // the rest), so the ROM beside RDRAM would need 44 MB. Said outright
    // rather than left to the heap test, so Azahar's Old 3DS (a 72 MB heap)
    // streams like the console; rom_room_mb is the measured rest, for the
    // log's figures and settings.ini's rom_stream. The cache is what the
    // heap can spare with room to grow. A level load reads its data once
    // (whole minutes pass without a read), so a bigger cache only saves the
    // revisits. A game mode's 64 MB ROM is streamed on both consoles
    // through the same cache.
    desc.rom_new3ds = recomp3ds::GameDesc::RomPlace::Memory;
    desc.rom_old3ds = recomp3ds::GameDesc::RomPlace::Stream;
    desc.rom_room_mb = 13;
    desc.rom_cache_mb = 4;
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
        .on_init_callback = bk_on_init_3ds,
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
    // The patches tag the cameras, models and bones for RT64's high frame
    // rate (transform_tagging.c): offer "60 interp." on the Frame rate row,
    // interpolated frames between each two of the game's: one while it runs
    // at 30, two (at 1/3 and 2/3) in the cutscenes at 20. Frames that render
    // to a texture (the jiggy transition, the puzzles) go without. The
    // in-betweens draw into eye targets of their own (double_eyes, 1500 KB of
    // VRAM: RGBA8 and D24S8) so the game's frame of each pair is recorded
    // while the GPU still draws its in-betweens. settings.ini's
    // gpu_double_eyes switches it.
    desc.render.frame_interpolation = true;
    desc.render.double_eyes = true;
    // The game's frame limiter waits for the last list to be drawn (the RDP's
    // completion) every frame: it hears that once the renderer has read the
    // list, not after the renderer's waits for the screen, which would make
    // it miss retraces (and its cutscenes fall behind the music).
    desc.render.early_drawn = true;
    // The GPU's fill is the limit in the busiest levels (Treasure Trove Cove:
    // about 5 screens of fragments an eye, 10 ms an eye): 16-bit eye depth,
    // I/IA textures as LA8, and the frame's leading full-screen fill (Banjo
    // starts every frame with one) turned into the eyes' clear. Same pictures.
    desc.render.eye_depth16 = true;
    desc.render.intensity_textures16 = true;
    desc.render.fill_clear = true;
    desc.audio_hle = true;      // n_aspMain interpreted on the CPU (rt64-3ds naudio_hle.cpp)
    desc.audio_hle_variant = recomp3ds::NaudioVariant::BanjoKazooie;   // no phase-inverted voices in this build
    // The Audio engine row's DSP mode (rt64-3ds dsp_synth.h): the voices
    // play on DSP channels, as banjo.us.rev0.toml's n_alSyn* hooks report
    // them, and the audio task is skipped. Rare's N_ALSynth has player
    // slots before the stock fields (each 0xC later): its clock at
    // +0x28/+0x2C, its free updates at +0x38, its lame voices at +0x20, its
    // aux bus at +0x40; n_syn (the synthesizer's pointer) at 0x80276E84.
    // banjo.us.rev0.toml's n_alAudioFrame hook at 0x8025C974 then skips the
    // command list the skipped task would have run.
    desc.synth = recomp3ds::SynthFlavor::NAudio;
    desc.synth.synth_global = 0x80276E84;
    desc.synth.param_samples = 0x28;
    desc.synth.cur_samples = 0x2C;
    desc.synth.aux_bus = 0x40;
    desc.synth.param_list = 0x38;
    desc.synth.lame_list = 0x20;
    // With two cores (an Old 3DS) the game takes core 0, which the
    // application has whole, and the renderer and audio core 1's 80%: at the
    // Old clock the game's own work filled core 1 (13-20 lists a second
    // against the 30 asked) while core 0 sat about 55% busy. settings.ini's
    // dbg_cores=1 is the other layout ("2, game on 1"), for comparison.
    desc.two_cores_game_on_core0 = true;
    // game_draw waits on "parsed" every frame, so core 0 sits idle while
    // core 1 reads the list (26 ms there against about 21 on core 0): the
    // read goes to core 0, and the triangles' output and the drawing stay on
    // core 1. At the Old clock, over the attract demos: 17.94 lists a second
    // without it, 19.40 with it.
    desc.two_cores_read_on_game_core = true;
    for (int d = 0; d < 3; d++) {
        for (int l = 0; l < 2; l++) build_buttons(g_buttons[d][l], (DpadUse)d, l != 0);
    }
    desc.button_map = g_buttons[DpadNone][0].map;
    desc.button_map_count = g_buttons[DpadNone][0].count;
    desc.menu_options = kOptions;
    desc.menu_option_count = sizeof(kOptions) / sizeof(kOptions[0]);
    desc.menu_defaults = kDefaults;
    desc.menu_default_count = sizeof(kDefaults) / sizeof(kDefaults[0]);
    // On an Old 3DS the "60 interp." default becomes the game's own rate.
    desc.old3ds_frame_cap = 0;
#ifdef BK_NULL_RENDERER
    desc.null_renderer = true;
#endif

    return recomp3ds::run(desc);
}
