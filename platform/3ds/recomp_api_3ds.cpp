// The Banjo-specific host functions (patches/syms.ld) for the 3DS. The
// desktop versions in src/game/recomp_api.cpp read the launcher's
// configuration; here the values come from the 3DS settings (main_3ds.cpp).
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>

#include "recomp.h"
#include "librecomp/helpers.hpp"
#include "librecomp/overlays.hpp"
#include "librecomp/addresses.hpp"
#include "librecomp/game.hpp"
#include "ultramodern/ultramodern.hpp"
#include "ultramodern/error_handling.hpp"
#include "xxHash/xxh3.h"
#include "recomp3ds.h"
#include "rt64_3ds.h"
#include "bk_settings.h"

namespace recomp3ds {
    void input_get_right_stick(float* x, float* y);   // input_hid.cpp
    void input_set_cstick_buttons(bool on);
}

namespace {

// The top screen, the "window" the desktop's aspect ratio options see.
constexpr int kScreenWidth = 400;
constexpr int kScreenHeight = 240;
constexpr float kScreenAspect = (float)kScreenWidth / (float)kScreenHeight;

constexpr uint32_t k1_to_phys(uint32_t addr) { return addr & 0x1FFFFFFF; }

void return_axes(uint8_t* rdram, recomp_context* ctx, int invert_mode) {
    s32* x_out = _arg<0, s32*>(rdram, ctx);
    s32* y_out = _arg<1, s32*>(rdram, ctx);
    *x_out = (invert_mode & 1) != 0;
    *y_out = (invert_mode & 2) != 0;
}
}

// The desktop's options, in its units unless noted (bk_settings.h), set from
// the 3DS settings menu. Defaults as on the desktop.
int g_bk_note_saving = BK_SAVING_BOTH;
int g_bk_analog_cam = 0;
int g_bk_analog_cam_sensitivity = 3;    // 1..10
int g_bk_camera_invert = 1;             // Invert X: the original game's third-person camera
int g_bk_first_person_invert = 2;       // Invert Y: the original game's
int g_bk_fly_swim_invert = 2;           // Invert Y: the original game's
int g_bk_bgm_volume = 100;              // percent

extern "C" void recomp_update_inputs(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram; (void)ctx;
}

extern "C" void recomp_puts(uint8_t* rdram, recomp_context* ctx) {
    PTR(char) cur_str = _arg<0, PTR(char)>(rdram, ctx);
    u32 length = _arg<1, u32>(rdram, ctx);
    for (u32 i = 0; i < length; i++) {
        fputc(MEM_B(i, (gpr)cur_str), stderr);
    }
}

extern "C" void recomp_exit(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram; (void)ctx;
    ultramodern::quit();
}

extern "C" void recomp_error(uint8_t* rdram, recomp_context* ctx) {
    std::string str{};
    PTR(u8) str_ptr = _arg<0, PTR(u8)>(rdram, ctx);
    for (size_t i = 0; MEM_B(str_ptr, i) != '\x00'; i++) {
        str += (char)MEM_B(str_ptr, i);
    }
    ultramodern::error_handling::message_box(str.c_str());
    ultramodern::error_handling::quick_exit(__FILE__, __LINE__, __FUNCTION__);
}

extern "C" void recomp_abort(uint8_t* rdram, recomp_context* ctx) {
    std::string msg = _arg_string<0>(rdram, ctx);
    ultramodern::error_handling::message_box(msg.c_str());
    ultramodern::error_handling::quick_exit(__FILE__, __LINE__, __FUNCTION__);
}

extern "C" void recomp_get_gyro_deltas(uint8_t* rdram, recomp_context* ctx) {
    float* x_out = _arg<0, float*>(rdram, ctx);
    float* y_out = _arg<1, float*>(rdram, ctx);
    *x_out = 0.0f; *y_out = 0.0f;
}

extern "C" void recomp_get_mouse_deltas(uint8_t* rdram, recomp_context* ctx) {
    float* x_out = _arg<0, float*>(rdram, ctx);
    float* y_out = _arg<1, float*>(rdram, ctx);
    *x_out = 0.0f; *y_out = 0.0f;
}

extern "C" void recomp_powf(uint8_t* rdram, recomp_context* ctx) {
    float a = _arg<0, float>(rdram, ctx);
    float b = ctx->f14.fl;
    _return(ctx, std::pow(a, b));
}

extern "C" void recomp_get_target_framerate(uint8_t* rdram, recomp_context* ctx) {
    int frame_divisor = _arg<0, u32>(rdram, ctx);
    _return(ctx, ultramodern::get_target_framerate(60 / frame_divisor));
}

extern "C" void recomp_get_window_resolution(uint8_t* rdram, recomp_context* ctx) {
    gpr width_out = _arg<0, PTR(u32)>(rdram, ctx);
    gpr height_out = _arg<1, PTR(u32)>(rdram, ctx);
    MEM_W(0, width_out) = kScreenWidth;
    MEM_W(0, height_out) = kScreenHeight;
}

// The desktop's Expand option: the window's shape, never narrower than the game's.
extern "C" void recomp_get_target_aspect_ratio(uint8_t* rdram, recomp_context* ctx) {
    float original = _arg<0, float>(rdram, ctx);
    _return(ctx, std::max(kScreenAspect, original));
}

// The desktop's default cutscene limit, 16:9. The screen's 5:3 is narrower,
// so cutscenes fill it (the pillarbox takes the smaller of the two).
extern "C" void recomp_get_cutscene_aspect_ratio(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return(ctx, 16.0f / 9.0f);
}

// The desktop scales convergence down in the scenes stereo_runtime.c flags
// (menus, cutscenes, first person). On the 3DS convergence is depth-driven
// only (rt64-3ds auto convergence), so the flag is ignored.
extern "C" void recomp_stereo_set_low_convergence_scene(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram; (void)ctx;
}

// The game's music volume is scaled by this factor (1.0 = unchanged).
extern "C" void recomp_get_bgm_volume(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return(ctx, (float)g_bk_bgm_volume / 100.0f);
}

extern "C" void recomp_get_analog_cam_sensitivity(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return<uint32_t>(ctx, (uint32_t)g_bk_analog_cam_sensitivity);
}

extern "C" void recomp_time_us(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return(ctx, static_cast<u32>(std::chrono::duration_cast<std::chrono::microseconds>(ultramodern::time_since_start()).count()));
}

extern "C" void recomp_load_overlays(uint8_t* rdram, recomp_context* ctx) {
    u32 rom = _arg<0, u32>(rdram, ctx);
    PTR(void) ram = _arg<1, PTR(void)>(rdram, ctx);
    u32 size = _arg<2, u32>(rdram, ctx);
    load_overlays(rom, ram, size);
}

extern "C" void recomp_load_overlays_by_rom(uint8_t* rdram, recomp_context* ctx) {
    u32 rom_addr = _arg<0, u32>(rdram, ctx);
    PTR(void) ram_addr = _arg<1, PTR(void)>(rdram, ctx);
    u32 size = _arg<2, u32>(rdram, ctx);
    load_overlays(rom_addr, ram_addr, size);
}

extern "C" void recomp_high_precision_fb_enabled(uint8_t* rdram, recomp_context* ctx) { (void)rdram; _return(ctx, static_cast<s32>(0)); }
extern "C" void recomp_get_resolution_scale(uint8_t* rdram, recomp_context* ctx) { (void)rdram; _return(ctx, 1.0f); }

// The desktop reads an unregistered "camera_invert_mode" key here, which is
// always its default, None.
extern "C" void recomp_get_inverted_axes(uint8_t* rdram, recomp_context* ctx) { return_axes(rdram, ctx, 0); }
extern "C" void recomp_get_analog_inverted_axes(uint8_t* rdram, recomp_context* ctx) { return_axes(rdram, ctx, g_bk_camera_invert); }
extern "C" void recomp_get_flying_and_swimming_inverted_axes(uint8_t* rdram, recomp_context* ctx) { return_axes(rdram, ctx, g_bk_fly_swim_invert); }
extern "C" void recomp_get_first_person_inverted_axes(uint8_t* rdram, recomp_context* ctx) { return_axes(rdram, ctx, g_bk_first_person_invert); }

extern "C" void recomp_get_analog_cam_enabled(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return<s32>(ctx, g_bk_analog_cam != 0);
}

extern "C" void recomp_get_note_saving_enabled(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return<s32>(ctx, g_bk_note_saving == BK_SAVING_NOTES || g_bk_note_saving == BK_SAVING_BOTH);
}

extern "C" void recomp_get_jinjo_saving_enabled(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    _return<s32>(ctx, g_bk_note_saving == BK_SAVING_JINJOS || g_bk_note_saving == BK_SAVING_BOTH);
}

// The analog camera's stick: the C-Stick, always (the desktop reads it
// without the suppression below).
extern "C" void recomp_get_right_analog_inputs(uint8_t* rdram, recomp_context* ctx) {
    float* x_out = _arg<0, float*>(rdram, ctx);
    float* y_out = _arg<1, float*>(rdram, ctx);
    recomp3ds::input_get_right_stick(x_out, y_out);
}

// Set by the game every frame while the analog camera may turn: the C-Stick
// then stops pressing C buttons (the desktop drops the right stick's button
// bindings). While crouching or firing eggs it presses them again.
extern "C" void recomp_set_right_analog_suppressed(uint8_t* rdram, recomp_context* ctx) {
    recomp3ds::input_set_cstick_buttons(_arg<0, s32>(rdram, ctx) == 0);
}

// The desktop runs its UI's queued callbacks here; the 3DS has none.
extern "C" void recomp_run_ui_callbacks(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram; (void)ctx;
}

extern "C" void osPiReadIo_recomp(uint8_t* rdram, recomp_context* ctx) {
    uint32_t devAddr = recomp::rom_base | ctx->r4;
    gpr dramAddr = ctx->r5;
    uint32_t physical_addr = k1_to_phys(devAddr);
    if (physical_addr > recomp::rom_base) {
        recomp::do_rom_pio(rdram, dramAddr, physical_addr);
    }
    ctx->r2 = 0;
}

extern "C" void osPfsInit_recomp(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    ctx->r2 = 11;   // PFS_ERR_DEVICE
}

extern "C" void recomp_xxh3(uint8_t* rdram, recomp_context* ctx) {
    PTR(void) data = _arg<0, PTR(void)>(rdram, ctx);
    u32 size = _arg<1, u32>(rdram, ctx);
    XXH3_state_t xxh3;
    XXH3_64bits_reset(&xxh3);
    // One byte at a time to account for the byteswapped RDRAM.
    for (size_t i = 0; i < size; i++) {
        XXH3_64bits_update(&xxh3, TO_PTR(u8, data + i), 1);
    }
    uint64_t ret = XXH3_64bits_digest(&xxh3);
    ctx->r2 = (int32_t)(ret >> 32);
    ctx->r3 = (int32_t)(ret >> 0);
}
