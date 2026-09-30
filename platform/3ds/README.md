# Nintendo 3DS build

Banjo: Recompiled for the New Nintendo 3DS and New 2DS XL, with
stereoscopic 3D. The renderer and platform layer are
[rt64-3ds](https://github.com/oneup03/rt64-3ds) (`lib/rt64-3ds`).

Status: the game plays at its full frame rate (30 fps) with stereo 3D,
audio, the settings menu, the built-in mods and the three game modes in the
Azahar emulator. On a New 3DS it has been played through the intro and into
the game; the mods and game modes are so far checked in the emulator only.
In the emulator's Old 3DS mode the intro and the game modes' title and file
screens run at the rate the game asks for (20 or 30 fps), with the ROM read
from the SD card; levels there, and a real Old 3DS, are untested.

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
  mods/              game modes' .nrm files (see "Game modes")
  modes/             the game modes' ROMs, made on their first start
  saves/             saves; a game mode keeps its own in saves/<mod id>/
  mod_config/        the built-in mods' options
  mods.json          which built-in mods are on
  settings.ini       the settings (once one is changed)
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
| C-Stick | C buttons, or the analog camera (Controls settings) |

SELECT opens the settings on the bottom screen.

## Mods

Mods from the Banjo: Recompiled community are built into the executable (the
3DS cannot load mod code at runtime). They start switched off: turn them on
from the Mods page of the settings, which also holds their options. Turning
a mod on or off takes effect at the next start; options apply at once.

| Mod | Author | What it does |
|---|---|---|
| Ability Anywhere | ProxySaw | Fly or Shock Jump anywhere, no pad needed |
| Cheats | ProxySaw | Moon jump, speed, infinite health, lives, ammo and air |
| Exit To Lair | ProxySaw | An Exit to Lair option in the pause menu |
| Skip It | ProxySaw | Cutscene skips and speed-ups |
| Toggle Trot | ProxySaw | Talon Trot toggles instead of being held |
| Transform Anywhere | ProxySaw | Mumbo's transformations anywhere |
| Always High Poly Banjo | TSR Stormed | Always the high-poly Banjo model |
| Banjo Bee | TSR Stormed | Fly as Banjo Bee |
| Instant FFM | TSR Stormed | "No FFM setup required!" |
| Map Select | TSR Stormed | Warp to any map (opens with the N64 D-pad, which the 3DS does not map yet) |
| Quiet Talon Trot | PunishedBigs | Mutes Kazooie's running noise |
| Sir Slush Configuration | Thermopolis | Tunes Sir Slush's attacks |

All are CC0 and built from source (`platform/3ds/mods/`, `mods.list`).

In a game mode, Instant FFM, Map Select and Exit to Lair stay off and aren't
listed: Instant FFM changes the same function the game modes do (a game mode
would not start with it), and the other two are built around the original
game's maps. They stay on for the game itself if you turned them on.

## Game modes

Three ROM hacks can be played as game modes. Their code is built in; the
hack itself comes from the mod's file, which you download:

| Game mode | Version | Thunderstore package |
|---|---|---|
| BK The Jiggies of Time | 1.0.1 | [KurkoMods/BK_The_Jiggies_of_Time](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_The_Jiggies_of_Time/) |
| BK Nostalgia 64 | 1.0.1 | [KurkoMods/BK_Nostalgia_64](https://thunderstore.io/c/banjo-recompiled/p/KurkoMods/BK_Nostalgia_64/) |
| Banjo Dreamie | 1.3.2 | [Loggo/Banjo_Dreamie](https://thunderstore.io/c/banjo-recompiled/p/Loggo/Banjo_Dreamie/) |

Take the `.nrm` out of the package (a zip) and copy it to
`sdmc:/3ds/BanjoRecomp/mods/`. With one there, a menu at start offers the
game and each game mode. A mode that can't be played is listed in red with
the reason: the file it needs, or the version it needs when the file is
another version than the one above. The first start of a mode writes its
ROM (64 MB) to `sdmc:/3ds/BanjoRecomp/modes/`, which takes about 10 seconds
in the emulator; each mode keeps its own saves.

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
