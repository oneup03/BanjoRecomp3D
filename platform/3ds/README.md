# Nintendo 3DS build

Banjo: Recompiled for the Nintendo 3DS family, with stereoscopic 3D. A New
3DS or New 2DS XL is recommended; an Old 3DS or 2DS runs it, but slowly
(see Performance). The renderer and platform layer are
[rt64-3ds](https://github.com/oneup03/rt64-3ds) (`lib/rt64-3ds`).

## Installing

- **CIA** (`BanjoRecompiled3D-3ds-<commit>.cia`): install it with FBI or
  another CIA installer; FBI can also scan the QR code on the release page
  (Remote Install > Scan QR Code). It appears on the HOME Menu as "Banjo:
  Recompiled". On an Old 3DS or 2DS, use the CIA (see Performance).
- **.3dsx** (`BanjoRecompiled3D-3ds-<commit>.zip`): extract it to the root of
  the SD card and start `3ds/BanjoRecomp/BanjoRecomp.3dsx` from the Homebrew
  Launcher.

Both need your own ROM: the US 1.0 version of Banjo-Kazooie in .z64 format
(the file the desktop version asks for), at `sdmc:/3ds/BanjoRecomp/BK.z64`.
The game checks the ROM's hash at start and deletes a file that does not
match, so copy it rather than move it.

For sound, the console's DSP firmware must be on the SD card at
`sdmc:/3ds/dspfirm.cdc`, as for any homebrew with audio. Dump it once with
Luma3DS: open the Rosalina menu (L + D-pad down + SELECT), then
Miscellaneous options > Dump DSP firmware. Without it the game runs silent.

On the SD card, once installed:

```
sdmc:/3ds/
  dspfirm.cdc              the DSP firmware, for sound
  BanjoRecomp/
    BanjoRecomp.3dsx       the program (not needed with the CIA installed)
    BK.z64                 your ROM
    mods/                  the game modes' ROMs, for those you play (see Game modes)
      bk_jiggies_of_time.z64       BK The Jiggies of Time
      bk_jiggies_of_time.z64.ok
      bk_nostalgia_64.z64          BK Nostalgia 64
      bk_nostalgia_64.z64.ok
      Loggo_BanjoDreamie.z64       Banjo Dreamie
      Loggo_BanjoDreamie.z64.ok
```

### Game modes

Three ROM hacks can be played as game modes. Their code is built in; each
needs its own ROM, which you make once on a PC from your `BK.z64` and the
mod's file:

| Game mode | Version | Thunderstore package |
|---|---|---|
| BK The Jiggies of Time | 1.0.1 | [KurkoMods/BK_The_Jiggies_of_Time](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_The_Jiggies_of_Time/) |
| BK Nostalgia 64 | 1.0.1 | [KurkoMods/BK_Nostalgia_64](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_Nostalgia_64/) |
| Banjo Dreamie | 1.3.2 | [Loggo/Banjo_Dreamie](https://thunderstore.io/c/banjo-recompiled/p/Loggo/Banjo_Dreamie/) |

1. Extract the .3dsx zip into a folder on your PC, or straight onto the SD
   card's root. Do this with the CIA too: the zip holds the script.
2. Put your ROM there as `3ds/BanjoRecomp/BK.z64`.
3. Download the game modes you want from Thunderstore (the table above), take
   the `.nrm` file out of each package (a zip), and put it next to
   `make_game_mod_rom.py` at the folder's root.
4. Run `make_game_mod_rom.py`: double-click it on Windows, or run
   `python3 make_game_mod_rom.py` in that folder. It needs Python 3
   (python.org) and nothing else. It writes each mode's ROM (64 MB) and a
   small `.ok` file into `3ds/BanjoRecomp/mods/`.
5. Copy the `3ds` folder onto the SD card's root, unless you extracted onto
   the card. The `.nrm` files and the script aren't needed on the card.

Use the versions above: a ROM made from another version is refused. The
console doesn't make these ROMs itself: the patch alone is more than an Old
3DS has memory for.

With a mode's ROM in place, a menu at start offers the game and each game
mode. A mode that can't be played is listed in red with the reason: its ROM
missing, made from another version, or incomplete. Each mode keeps its own
saves.

## Controls

| 3DS | N64 |
|---|---|
| A / B | A / B |
| L | Z |
| R | R |
| START | START |
| X, D-pad up | C-Up |
| Y, D-pad down | C-Down |
| ZL, D-pad left | C-Left |
| ZR, D-pad right | C-Right |
| Circle Pad | analog stick |
| C-Stick | C buttons, but not C-Up (first person) unless "C-Stick up" is on; or the analog camera (Controls settings) |

SELECT opens the settings on the bottom screen. Some built-in mods add
buttons on the touch screen while they're on: see Mods.

Holding SELECT for a second saves a screenshot to
`sdmc:/3ds/BanjoRecomp/screenshots/`: both eyes when 3D is on, and the touch
screen, as BMPs.

## Stereo 3D

The console's 3D slider turns the 3D on and sets how strong it is; with
the slider down the game draws one image. The settings' 3D page tunes it:

| Setting | What it does |
|---|---|
| 3D depth | The separation between the eyes at the top of the slider (default 50). Raise it for more depth, lower it if the image is hard to focus. |
| Convergence | How far into the scene the screen's plane sits (default 10.0): things nearer than it come out of the screen, things beyond it go in. |
| HUD depth | Where the HUD, menus and text sit (50 is on the screen's plane; higher brings them out of the screen, lower pushes them in). |
| Auto convergence | On by default: when something comes close to the camera (a wall, Banjo in a close-up, a cutscene's speaker), the screen's plane moves nearer so it doesn't pop out too far. Convergence stays the farthest it goes. |
| Comfort target | How far out of the screen Auto convergence lets near things come (0 by default; higher for more pop-out, lower to keep everything behind the screen). |
| Ghost reduction: contrast, Ghost black floor | Ease ghosting (a faint double image of high-contrast edges on some screens) by softening contrast and lifting blacks. Off by default (100% and 0%). |

## Performance

**New 3DS / New 2DS XL:** holds 60 fps most of the time. The game itself
runs at 30 in play and 20 in many cutscenes, and by default interpolated
frames are drawn between its own (the Game page's Frame rate: 60 interp.):
one at 30, two at 20. The heaviest scenes drop back toward the game's rate,
and frames that render into a texture (the jiggy transition, the picture
puzzles) go without. Set Frame rate to Game for the original frame rate.

**Old 3DS / 2DS:** runs at 15-25 fps. Install the CIA rather than running
the .3dsx: an Old 3DS gives a game only 64 MB, too little to hold the 16 MB
ROM beside the game, so the game reads the ROM from the SD card as it needs
it, and the CIA leaves it more room than the Homebrew Launcher does. Loads
take a little longer than on a New 3DS; play itself isn't slowed by the
reads. It starts at the game's own frame rate, as it can't afford the
frames in between, and with particles at Quarter.

To make it run on the 3DS, this build keeps a few things the desktop build
turns off. The game's own culling stays on and far objects use their
low-detail models, as on the N64. The Game page's **Particles** row (Full,
Half, Quarter) thins the sparkles, dust and smoke: each particle costs a
draw (a jiggy's sparkles were 225 of one frame's 360 draws), which an Old
3DS can't afford by the hundred. Half divides every burst by two and halves
the steady emitters' rate; nothing is switched off, since effects end when
their particles run out.

## Mods

Mods from the Banjo: Recompiled community are built into the executable (the
3DS cannot load mod code at runtime). They start switched off: turn them on
from the Mods page of the settings, which also holds their options. Both
take effect at once, and are kept in `settings.ini`.

"D-pad" and "L" below are the N64's, which these mods put on the touch
screen (or on the D-pad and ZL: see below); other buttons are the 3DS's, as
in Controls.

| Mod | Author | What it does | How to use it |
|---|---|---|---|
| Ability Anywhere | ProxySaw | Flight and the Shock Jump without their pads | Standing or walking, hold L and press A: tap A to take off, hold A to Shock Jump. With "Require Ability Unlocks" on, only the moves you've learned. |
| Banjo Bee | TSR Stormed | Fly as Banjo Bee | D-pad down turns bee flight on or off (a message says which); then jump to fly. |
| Cheats | ProxySaw | Moon jump, extra speed, infinite health, lives, ammo and air | Each is an option. Moon Jump: hold L to float up. Speed Modifier: hold R to run faster. The rest need no buttons. |
| Exit To Lair | ProxySaw | The game's unused "Exit to Lair" pause option | Pause (START) inside a world and choose it. |
| Quiet Talon Trot | PunishedBigs | Mutes Kazooie's running noise | Nothing to press; options choose which noises. |
| Sir Slush Configuration | Thermopolis | Tunes the Freezeezy Peak snowmen's snowballs | Nothing to press; options set their attacks. |
| Skip It | ProxySaw | Skips cutscenes, text and Bottles' lessons, speeds up Mumbo, boots straight to the file select | Each is an option. "Skip Cutscenes: Press Start" skips a cutscene with START; the rest work by themselves. |
| Toggle Trot | ProxySaw | Talon Trot stays on without holding Z | Start Talon Trot as usual (hold the 3DS's L, which is Z, and press C-Left), let go and keep trotting; press it again to stop. |
| Transform Anywhere | ProxySaw | Mumbo's transformations anywhere | D-pad up: washing machine. Down: crocodile, then walrus. Left: bee. Right: termite, then pumpkin. Pressing a direction again moves to its next form, then back to Banjo. "Token Payment" (on at first) allows only the forms you've paid Mumbo's tokens for; turn it off for all of them. "Allowed Transformations: Current World" gives only D-pad up, for the world you're in. "Button Mode: L + D-pad" needs L held with the D-pad (on the buttons only: the touch screen takes one touch at a time). |

All are CC0 and built from source (`platform/3ds/mods/`, `mods.list`).

Some mods read N64 buttons Banjo's controls otherwise leave out: the D-pad
(Banjo Bee uses its down, Transform Anywhere all four directions) and L
(Ability Anywhere, Cheats, Transform Anywhere's "L + D-pad" mode). While one
of them is on, those buttons appear on the touch screen, under the status
lines: touch and hold one to press it (one at a time). They make way while
the settings are open.

"Mod buttons" on the Controls page can put them on the buttons instead: then
the D-pad's directions those mods use become the N64 D-pad's, and ZL becomes
N64 L. The changes add up when several mods are on:

| Mod on | D-pad | ZL | C-Left is on |
|---|---|---|---|
| Banjo Bee | down is the N64 D-pad's down (C-Down stays on Y) | C-Left | ZL, D-pad left, C-Stick |
| Ability Anywhere, Cheats | C buttons | N64 L | D-pad left, C-Stick |
| Transform Anywhere | the N64 D-pad | N64 L | C-Stick |

C-Up stays on X, C-Down on Y and C-Right on ZR throughout. An Old 3DS has no
ZL, ZR or C-Stick, so keep "Mod buttons" on the touch screen there.

In a game mode, Exit to Lair stays off and isn't listed: it returns to the
original game's lair entrances, which the game modes rework. It stays on for
the game itself if you turned it on.

## AI use

The 3DS port, including this build's code and documentation and the
rt64-3ds renderer it uses, was written with the help of AI tools (Claude),
under the developer's direction and with their testing on real consoles.

## Building

```
git submodule update --init --recursive     # rt64-3ds has a submodule of its own
git submodule update --init --checkout platform/3ds/mods   # the built-in mods' sources
make -C platform/3ds nmr-patch host-tools   # once
make -C platform/3ds codegen                # after changing the ROM inputs, the patches or mods.list
make -C platform/3ds                        # the .3dsx in build-3ds/
make -C platform/3ds deploy-azahar          # copy it to the Azahar emulator's SD card
```

This needs devkitPro's `3ds-dev` group (`DEVKITPRO` defaults to
`/opt/devkitpro`), `makerom` and `bannertool` on the PATH for the CIA, clang
with the MIPS target and `ld.lld` for the patches (`PATCH_CC=`, `LLD=`,
`LLD_LIBS=` to name other ones) and the decompressed ROM in the repository
root, as for the desktop build. `RT64_3DS=<path>` builds against an rt64-3ds
checkout elsewhere.

On Fedora: `sudo dnf install clang lld cmake ninja-build pacman`, then
devkitPro's pacman setup for Fedora (https://devkitpro.org/wiki/devkitPro_pacman)
and `sudo pacman -S 3ds-dev`.
