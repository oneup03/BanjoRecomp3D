# Nintendo 3DS build

Banjo: Recompiled for the Nintendo 3DS family, with stereoscopic 3D. A New
3DS or New 2DS XL runs it at 60 fps most of the time, an Old 3DS or 2DS at
20-30 fps (see Performance). The renderer and platform layer are
[rt64-3ds](https://github.com/oneup03/rt64-3ds) (`lib/rt64-3ds`).

## Installing

Download the latest build from the
[releases page](https://github.com/oneup03/BanjoRecomp3D/releases/tag/rolling):

- **CIA** (`BanjoRecompiled3D-3ds-<commit>.cia`): install it with FBI or
  another CIA installer, or scan the QR code below with FBI
  (Remote Install > Scan QR Code). It appears on the HOME Menu as "Banjo:
  Recompiled". On an Old 3DS or 2DS, use the CIA (see Performance).
- **.3dsx** (`BanjoRecompiled3D-3ds-<commit>.zip`): extract it to the root of
  the SD card and start `3ds/BanjoRecomp/BanjoRecomp.3dsx` from the Homebrew
  Launcher.

![QR code for FBI: the latest CIA](https://github.com/oneup03/BanjoRecomp3D/releases/download/rolling/BanjoRecompiled3D-3ds-QR.png)

Both need your own ROM: the US 1.0 version of Banjo-Kazooie in .z64 format
(the file the desktop version asks for), at `sdmc:/3ds/BanjoRecomp/BK.z64`.
The game checks the ROM's hash at start: if it's another version or
damaged, it says so and leaves the file where it is.

Sound needs nothing more: the game brings its own DSP firmware, OpenDSP
(BSD 2-Clause, Copyright (c) 2026 Cellenseres; its licence is
`OpenDSP-LICENSE.txt` in the release zip), which also plays the game's
reverb. Should it fail to start, the game falls back to the console's own
firmware at `sdmc:/3ds/dspfirm.cdc`, without reverb: dump that once with
Luma3DS (the Rosalina menu, L + D-pad down + SELECT, then Miscellaneous
options > Dump DSP firmware).

On the SD card, once installed:

```
sdmc:/3ds/
  BanjoRecomp/
    BanjoRecomp.3dsx       the program (not needed with the CIA installed)
    BK.z64                 your ROM
    OpenDSP-LICENSE.txt    OpenDSP's licence (from the release zip)
    settings.ini           the settings (written by the game)
    saves/                 the saves, a folder for each game mode's
    screenshots/           SELECT-hold screenshots
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

The Controls page maps each button (A, B, X, Y, L, R, ZL, ZR and the
D-pad's directions) to any N64 button, adds up to two N64 buttons on the
touch screen, sets the Circle Pad's and C-Stick's deadzones, inverts the
camera, the first-person view, and flying and swimming (X, Y or both), can
turn off C-Stick up and down, and resets it all with Reset controls. The
Game page's Performance monitor setting hides the bottom screen's frame
rate and text; touch buttons stay. The Game page also sets each screen's
brightness.

SELECT opens the settings on the bottom screen; changes are kept in
`settings.ini`. Some built-in mods add buttons on the touch screen while
they're on: see Mods. The Game page also holds Audio and Particles (see
Performance), Music volume and Note & Jinjo saving: Both by default, as on
the desktop, so notes and jinjos you collect stay collected when you leave
a world or lose a life; Off is the original game's rule. A change to it
applies the next time you enter or leave a world.

The HUD is on the top screen by default. The Game page's HUD row can put
its counters on the touch screen instead, leaving the top screen to the
game. On the touch screen the pause screen's counters (honeycombs, notes,
eggs, feathers, jiggies, Mumbo tokens, jinjos, lives) stay up through play
instead of sliding away after a few seconds; a jiggy or token collected
counts up those totals rather than showing a second counter. Only what
counts in the area shows: the jinjos in the nine worlds, the notes
everywhere but Spiral Mountain. The file select and cutscenes show none,
and the Totals page hides them while it's open. The honeycombs sit at the top
in the middle and the other counters down the left (the air across the top
right), leaving the right half to touch buttons; more buttons than fit
there take their usual rows over the HUD with see-through fills. The pause
menu, dialogue and challenge timers stay on the top screen. The bottom
screen's frame rate and text are off by default (Performance monitor);
turned on, they draw over the counters there.

Holding SELECT for a second saves a screenshot to
`sdmc:/3ds/BanjoRecomp/screenshots/`: both eyes when 3D is on, and the touch
screen, as BMPs.

HOME pauses the game and asks on the touch screen whether to quit: A quits,
B goes back to the game. The HOME Menu itself isn't offered while the game
runs.

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

**Old 3DS / 2DS:** runs at 20-30 fps: about 25 in play, where the game asks
for 30, and 20 in the cutscenes that run at 20. Install the CIA rather than
running the .3dsx: the CIA asks an Old 3DS for 80 MB, enough to hold the
16 MB ROM in memory. The .3dsx gets only the Homebrew Launcher's 64 MB, so
it reads the ROM from the SD card as it needs it, and loads take a little
longer; play itself isn't slowed by the reads. It starts at the game's own
frame rate, as it can't afford the frames in between, with particles at
Quarter and Audio at Low priority.

The Game page's **Audio** row sets how the sound shares the processor with
the game. High priority, the New 3DS's default, interrupts the game whenever
the sound is about to run short. Low priority, the Old 3DS's default, never
interrupts it: about 5% more frames on an Old 3DS, but in busy scenes the
music can run a little slow or crackle. Off turns the sound off.

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

Names in **bold** below are buttons these mods put on the touch screen;
other buttons are the 3DS's, as in Controls.

| Mod | Author | What it does | How to use it |
|---|---|---|---|
| Ability Anywhere | ProxySaw | Flight and the Shock Jump without their pads | Standing or walking, tap **Fly / Shock jump** to take off, or hold it to Shock Jump. With "Require Ability Unlocks" on, only the moves you've learned. |
| Banjo Bee | TSR Stormed | Fly as Banjo Bee | **Bee flight** turns bee flight on or off (a message says which); then jump to fly. |
| Cheats | ProxySaw | Moon jump, extra speed, infinite health, lives, eggs and feathers, and air; Bottles' silly looks | Each is an option. Moon Jump: hold **Moon jump** to float up. Speed Modifier: hold R to run faster. The rest need no buttons. |
| Exit To Lair | ProxySaw | The game's unused "Exit to Lair" pause option | Pause (START) inside a world and choose it. |
| Quiet Talon Trot | PunishedBigs | Mutes Kazooie's running noise | Nothing to press; options choose which noises. |
| Sir Slush Configuration | Thermopolis | Tunes the Freezeezy Peak snowmen's snowballs | Nothing to press; options set their attacks. With its defaults they throw every 5 seconds (the original game: 0.8), and no faster while you fly. |
| Skip It | ProxySaw | Skips cutscenes, Bottles' lessons and Mumbo's talks and transformation scenes, speeds up or skips text, boots straight to the file select | Each is an option. "Skip Cutscenes: Press Start" skips a cutscene with START; the rest work by themselves. |
| Toggle Trot | ProxySaw | Talon Trot stays on without holding Z | Start Talon Trot as usual (hold the 3DS's L, which is Z, and press C-Left: ZL or D-pad left), then let go of L straight away and keep trotting; press and release L again to stop. Held longer, L stops the trot when you let go, as in the original. |
| Transform Anywhere | ProxySaw | Mumbo's transformations anywhere | **Washing machine**, **Croc / Walrus**, **Bee**, **Termite / Pumpkin**: each turns Banjo into its form; pressing it again moves to its next form, then back to Banjo. "Token Payment" (on at first) allows only the forms you've paid Mumbo's tokens for; turn it off for all of them. "Allowed Transformations: Current World" gives one **Transform** button, for the world you're in, and "Banjo" a **Back to Banjo** button. |

All are CC0 and built from source (`platform/3ds/mods/`, `mods.list`).

While one of these mods is on, its buttons appear on the touch screen under
the status lines (in its right half while the HUD is there): touch one to
use it (one at a time). None needs another
button held with it, so one finger plays them all, on an Old 3DS too. They
make way while the settings are open.

Banjo Bee and Transform Anywhere both read the N64 D-pad's down. With both
on, **Croc / Walrus** also toggles bee flight, and in Transform Anywhere's
default "Button Mode: D-pad", **Bee flight** also turns Banjo into the
crocodile. Set Button Mode to "L + D-pad" to keep **Bee flight** to itself.

To play these mods with the 3DS's buttons instead, map the N64 buttons
they read on the Controls page: a D-pad direction to the N64 D-pad's
(D-pad down to D-Down for Banjo Bee, all four for Transform Anywhere's
D-pad mode) and ZL to N64 L (Ability Anywhere's L + A, Cheats' moon jump).
The touch buttons stay either way. An Old 3DS has no ZL, ZR or C-Stick,
so the touch buttons suit it best.

In a game mode, Exit to Lair stays off and isn't listed: it returns to the
original game's lair entrances, which the game modes rework. It stays on for
the game itself if you turned it on.

### Languages

The Mods page's **Language (at restart)** row plays the game in English (the
original), Spanish or German. The choice takes effect the next time the game
starts.

| Language | Mod | Author |
|---|---|---|
| Espanol | [Spanish Language](https://github.com/wilrojasdev/BKSpanish) 3.2.0: Latin American Spanish, with dialogue, menus, the quiz and cutscenes. By its author's note the translation was generated with AI and may contain errors, and the worlds' signs aren't translated yet. | wilrojasdev |
| Deutsch | [German](https://github.com/V10lator/BK-german) 1.3.7 | V10lator |

Both are CC0 and built from source like the other mods. They replace the
game's text through Darío's
[Asset Expansion Pak](https://github.com/DarioSamo/BKRecompAssetExpansionPak)
(CC0), which the Language row turns on with them; on the 3DS its asset heap
is 2.5 MB rather than 8, to fit the game's memory. Languages stay off in the
game modes, whose text differs.

## AI use

The 3DS port, including this build's code and documentation and the
rt64-3ds renderer it uses, was written with the help of AI tools (Claude),
under the developer's direction and with their testing on real consoles.

## Building

```
git submodule update --init --recursive     # rt64-3ds has a submodule of its own
git submodule update --init --checkout platform/3ds/mods   # the built-in mods' sources
make -C platform/3ds nmr-patch host-tools   # once
make -C platform/3ds codegen                # after changing the ROM inputs or mods.list
make -C platform/3ds patches                # after changing patches/*.c alone
make -C platform/3ds                        # the .3dsx in build-3ds/
make -C platform/3ds cia                    # the CIA in build-3ds/
make -C platform/3ds deploy-azahar          # copy the .3dsx to the Azahar emulator's SD card
```

This needs devkitPro's `3ds-dev` group (`DEVKITPRO` defaults to
`/opt/devkitpro`), `makerom` and `bannertool` on the PATH for the CIA, clang
with the MIPS target and `ld.lld` for the patches (`PATCH_CC=`, `LLD=`,
`LLD_LIBS=` to name other ones) and the decompressed ROM in the repository
root as `decompressed.us.v10.z64`, as for the desktop build. `codegen`
downloads the game modes' pinned Thunderstore packages (their code only;
nothing from them is packaged). `RT64_3DS=<path>` builds against an rt64-3ds
checkout elsewhere.

On Fedora: `sudo dnf install clang lld cmake ninja-build pacman`, then
devkitPro's pacman setup for Fedora (https://devkitpro.org/wiki/devkitPro_pacman)
and `sudo pacman -S 3ds-dev`.
