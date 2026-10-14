# Nintendo 3DS build

Banjo: Recompiled for the New Nintendo 3DS and New 2DS XL, with
stereoscopic 3D. The renderer and platform layer are
[rt64-3ds](https://github.com/oneup03/rt64-3ds) (`lib/rt64-3ds`).

Status: the game plays at its full frame rate with stereo 3D, audio, the
settings menu, the built-in mods and the three game modes. On a New 3DS it
has been played for hours, levels and a game mode (Jiggies of Time)
included, with the interpolated 60 fps mode below. At the Old 3DS clock
with two cores (`dbg_old_clock` and `dbg_cores` in `settings.ini` on a New 3DS) it
runs at about 16-19 frames a second where it asks for 20 or 30, with the
ROM read from the SD card; a real Old 3DS is untested.

## Installing

Extract the `.3dsx` to `sdmc:/3ds/BanjoRecomp/BanjoRecomp.3dsx` and start it
from the Homebrew Launcher, or install the `.cia` (with FBI, for instance)
to start it from the HOME Menu. Both use the same folder on the SD card.

It needs your own ROM: the US 1.0 version of Banjo-Kazooie in .z64 format
(the file the desktop version asks for), at `sdmc:/3ds/BanjoRecomp/BK.z64`.
The game checks the ROM's hash at start and deletes a file that does not
match, so copy it rather than move it.

What the folder holds once the game has run:

```
sdmc:/3ds/BanjoRecomp/
  BanjoRecomp.3dsx   the program (not needed with the CIA installed)
  BK.z64             your ROM
  mods/              game modes' ROMs, made on a PC (see "Game modes")
  saves/             saves; a game mode keeps its own in saves/<mod id>/
  settings.ini       the settings, the built-in mods' included (once one is changed)
  log.txt            the last run's log, useful in a bug report
```

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

## Performance

A New 3DS runs the game at its full frame rate (30 fps in play, 20 in many
cutscenes) and, by default, draws interpolated frames between each two of
the game's (the Game page's Frame rate: 60 interp.): one while the game
runs at 30, two in the cutscenes at 20. A scene too heavy to draw twice in
time drops back to the game's rate until it fits again, and frames that
render into a texture (the jiggy transition, the picture puzzles) go
without. An Old 3DS defaults to the game's own frame rate, as it can't
afford the frames in between.

The `stats:` line in `log.txt` says how many frames were shown and why
lists went without an in-between; see "Testing the Old 3DS on a New 3DS"
below for the rows that mimic an Old 3DS.

The Game page's **Particles** row (Full, Half, Quarter; an Old 3DS starts
at Quarter) scales down what the particle emitters put out. Each particle is
a draw of its own, with its own sprite frame and colour, so the renderer's
cost per particle is a draw's: in a capture of a jiggy's sparkles, 225 of
the frame's 360 draws were sparkles. Half divides every burst by two and
halves the timed emitters' rate; nothing is switched off, since effects end
when their particles run out.

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

## Game modes

Three ROM hacks can be played as game modes. Their code is built in; each
needs its ROM, which you make on a PC from your `BK.z64` and the mod's file:

| Game mode | Version | Thunderstore package |
|---|---|---|
| BK The Jiggies of Time | 1.0.1 | [KurkoMods/BK_The_Jiggies_of_Time](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_The_Jiggies_of_Time/) |
| BK Nostalgia 64 | 1.0.1 | [KurkoMods/BK_Nostalgia_64](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_Nostalgia_64/) |
| Banjo Dreamie | 1.3.2 | [Loggo/Banjo_Dreamie](https://thunderstore.io/c/banjo-recompiled/p/Loggo/Banjo_Dreamie/) |

Take the `.nrm` out of the package (a zip) and run
[`make_game_mod_rom.py`](https://github.com/oneup03/rt64-3ds/blob/main/tools/make_game_mod_rom.py)
on it (Python 3, nothing else to install; `lib/rt64-3ds/tools/` in this
repository):

```
python3 make_game_mod_rom.py BK.z64 bk_jiggies_of_time.nrm bk_nostalgia_64.nrm Loggo_BanjoDreamie.nrm
```

It writes a `mods` folder with each mode's ROM (64 MB) and a small `.ok`
file beside it: copy both into `sdmc:/3ds/BanjoRecomp/mods/`. The `.nrm`
itself isn't needed on the SD card. Use the versions above: a ROM made from
another version is refused. The console doesn't make these itself: the
patch alone is more than an Old 3DS has memory for.

With a mode's ROM there, a menu at start offers the game and each game
mode. A mode that can't be played is listed in red with the reason: its ROM
missing, made from another version, or incomplete. Each mode keeps its own
saves.

## Testing the Old 3DS on a New 3DS

Two keys in `settings.ini` (beside the executable; not on the menu) give a
New 3DS most of an Old 3DS's limits:
- `dbg_old_clock=1`: 268 MHz without the L2 cache, applied at start.
- `dbg_cores`: the two-core layouts an Old 3DS has, read at start. `2`
  ("2, game on 0") is what an Old 3DS gets for this game: the game and the
  reading of each display list on core 0, the drawing and audio on core 1.
  `1` ("2, game on 1") swaps the cores, for comparison. Both leave the
  in-between builder off.

The console still has its extra cores and more memory, so it runs a little
better than a real Old 3DS would.

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
