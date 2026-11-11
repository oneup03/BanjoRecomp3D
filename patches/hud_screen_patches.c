// The HUD on the 3DS's touch screen (the Game page's HUD row, there by
// default). rt64-3ds draws the display list's marked spans there
// (rt64_3ds_gbi.h), each moved by its own shift from the game's place, and
// ignores the markers while the row says the top screen. Marked: every
// counter itemPrint_draw draws but the timers (items 0-5), and the counters'
// numbers, which wait in the print buffer (printbuffer_draw,
// text_transform_tagging.c). The pause menu, dialogue, transitions and the
// timers stay on the top screen.
//
// The counters slide in when something changes and out 3 seconds later. With
// the HUD on the touch screen the pause's set stays shown through play.
#ifdef BK_3DS
#include "patches.h"
#include "patch_helpers.h"
#include "transform_ids.h"
#include "functions.h"
#include "rt64_3ds_gbi.h"

DECLARE_FUNC(s32, recomp_get_hud_on_bottom);

// code_73640.c's table of the HUD's counters, by item.
typedef struct {
    struct8s *(*init)(s32);
    void (*update)(s32, struct8s *);
    void (*draw)(enum item_e, struct8s *, Gfx **, Mtx **, Vtx **);
    void (*release)(s32, struct8s *);
    s32 group;
    struct8s *state;
} ItemPrint;

extern ItemPrint D_803692F8[0x2C];
extern s32 D_803810B0;      // the HUD drawn at all
extern void func_802FACA4(enum item_e item_id);
extern s32 func_802FB0D4(struct8s *this);
extern bool func_802E4A08(void);
extern bool func_802FC3C4(void);
extern bool func_802FD2D4(void);
extern s32 getGameMode(void);
extern s32 level_get(void);
enum map_e map_get(void);
extern f32 func_802FBE60(struct8s *this);

typedef struct {
    void (*update)(void);
    void (*funcs[5])(s32, struct8s *);
} HudGroup;
extern HudGroup D_80369720[7];

// pauseMenu.c's state, the fields used here (hud_transform_tagging.c has the
// whole struct), and the values used here of its state and selection enums.
extern struct {
    u8 state;
    u8 menu;
    u8 selection;
    u8 pad3[0x6D];
    u32 unk70_31 : 1;   // the jinjos, to show once they've stopped
    u32 unk70_30 : 1;   // the lives, the same
    u32 pad70_28 : 30;
} D_80383010;
extern void itemPrint_reset(void);
#define PAUSE_STATE_5_SELECTION_CONFIRMATION 5
#define PAUSE_SELECTION_2_VIEW_TOTALS 2
#define PAUSE_SELECTION_3_SAVE_AND_EXIT 3
extern s32 controller_getStartButton(s32 controller_index);

#define FIRST_ON_BOTTOM 6   // items 0-5, the timers, stay on the top screen

// A counter's state (struct8s unk0) and its group's timer (unk10, seconds
// left shown).
enum { HUD_HIDDEN, HUD_SLIDING_IN, HUD_SHOWN, HUD_SLIDING_OUT };

// Where a counter goes on the touch screen: its shift from the game's place
// (N64 pixels; the 292x216 frame sits at 14, 12 on the 320x240 screen). The
// honeycombs centred at the top, the rest down the left, the right half for
// the touch buttons (main_3ds.cpp, GameDesc::touch_area_hud). Most groups
// move whole, the game stacking their counters as ever. The minigames'
// counters (Mr. Vile's, the Grumblies', Twinkly's) keep their places.
static void hud_screen_shift(s32 item, s32 *dx, s32 *dy) {
    *dx = 0;
    *dy = 0;
    switch (D_803692F8[item].group) {
        case 1:     // notes, eggs, feathers: down the left edge, under the lives
            *dx = -203;
            *dy = 40;
            return;
        case 4:
            // The jiggy and Mumbo token totals: a second column, on the
            // notes' and eggs' rows (y 77 and 110), the icons centred at x 112
            // (clear of a three-digit note count) and the numbers from x 138.
            // The game has the token 19 px right of the jiggy and lower than
            // the left column's pitch, so each moves on its own.
            if (item == ITEM_25_MUMBO_TOKEN_TOTAL) {
                *dx = -125;
                *dy = -95;
            }
            else {
                *dx = -106;
                *dy = -91;
            }
            return;
        case 2:     // this level's oranges, presents, acorns...: under the totals
            *dx = -122;
            *dy = -46;
            return;
        case 6:     // lives (and Mr. Vile's score under them)
            *dx = -16;
            return;
        case 3: {
            // The jinjos and this level's jiggies stack 40 px apart, the
            // newest lowest: a jiggy coming after the jinjos (drawn or not,
            // hud_screen_doubled) would lift them into the notes. Each keeps
            // a place of its own instead.
            s32 unstack = (s32)func_802FBE60(D_803692F8[item].state);
            if (item == ITEM_12_JINJOS) {
                *dx = -37;
                *dy = 20 + unstack;
            }
            else if (item == ITEM_E_JIGGY) {
                *dx = -50;
                *dy = -20 + unstack;
            }
            return;
        }
    }
    switch (item) {
        case ITEM_14_HEALTH:
            // The honeycombs (13 px apart) from x 92, right of the lives:
            // the full 8 end at x 191, short of the air.
            *dx = -12;
            break;
        case ITEM_17_AIR:
            // Across, right of the honeycombs (fxairscore_draw lays it out
            // so), x 200 to 297, and 6 px down: centred on the honeycombs'
            // row (y 25 to 54; the game's air row sat at 20 to 47).
            *dx = 156;
            *dy = 6;
            break;
        case ITEM_13_EMPTY_HONEYCOMB:
            // Into the bottom right corner, under the touch buttons' usual rows.
            *dx = 20;
            *dy = 20;
            break;
    }
}

// One counter each for the Mumbo tokens and the jiggies on the touch screen:
// the totals, which stay up there, rather than the counts that pop in beside
// them when one is collected (the tokens' is the same number; the jiggies'
// is this level's, which the pause's Totals page has).
static bool hud_screen_doubled(s32 item) {
    s32 total;
    switch (item) {
        case ITEM_1C_MUMBO_TOKEN: total = ITEM_25_MUMBO_TOKEN_TOTAL; break;
        case ITEM_E_JIGGY: total = ITEM_26_JIGGY_TOTAL; break;
        default: return FALSE;
    }
    return recomp_get_hud_on_bottom() && func_802FB0D4(D_803692F8[total].state) != HUD_HIDDEN;
}

// @recomp The counters but the timers in touch-screen spans, each moved to
// its place there.
RECOMP_PATCH void itemPrint_draw(Gfx **gdl, Mtx **mptr, Vtx **vptr) {
    s32 i;
    bool bottom = FALSE;
    s32 cur_dx = 0;
    s32 cur_dy = 0;

    if (D_803810B0 && level_get() != LEVEL_D_CUTSCENE) {
        for (i = 0; i < 0x2C; i++) {
            if (!func_802E4A08() || i < 6) {
                if (func_802FB0D4(D_803692F8[i].state) && !hud_screen_doubled(i)) {
                    if (i >= FIRST_ON_BOTTOM) {
                        s32 dx, dy;
                        hud_screen_shift(i, &dx, &dy);
                        if (!bottom || dx != cur_dx || dy != cur_dy) {
                            gEX3DSBottomScreen((*gdl)++, dx, dy);
                            bottom = TRUE;
                            cur_dx = dx;
                            cur_dy = dy;
                        }
                    }
                    D_803692F8[i].draw(i, D_803692F8[i].state, gdl, mptr, vptr);
                }
            }
        }
        if (bottom) {
            gEX3DSTopScreen((*gdl)++);
        }
    }
}

// A print buffer entry's id (the transform id its counter gave it): one of
// the counters' numbers on the touch screen, and its counter's shift there.
// printbuffer_draw marks those.
bool hud_screen_print_shift(u32 id, s32 *dx, s32 *dy) {
    s32 item;
    if (id >= HUD_LIFESCORE_TRANSFORM_PRINT_ID_START && id < HUD_LIFESCORE_TRANSFORM_ID_START) {
        item = ITEM_16_LIFE;
    }
    else if (id >= HUD_SCORE2_TRANSFORM_PRINT_ID_START && id < HUD_SCORE2_TRANSFORM_ID_START) {
        item = (id - HUD_SCORE2_TRANSFORM_PRINT_ID_START) / HUD_SCORE2_TRANSFORM_PRINT_ID_COUNT;
    }
    else if (id >= HUD_SCORE3_TRANSFORM_PRINT_ID_START && id < HUD_SCORE3_DEPTH_RECT_TRANSFORM_ID_START) {
        item = (id - HUD_SCORE3_TRANSFORM_PRINT_ID_START) / HUD_SCORE3_TRANSFORM_PRINT_ID_COUNT;
    }
    else {
        return FALSE;
    }
    if (item < FIRST_ON_BOTTOM || item >= 0x2C) {
        return FALSE;
    }
    hud_screen_shift(item, dx, dy);
    return TRUE;
}

// The pause's set (gcpausemenu_80311A84), with the jinjos and the lives
// under its conditions below.
static const u8 kKeptShown[] = {
    ITEM_14_HEALTH, ITEM_26_JIGGY_TOTAL, ITEM_25_MUMBO_TOKEN_TOTAL, ITEM_10_GOLD_FEATHER,
    ITEM_F_RED_FEATHER, ITEM_D_EGGS, ITEM_C_NOTE,
};

// In play: not the cutscenes, nor the file select (Banjo's house, a Spiral
// Mountain map), which the game leaves out alike (code_9A740.c).
static bool hud_keep_shown(void) {
    return recomp_get_hud_on_bottom() && D_803810B0 && getGameMode() == GAME_MODE_3_NORMAL &&
        level_get() != LEVEL_D_CUTSCENE && map_get() != MAP_91_FILE_SELECT;
}

// Whether a counter of the pause's set counts anything here, with the HUD on
// the touch screen: the jinjos are in the nine worlds only, and Spiral
// Mountain has no notes (the Lair shows their total, for the note doors).
// On a map change the counters start hidden, so one that doesn't count is
// only kept from showing.
static bool hud_counts_here(enum item_e item) {
    s32 level = level_get();

    if (!recomp_get_hud_on_bottom()) {
        return TRUE;
    }
    switch (item) {
        case ITEM_12_JINJOS:
            return level >= LEVEL_1_MUMBOS_MOUNTAIN && level <= LEVEL_A_MAD_MONSTER_MANSION &&
                level != LEVEL_6_LAIR;
        case ITEM_C_NOTE:
            return level != LEVEL_B_SPIRAL_MOUNTAIN;
        default:
            return TRUE;
    }
}

// Slides a counter in unless it's in or coming. func_802FACA4 is the game's
// own (a game mode's, if it replaces it); on a shown counter it would also
// jump the rolling count.
static void hud_show(enum item_e item) {
    s32 state = func_802FB0D4(D_803692F8[item].state);
    if (hud_counts_here(item) && (state == HUD_HIDDEN || state == HUD_SLIDING_OUT)) {
        func_802FACA4(item);
    }
}

// @recomp The groups' updates (itemPrint_update's first step), with the
// pause's set kept shown while the HUD is on the touch screen. The groups
// that hold the set (1: notes, eggs, feathers; 3: jinjos; 4: the totals; 6:
// lives) hide together when their newest counter's 3 seconds run out, so
// those timers stay full: a counter that pops in beside the set (this
// level's jiggies, Mr. Vile's score) stays up with it until the map
// changes. Group 0's counters each have their own timer: only the health's
// is held, so the air, empty honeycombs and minigame scores still go.
RECOMP_PATCH void func_802FB1CC(void) {
    s32 i;

    if (hud_keep_shown()) {
        for (i = 0; i < ARRLEN(kKeptShown); i++) {
            hud_show(kKeptShown[i]);
        }
        if (!func_802FC3C4()) {
            hud_show(ITEM_12_JINJOS);
        }
        if (!func_802FD2D4()) {
            hud_show(ITEM_16_LIFE);
        }
        for (i = 0; i < 0x2C; i++) {
            struct8s *s = D_803692F8[i].state;
            s32 group = D_803692F8[i].group;
            if (s->unk0 == HUD_SHOWN &&
                (group == 1 || group == 3 || group == 4 || group == 6 || i == ITEM_14_HEALTH)) {
                s->unk10 = 3.0f;
            }
        }
    }

    for (i = 0; i < 7; i++) {
        D_80369720[i].update();
    }
}

// @recomp The pause's menu shows its set: on the touch screen only what counts
// here.
RECOMP_PATCH void gcpausemenu_80311A84(void) {
    s32 i;

    itemPrint_reset();
    for (i = 0; i < ARRLEN(kKeptShown); i++) {
        if (hud_counts_here(kKeptShown[i])) {
            func_802FACA4(kKeptShown[i]);
        }
    }

    if (!hud_counts_here(ITEM_12_JINJOS)) {
        D_80383010.unk70_31 = 0;
    }
    else if (func_802FC3C4()) {
        D_80383010.unk70_31 = 1;
        func_802FAD64(ITEM_12_JINJOS);
    }
    else {
        func_802FACA4(ITEM_12_JINJOS);
    }

    if (func_802FD2D4()) {
        D_80383010.unk70_30 = 1;
        func_802FAD64(ITEM_16_LIFE);
    }
    else {
        func_802FACA4(ITEM_16_LIFE);
    }
}

// Whether leaving the pause's menu keeps its set shown: back to play or the
// Lair, yes. Into the totals, no: their text would colour the counters'
// numbers, and the game shows the set again back on the menu. A opens the
// totals, START closes the pause from any row. Out through save and exit
// (confirmed, the state not yet moved on from the confirmation), no.
static bool hud_pause_keeps_set(void) {
    if (!recomp_get_hud_on_bottom()) {
        return FALSE;
    }
    if (D_80383010.selection == PAUSE_SELECTION_2_VIEW_TOTALS) {
        return controller_getStartButton(0) == 1;
    }
    return !(D_80383010.state == PAUSE_STATE_5_SELECTION_CONFIRMATION &&
             D_80383010.selection == PAUSE_SELECTION_3_SAVE_AND_EXIT);
}

// @recomp Leaving the pause's menu hides its set, unless it's kept shown.
RECOMP_PATCH void gcpausemenu_80311B44(void) {
    s32 i;

    if (hud_pause_keeps_set()) {
        return;
    }
    for (i = 0; i < ARRLEN(kKeptShown); i++) {
        func_802FAD64(kKeptShown[i]);
    }
    func_802FAD64(ITEM_12_JINJOS);
    func_802FAD64(ITEM_16_LIFE);
}

#endif
