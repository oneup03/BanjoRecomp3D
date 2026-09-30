# Nintendo 3DS build

Banjo: Recompiled for the New Nintendo 3DS and New 2DS XL, with
stereoscopic 3D. The renderer and platform layer are
[rt64-3ds](https://github.com/oneup03/rt64-3ds) (`lib/rt64-3ds`).

Status: under construction. The game boots, and the intro, title screen and
file select render; audio runs the recompiled microcode (slow); stereo,
the settings and the built-in mods are still to come.

## Installing

Extract the `.3dsx` to `sdmc:/3ds/BanjoRecomp/BanjoRecomp.3dsx` and start it
from the Homebrew Launcher.

It needs your own ROM: the US 1.0 version of Banjo-Kazooie in .z64 format
(the file the desktop version asks for), at `sdmc:/3ds/BanjoRecomp/BK.z64`.
The game checks the ROM's hash at start and deletes a file that does not
match, so copy it rather than move it. Saves and `settings.ini` are kept in
the same folder.

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

## Building

```
git submodule update --init --recursive     # rt64-3ds has a submodule of its own
make -C platform/3ds nmr-patch host-tools   # once
make -C platform/3ds codegen                # after changing the ROM inputs or the patches
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
