#include "banjo_config.h"
#include "recompui/recompui.h"
#include "recompui/config.h"
#include "recompui/renderer.h"
#include "recompinput/recompinput.h"
#include "banjo_sound.h"
#include "banjo_support.h"
#include "ultramodern/config.hpp"
#include "librecomp/files.hpp"
#include "librecomp/config.hpp"
#include "util/file.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>

#if defined(_WIN32)
#include <Shlobj.h>
#elif defined(__linux__)
#include <unistd.h>
#include <pwd.h>
#elif defined(__APPLE__)
#include "apple/rt64_apple.h"
#endif

static void add_general_options(recomp::config::Config &config) {
    using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;

    static EnumOptionVector note_saving_mode_options = {
        {banjo::NoteSavingMode::Off,        "Off",    "Off"},
        {banjo::NoteSavingMode::OnlyNotes,  "Notes",  "Only Notes"},
        {banjo::NoteSavingMode::OnlyJinjos, "Jinjos", "Only Jinjos"},
        // Keeping as "On" to preserve compatibility with previous configs before jinjo saving
        {banjo::NoteSavingMode::Both,       "On",     "Both"},
    };
    config.add_enum_option(
        banjo::configkeys::general::note_saving_mode,
        "Note & Jinjo Saving",
        "Saves collected notes and jinjos so that you don't need to collect them again when revisiting a level. <recomp-color primary>Both</recomp-color> is the default, while <recomp-color primary>off</recomp-color> matches the original game.<br /><br />Changing this setting will only take effect when entering/exiting a level, or restarting the game.",
        note_saving_mode_options,
        banjo::NoteSavingMode::Both
    );
    static EnumOptionVector analog_cam_mode_options = {
        {banjo::AnalogCamMode::Off, "Off", "Off"},
        {banjo::AnalogCamMode::On, "On", "On"},
    };
    config.add_enum_option(
        banjo::configkeys::general::analog_cam_mode,
        "Analog Camera",
        "Enables the analog camera.",
        analog_cam_mode_options,
        banjo::AnalogCamMode::Off
    );
    config.add_number_option(
        banjo::configkeys::general::analog_camera_sensitivity,
        "Analog Camera Sensitivity",
        "Sets the sensitivity of the right stick analog camera, if enabled.",
        1, 10, 1, 0, false, 3
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::general::analog_camera_sensitivity,
        banjo::configkeys::general::analog_cam_mode,
        banjo::AnalogCamMode::Off
    );
    static EnumOptionVector camera_invert_mode_options = {
        {banjo::CameraInvertMode::InvertNone, "InvertNone", "None"},
        {banjo::CameraInvertMode::InvertX, "InvertX", "Invert X"},
        {banjo::CameraInvertMode::InvertY, "InvertY", "Invert Y"},
        {banjo::CameraInvertMode::InvertBoth, "InvertBoth", "Invert Both"}
    };
    config.add_enum_option(
        banjo::configkeys::general::third_person_camera_invert_mode,
        "Invert Camera",
        "Inverts the camera controls for the third person camera if it's enabled. <recomp-color primary>Invert X</recomp-color> is the default and matches the original game.<br /><br />If analog camera is off, only the <recomp-color primary>Invert X</recomp-color> setting will take effect.",
        camera_invert_mode_options,
        banjo::CameraInvertMode::InvertX
    );
    static EnumOptionVector first_person_invert_mode_options = {
        {banjo::CameraInvertMode::InvertNone, "InvertNone", "None"},
        {banjo::CameraInvertMode::InvertX, "InvertX", "Invert X"},
        {banjo::CameraInvertMode::InvertY, "InvertY", "Invert Y"},
        {banjo::CameraInvertMode::InvertBoth, "InvertBoth", "Invert Both"}
    };
    config.add_enum_option(
        banjo::configkeys::general::first_person_invert_mode,
        "Invert First Person View",
        "Inverts the camera controls in first person view. <recomp-color primary>Invert Y</recomp-color> is the default and matches the original game.",
        first_person_invert_mode_options,
        banjo::CameraInvertMode::InvertY
    );
    static EnumOptionVector flying_and_swimming_invert_options = {
        {banjo::CameraInvertMode::InvertNone, "InvertNone", "None"},
        {banjo::CameraInvertMode::InvertX, "InvertX", "Invert X"},
        {banjo::CameraInvertMode::InvertY, "InvertY", "Invert Y"},
        {banjo::CameraInvertMode::InvertBoth, "InvertBoth", "Invert Both"}
    };
    config.add_enum_option(
        banjo::configkeys::general::flying_and_swimming_invert_mode,
        "Invert Flying & Swimming",
        "Inverts the controls for swimming and flying. <recomp-color primary>Invert Y</recomp-color> is the default and matches the original game.",
        flying_and_swimming_invert_options,
        banjo::CameraInvertMode::InvertY
    );
}

template <typename T = uint32_t>
T get_general_config_enum_value(const std::string& option_id) {
    return static_cast<T>(std::get<uint32_t>(recompui::config::get_general_config().get_option_value(option_id)));
}

template <typename T = uint32_t>
T get_general_config_number_value(const std::string& option_id) {
    return static_cast<T>(std::get<double>(recompui::config::get_general_config().get_option_value(option_id)));
}

banjo::NoteSavingMode banjo::get_note_saving_mode() {
    return get_general_config_enum_value<banjo::NoteSavingMode>(banjo::configkeys::general::note_saving_mode);
}

banjo::CameraInvertMode banjo::get_camera_invert_mode() {
    return get_general_config_enum_value<banjo::CameraInvertMode>(banjo::configkeys::general::camera_invert_mode);
}

banjo::CameraInvertMode banjo::get_third_person_camera_mode() {
    return get_general_config_enum_value<banjo::CameraInvertMode>(banjo::configkeys::general::third_person_camera_invert_mode);
}

banjo::CameraInvertMode banjo::get_flying_and_swimming_invert_mode() {
    return get_general_config_enum_value<banjo::CameraInvertMode>(banjo::configkeys::general::flying_and_swimming_invert_mode);
}

banjo::CameraInvertMode banjo::get_first_person_invert_mode() {
    return get_general_config_enum_value<banjo::CameraInvertMode>(banjo::configkeys::general::first_person_invert_mode);
}

banjo::AnalogCamMode banjo::get_analog_cam_mode() {
    return get_general_config_enum_value<banjo::AnalogCamMode>(banjo::configkeys::general::analog_cam_mode);
}

uint32_t banjo::get_analog_cam_sensitivity() {
    return get_general_config_number_value(banjo::configkeys::general::analog_camera_sensitivity);
}

template <typename T = uint32_t>
T get_graphics_config_enum_value(const std::string& option_id) {
    return static_cast<T>(std::get<uint32_t>(recompui::config::get_graphics_config().get_option_value(option_id)));
}

static void add_sound_options(recomp::config::Config &config) {
    config.add_percent_number_option(
        banjo::configkeys::sound::bgm_volume,
        "Background Music Volume",
        "Controls the overall volume of background music.",
        100.0f
    );
}
template <typename T = uint32_t>
T get_sound_config_number_value(const std::string& option_id) {
    return static_cast<T>(std::get<double>(recompui::config::get_sound_config().get_option_value(option_id)));
}

int banjo::get_bgm_volume() {
    return get_sound_config_number_value<int>(banjo::configkeys::sound::bgm_volume);
}

static RT64::UserConfiguration::StereoMode banjo_to_rt64_stereo(banjo::StereoMode mode) {
    switch (mode) {
        case banjo::StereoMode::SideBySide:        return RT64::UserConfiguration::StereoMode::SideBySide;
        case banjo::StereoMode::TopAndBottom:      return RT64::UserConfiguration::StereoMode::TopAndBottom;
        case banjo::StereoMode::RowInterlaced:     return RT64::UserConfiguration::StereoMode::RowInterlaced;
        case banjo::StereoMode::ColumnInterlaced:  return RT64::UserConfiguration::StereoMode::ColumnInterlaced;
        case banjo::StereoMode::Checkerboard:      return RT64::UserConfiguration::StereoMode::Checkerboard;
        case banjo::StereoMode::Anaglyph:          return RT64::UserConfiguration::StereoMode::Anaglyph;
        case banjo::StereoMode::LeiaSR:            return RT64::UserConfiguration::StereoMode::LeiaSR;
        case banjo::StereoMode::Off:
        default:                                   return RT64::UserConfiguration::StereoMode::Off;
    }
}

static void push_stereo_config_to_renderer() {
    // Read the temp values so slider drags push to the renderer immediately,
    // before the user hits Apply. For tabs that don't require confirmation,
    // get_temp_option_value falls back to the committed value, so this is
    // safe for one-shot reads at startup too.
    auto &graphics_config = recompui::config::get_graphics_config();
    auto mode = static_cast<banjo::StereoMode>(std::get<uint32_t>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_mode)));
    uint32_t separation = static_cast<uint32_t>(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_separation)));
    // Convergence is the one slider with sub-unit steps, so it crosses the
    // renderer bridge in tenths rather than whole units. Everything downstream
    // of set_stereo_config works in those tenths.
    uint32_t convergence = static_cast<uint32_t>(std::lround(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_convergence)) * 10.0));
    uint32_t hudDepth = static_cast<uint32_t>(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_hud_depth)));
    bool autoConv = std::get<bool>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_auto_convergence));
    uint32_t autoConvScale = static_cast<uint32_t>(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_auto_convergence_scale)));
    uint32_t ghostContrast = static_cast<uint32_t>(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_ghost_contrast)));
    uint32_t ghostBlackFloor = static_cast<uint32_t>(std::get<double>(
        graphics_config.get_temp_option_value(banjo::configkeys::graphics::stereo_ghost_black_floor)));
    recompui::renderer::set_stereo_config(banjo_to_rt64_stereo(mode), separation, convergence, hudDepth, autoConv, autoConvScale, ghostContrast, ghostBlackFloor);
}

static void add_graphics_options(recomp::config::Config &config) {
    using EnumOptionVector = const std::vector<recomp::config::ConfigOptionEnumOption>;
    static EnumOptionVector cutscene_aspect_ratio_mode_options = {
        {banjo::CutsceneAspectRatioMode::Original, "Original", "Original"},
        {banjo::CutsceneAspectRatioMode::Clamp16x9, "Clamp16x9", "16:9"},
        {banjo::CutsceneAspectRatioMode::Full, "Expand", "Expand"},
    };
    config.add_enum_option(
        banjo::configkeys::graphics::cutscene_aspect_ratio_mode,
        "Cutscene Aspect Ratio",
        "Sets the aspect ratio limit for cutscenes. Cutscenes have been adjusted to work in <recomp-color primary>16:9</recomp-color>, which is the default option. Wider aspect ratios may show details that weren't meant to be on-screen."
        "<br />"
        "<br />"
        "<recomp-color primary>Note: Stereoscopic 3D requires 16:9.</recomp-color>",
        cutscene_aspect_ratio_mode_options,
        banjo::CutsceneAspectRatioMode::Clamp16x9
    );

    static EnumOptionVector stereo_mode_options = {
        {banjo::StereoMode::Off, "Off", "Off"},
        {banjo::StereoMode::SideBySide, "SideBySide", "SbS"},
        {banjo::StereoMode::TopAndBottom, "TopAndBottom", "TaB"},
        {banjo::StereoMode::RowInterlaced, "RowInterlaced", "RowInt"},
        {banjo::StereoMode::ColumnInterlaced, "ColumnInterlaced", "ColInt"},
        {banjo::StereoMode::Checkerboard, "Checkerboard", "Checker"},
        {banjo::StereoMode::Anaglyph, "Anaglyph", "Anaglyph"},
        {banjo::StereoMode::LeiaSR, "LeiaSR", "LeiaSR"},
    };
    config.add_enum_option(
        banjo::configkeys::graphics::stereo_mode,
        "Stereoscopic 3D",
        "Renders the world from two viewpoints and packs them into a single frame for stereo-capable displays. The HUD and menus stay flat at the screen plane.<br /><br /><recomp-color primary>SbS</recomp-color> and <recomp-color primary>TaB</recomp-color> are intended for 3D TVs, SbS-capable VR injectors, and full-SbS AR glasses; each eye gets the full game view, cropped to a 16:9 slice on ultra-wide desktops. <recomp-color primary>RowInt</recomp-color> / <recomp-color primary>ColInt</recomp-color> / <recomp-color primary>Checker</recomp-color> alternate eyes per row, column, or checkerboard cell — for passive 3D displays and similar pixel-level stereo formats; works best at the display's native resolution. <recomp-color primary>Anaglyph</recomp-color> packs both eyes into a single image for red/cyan glasses. <recomp-color primary>LeiaSR</recomp-color> drives a Leia autostereoscopic display via lenticular weaving; requires the SR Platform service and the <recomp-color primary>D3D12</recomp-color> graphics API.<br /><br />Stereo rendering roughly doubles GPU cost.",
        stereo_mode_options,
        banjo::StereoMode::Off
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_separation,
        "Stereo Separation",
        "Controls depth strength: how far apart the two eyes see the horizon, as a fraction of the screen width. Each point is 0.2% of the screen, so the default <recomp-color primary>10</recomp-color> is a comfortable 2% and the maximum <recomp-color primary>50</recomp-color> is 10% — roughly the most an average pair of eyes can still fuse on a 27-inch monitor, and too much on anything larger. Higher values give a stronger effect at the cost of eye strain, and on a Leia display they also spend more of the panel's crosstalk budget.",
        0, 50, 1, 0, false, 10
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_convergence,
        "Stereo Convergence",
        "Sets the distance to the zero-parallax plane (where geometry sits exactly on the screen). Lower values bring the plane toward the camera so more of the scene pops out of the display; higher values push it away so more of the scene sits behind the display. It no longer changes the overall depth strength — that is <recomp-color primary>Stereo Separation</recomp-color> alone. Steps in tenths below <recomp-color primary>1</recomp-color>, where the plane is close enough to the camera to put essentially the whole scene behind the display.",
        0.1, 50, 0.1, 1, false, 20
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_hud_depth,
        "HUD Depth",
        "Where the HUD, dialog, and text sit in stereo space. <recomp-color primary>50</recomp-color> places them flat on the screen plane. Lower values push the HUD behind the screen, higher values pop it out toward the viewer.",
        0, 100, 1, 0, false, 35
    );
    config.add_bool_option(
        banjo::configkeys::graphics::stereo_auto_convergence,
        "Auto Convergence",
        "Automatically reduces convergence in flat or near-camera scenes (file select, FMVs, first-person view, cutscenes, Bottles' bonus). Helps keep these scenes comfortable when the regular Stereo Convergence is tuned for gameplay.",
        true,
        false
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_auto_convergence_scale,
        "Auto Convergence Scale",
        "Fraction of Stereo Convergence applied during auto-triggered scenes. <recomp-color primary>25</recomp-color> = 25% of the slider value (default). <recomp-color primary>100</recomp-color> = no reduction.",
        0, 100, 1, 0, false, 25
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_ghost_contrast,
        "Ghost Reduction: Contrast",
        "Reduces <recomp-color primary>ghosting</recomp-color> (a faint copy of the other eye bleeding through) by squeezing the image's contrast before it reaches the display. How visible a display's crosstalk is depends on the brightness difference between the two eyes, so compressing the range shrinks the leak directly. Costs contrast across the whole image. <recomp-color primary>100</recomp-color> is off; try <recomp-color primary>90</recomp-color> first and go lower only if edges still ghost.",
        50, 100, 1, 0, false, 100
    );
    config.add_number_option(
        banjo::configkeys::graphics::stereo_ghost_black_floor,
        "Ghost Reduction: Black Floor",
        "Raises the black level, leaving white alone. Displays that cancel crosstalk themselves (Leia autostereoscopic panels in particular) subtract part of the opposite eye, which drives dark pixels below zero where they clip — and the clipped part is what you still see as a ghost. Lifting the floor gives that subtraction room to work. Costs black level rather than contrast, and only helps on displays that actually cancel. <recomp-color primary>0</recomp-color> is off; try <recomp-color primary>2</recomp-color>–<recomp-color primary>5</recomp-color>, blacks go grey fast above that.",
        0, 20, 1, 0, false, 0
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_separation,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_convergence,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_hud_depth,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_auto_convergence,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_ghost_contrast,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    config.add_option_hidden_dependency(
        banjo::configkeys::graphics::stereo_ghost_black_floor,
        banjo::configkeys::graphics::stereo_mode,
        banjo::StereoMode::Off
    );
    // Hide the scale slider when either stereo is off or auto-convergence is
    // off. The slider has no effect in those cases so it'd just be noise.
    //
    // ConfigOptionDependency stores ONE value-list per dependent and overwrites
    // it on every add_option_hidden_dependency call, so the naive pattern of
    // two separate calls only ends up honoring the last one. Build a single
    // combined value-list ({stereo_mode == Off, auto_conv == false}) and
    // register it under both source options — each source's value comparison
    // will only match the entry of its own variant type, so the right
    // condition triggers on the right change.
    {
        std::vector<recomp::config::ConfigValueVariant> hide_when_scale = {
            static_cast<uint32_t>(banjo::StereoMode::Off),
            false
        };
        config.add_option_hidden_dependency(
            banjo::configkeys::graphics::stereo_auto_convergence_scale,
            banjo::configkeys::graphics::stereo_mode,
            hide_when_scale
        );
        config.add_option_hidden_dependency(
            banjo::configkeys::graphics::stereo_auto_convergence_scale,
            banjo::configkeys::graphics::stereo_auto_convergence,
            hide_when_scale
        );
    }

    auto stereo_change_callback = [](recomp::config::ConfigValueVariant, recomp::config::ConfigValueVariant, recomp::config::OptionChangeContext) {
        push_stereo_config_to_renderer();
    };
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_mode, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_separation, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_convergence, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_hud_depth, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_auto_convergence, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_auto_convergence_scale, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_ghost_contrast, stereo_change_callback);
    config.add_option_change_callback(banjo::configkeys::graphics::stereo_ghost_black_floor, stereo_change_callback);

    // LeiaSR weaving only works on the D3D12 backend (the weaver SDK is DX12-
    // only). Disable the LeiaSR entry in the stereo-mode dropdown whenever
    // the user switches Graphics API to Vulkan/Metal at runtime. We skip the
    // OptionChangeContext::Load path because calling update_enum_option_disabled
    // during config load (before the UI is set up) crashes the app at startup.
    // Phase 3's runtime will fall back gracefully if LeiaSR is selected on a
    // non-DX12 API at boot.
    config.add_option_change_callback(recompui::config::graphics::options::api_option,
        [&config](recomp::config::ConfigValueVariant cur_value, recomp::config::ConfigValueVariant, recomp::config::OptionChangeContext change_context) {
            if (change_context == recomp::config::OptionChangeContext::Load) {
                return;
            }
            const auto api = static_cast<ultramodern::renderer::GraphicsApi>(std::get<uint32_t>(cur_value));
            const bool dx12_capable = (api == ultramodern::renderer::GraphicsApi::Auto) ||
                                       (api == ultramodern::renderer::GraphicsApi::D3D12);
            config.update_enum_option_disabled(
                banjo::configkeys::graphics::stereo_mode,
                static_cast<uint32_t>(banjo::StereoMode::LeiaSR),
                !dx12_capable
            );
        }
    );

}

static void set_control_defaults() {
    using namespace recompinput;

    // Left shoulder -> C Down | Backwards eggs / zoom out
    set_default_mapping_for_controller(
        GameInput::C_DOWN,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, true),
            InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
        }
    );

    // Right shoulder -> C Up | Forwards eggs / first person
    set_default_mapping_for_controller(
        GameInput::C_UP,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, false),
            InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
        }
    );

    // North button -> C Left | Talon trot / camera left
    set_default_mapping_for_controller(
        GameInput::C_LEFT,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, false),
            InputField::controller_digital(SDL_CONTROLLER_BUTTON_NORTH)
        }
    );

    // East button -> C Right | Wonderwing / camera right
    set_default_mapping_for_controller(
        GameInput::C_RIGHT,
        { 
            InputField::controller_analog(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, true),
            InputField::controller_digital(SDL_CONTROLLER_BUTTON_EAST)
        }
    );

    // R3 -> L | Unused in BK but can be used in mods
    set_default_mapping_for_controller(GameInput::L, { InputField::controller_digital(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK) });
}

static void set_control_descriptions() {
    recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_POS, "Used to move and for steering while flying and swimming. Axis inversion for flying and swimming can be configured in the General tab.");
    recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_NEG, "Used to move and for steering while flying and swimming. Axis inversion for flying and swimming can be configured in the General tab.");
    recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_NEG, "Used to move and for steering while flying and swimming. Axis inversion for flying and swimming can be configured in the General tab.");
    recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_POS, "Used to move and for steering while flying and swimming. Axis inversion for flying and swimming can be configured in the General tab.");
    recompinput::set_game_input_description(recompinput::GameInput::A, "Used to jump and select options in menus. Also used for flying upwards.");
    recompinput::set_game_input_description(recompinput::GameInput::B, "Used for attacks, which change depending on whether you are stationary, moving, in the air, or crouching.");
    recompinput::set_game_input_description(recompinput::GameInput::Z, "Used to crouch, which enables A, B and the C-Buttons to perform different actions.");
    recompinput::set_game_input_description(recompinput::GameInput::L, "Used to skip dialogue boxes during game replays (press L + R + B simultaneously). Mods may also use it separately for additional features.");
    recompinput::set_game_input_description(recompinput::GameInput::R, "Used to center the camera behind Banjo on the ground, and to perform tighter turns while flying or swimming.");
    recompinput::set_game_input_description(recompinput::GameInput::START, "Used for pausing and for skipping certain cutscenes.");
    recompinput::set_game_input_description(recompinput::GameInput::C_UP, "Used to enter first-person mode, and to shoot eggs while holding Z.");
    recompinput::set_game_input_description(recompinput::GameInput::C_DOWN, "Used to toggle between the different camera zoom levels, and to shoot eggs backwards while holding Z.");
    recompinput::set_game_input_description(recompinput::GameInput::C_LEFT, "Used to rotate the camera sideways. Axis inversion can be configured in the General tab. Also used to enter Talon Trot while holding Z.");
    recompinput::set_game_input_description(recompinput::GameInput::C_RIGHT, "Used to rotate the camera sideways. Axis inversion can be configured in the General tab). Also used to enter Wonderwing while holding Z.");
    recompinput::set_game_input_description(recompinput::GameInput::DPAD_UP, "Unused. Mods may use it for additional features.");
    recompinput::set_game_input_description(recompinput::GameInput::DPAD_DOWN, "Unused. Mods may use it for additional features.");
    recompinput::set_game_input_description(recompinput::GameInput::DPAD_LEFT, "Unused. Mods may use it for additional features.");
    recompinput::set_game_input_description(recompinput::GameInput::DPAD_RIGHT, "Unused. Mods may use it for additional features.");
}

banjo::CutsceneAspectRatioMode banjo::get_cutscene_aspect_ratio_mode() {
    return get_graphics_config_enum_value<banjo::CutsceneAspectRatioMode>(banjo::configkeys::graphics::cutscene_aspect_ratio_mode);
}

template <typename T = uint32_t>
T get_graphics_config_number_value(const std::string& option_id) {
    return static_cast<T>(std::get<double>(recompui::config::get_graphics_config().get_option_value(option_id)));
}

banjo::StereoMode banjo::get_stereo_mode() {
    return get_graphics_config_enum_value<banjo::StereoMode>(banjo::configkeys::graphics::stereo_mode);
}

uint32_t banjo::get_stereo_separation() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_separation);
}

uint32_t banjo::get_stereo_convergence() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_convergence);
}

uint32_t banjo::get_stereo_hud_depth() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_hud_depth);
}

bool banjo::get_stereo_auto_convergence() {
    return std::get<bool>(recompui::config::get_graphics_config().get_option_value(banjo::configkeys::graphics::stereo_auto_convergence));
}

uint32_t banjo::get_stereo_ghost_contrast() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_ghost_contrast);
}

uint32_t banjo::get_stereo_ghost_black_floor() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_ghost_black_floor);
}

uint32_t banjo::get_stereo_auto_convergence_scale() {
    return get_graphics_config_number_value(banjo::configkeys::graphics::stereo_auto_convergence_scale);
}

void banjo::init_config() {
    std::filesystem::path recomp_dir = recompui::file::get_app_folder_path();

    if (!recomp_dir.empty()) {
        std::filesystem::create_directories(recomp_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    general_options.has_gyro_sensitivity = false;
    general_options.has_mouse_sensitivity = false;

    auto &general_config = recompui::config::create_general_tab(general_options);
    add_general_options(general_config);

    auto &graphics_config = recompui::config::create_graphics_tab();
    add_graphics_options(graphics_config);

    set_control_defaults();
    set_control_descriptions();
    recompui::config::create_controls_tab();

    auto &sound_config = recompui::config::create_sound_tab();
    add_sound_options(sound_config);

    recompui::config::create_mods_tab();

    recompui::config::finalize();

    // Seed the renderer with the stereo config that was just loaded so the first frame
    // already has the user's saved Stereo Mode / Separation / Convergence applied.
    push_stereo_config_to_renderer();
}
