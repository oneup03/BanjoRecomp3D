#ifndef __BANJO_CONFIG_H__
#define __BANJO_CONFIG_H__

#include <filesystem>
#include <string>
#include <string_view>

#include "json/json.hpp"

namespace banjo {
    inline const std::u8string program_id = u8"BanjoRecompiled";
    inline const std::string program_name = "Banjo: Recompiled";

    namespace configkeys {
        namespace general {
            inline const std::string note_saving_mode = "note_saving_mode";
            inline const std::string camera_invert_mode = "camera_invert_mode";
            inline const std::string analog_cam_mode = "analog_cam_mode";
            inline const std::string third_person_camera_invert_mode = "third_person_camera_invert_mode";
            inline const std::string flying_and_swimming_invert_mode = "flying_and_swimming_invert_mode";
            inline const std::string first_person_invert_mode = "first_person_invert_mode";
            inline const std::string analog_camera_sensitivity = "analog_camera_sensitivity";
        }

        namespace sound {
            inline const std::string bgm_volume = "bgm_volume";
        }

        namespace graphics {
            inline const std::string cutscene_aspect_ratio_mode = "cutscene_aspect_ratio_mode";
            inline const std::string stereo_mode = "stereo_mode";
            inline const std::string stereo_separation = "stereo_separation";
            inline const std::string stereo_convergence = "stereo_convergence";
            inline const std::string stereo_hud_depth = "stereo_hud_depth";
            inline const std::string stereo_auto_convergence = "stereo_auto_convergence";
            inline const std::string stereo_auto_convergence_scale = "stereo_auto_convergence_scale";
        }
    }

    // TODO: Move loading configs to the runtime once we have a way to allow per-project customization.
    void init_config();

    enum class CameraInvertMode {
        InvertNone,
        InvertX,
        InvertY,
        InvertBoth
    };

    CameraInvertMode get_camera_invert_mode();

    CameraInvertMode get_third_person_camera_mode();

    CameraInvertMode get_flying_and_swimming_invert_mode();

    CameraInvertMode get_first_person_invert_mode();

    enum class AnalogCamMode {
        On,
        Off,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(banjo::AnalogCamMode, {
        {banjo::AnalogCamMode::On, "On"},
        {banjo::AnalogCamMode::Off, "Off"}
    });

    AnalogCamMode get_analog_cam_mode();

    uint32_t get_analog_cam_sensitivity();

    enum class NoteSavingMode {
        On,
        Off,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(banjo::NoteSavingMode, {
        {banjo::NoteSavingMode::On, "On"},
        {banjo::NoteSavingMode::Off, "Off"}
    });

    NoteSavingMode get_note_saving_mode();

    enum class CutsceneAspectRatioMode {
        Original,
        Clamp16x9,
        Full,
        OptionCount
    };

    CutsceneAspectRatioMode get_cutscene_aspect_ratio_mode();

    enum class StereoMode {
        Off,
        SideBySide,
        TopAndBottom,
        RowInterlaced,
        ColumnInterlaced,
        Checkerboard,
        Anaglyph,
        LeiaSR,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(banjo::StereoMode, {
        {banjo::StereoMode::Off, "Off"},
        {banjo::StereoMode::SideBySide, "SideBySide"},
        {banjo::StereoMode::TopAndBottom, "TopAndBottom"},
        {banjo::StereoMode::RowInterlaced, "RowInterlaced"},
        {banjo::StereoMode::ColumnInterlaced, "ColumnInterlaced"},
        {banjo::StereoMode::Checkerboard, "Checkerboard"},
        {banjo::StereoMode::Anaglyph, "Anaglyph"},
        {banjo::StereoMode::LeiaSR, "LeiaSR"}
    });

    StereoMode get_stereo_mode();
    // Returns the user-facing slider value (0..100). Conversion to world-space
    // separation/convergence happens at the render layer.
    uint32_t get_stereo_separation();
    uint32_t get_stereo_convergence();
    // HUD/textbox depth, 0..100. 50 = screen plane (mono). Below 50 pushes HUD
    // behind the screen; above 50 makes it pop out.
    uint32_t get_stereo_hud_depth();
    // Auto-convergence: when on, the renderer scales the configured convergence
    // down in flat / near-camera scenes (file select, FMVs, cutscenes, first
    // person, Bottles' bonus) so they sit comfortably without the user having
    // to drag the slider.
    bool get_stereo_auto_convergence();
    // 0..100, applied as a fraction (0.0..1.0) of the configured convergence
    // when auto-convergence triggers. 25 = 25% of the slider value.
    uint32_t get_stereo_auto_convergence_scale();

    void open_quit_game_prompt();
};

#endif
