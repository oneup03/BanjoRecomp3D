// The game options the 3DS settings menu sets (main_3ds.cpp) and the host
// functions read (recomp_api_3ds.cpp).
#ifndef BK_SETTINGS_H
#define BK_SETTINGS_H

// Note & Jinjo Saving, in the menu's order.
enum {
    BK_SAVING_OFF,
    BK_SAVING_NOTES,
    BK_SAVING_JINJOS,
    BK_SAVING_BOTH,
};

extern int g_bk_note_saving;
extern int g_bk_analog_cam;
extern int g_bk_analog_cam_sensitivity;
// Axis inversion: 0 none, 1 X, 2 Y, 3 both.
extern int g_bk_camera_invert;
extern int g_bk_first_person_invert;
extern int g_bk_fly_swim_invert;
extern int g_bk_bgm_volume;
// Particles: 0 full, 1 half, 2 quarter (the divisor is 1 << value).
extern int g_bk_particles;

#endif
