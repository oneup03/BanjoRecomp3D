# Driver for the 3DS build. Host code generation first, then the cross build.
#
#   make nmr-patch    apply the 3DS runtime patch series to lib/N64ModernRuntime (once)
#   make host-tools   build N64Recomp, RSPRecomp and file_to_c (once)
#   make codegen      run the recompiler over the ROM and the patches
#   make patches      the patches alone (after editing patches/*.c)
#   make              configure + build the .3dsx
#   make cia          installable title (needs makerom + bannertool)
#   make deploy-azahar / run-azahar / hw N3DS_IP=...
#
# Needs devkitPro's 3ds-dev (DEVKITPRO, default /opt/devkitpro), makerom and
# bannertool on the PATH for the CIA, clang with the MIPS target and ld.lld
# for the patches (PATCH_CC=... / LLD=... to name other ones; LLD_LIBS=...
# when that ld.lld needs its libraries on the loader path), the
# decompressed ROM in the repository root (decompressed.us.v10.z64, as for the
# desktop build) and the submodules, rt64-3ds (lib/rt64-3ds; RT64_3DS=... for a
# checkout elsewhere) among them.
ROOT       := $(abspath ../..)
RT64_3DS   ?= $(ROOT)/lib/rt64-3ds
BUILD      := $(ROOT)/build-3ds
HOST       := $(ROOT)/build-host
PATCH_CC   ?= clang
LLD        ?= ld.lld
LLD_LIBS   ?=
TARGET     := BanjoRecompiled3DS
AZAHAR_SD  := $(HOME)/.var/app/org.azahar_emu.Azahar/data/azahar-emu/sdmc
APPDIR     := $(AZAHAR_SD)/3ds/BanjoRecomp
# The name banjo.us.rev0.toml and n_aspMain.us.rev0.toml read the ROM under.
DECOMP_ROM := banjo.us.v10.decompressed.z64
MODS_OUT   := $(ROOT)/RecompiledMods
# Mods built into the executable (rt64-3ds tools/mod_bake): .nrm files, each
# with ":embed" to carry the file itself too (code mods; a game mode's comes
# from the SD card, with the ROM patch). Changing the list needs `codegen`.
BAKE_MODS  ?=

# devkitPro's own pacman installs to /opt/devkitpro (3ds-dev group).
export DEVKITPRO ?= /opt/devkitpro
export DEVKITARM ?= $(DEVKITPRO)/devkitARM

all: build

# The runtime needs a small patch series for the 3DS (kept in rt64-3ds so
# every game applies the same one). Idempotent.
nmr-patch:
	$(RT64_3DS)/nmr-patches/apply.sh $(ROOT)/lib/N64ModernRuntime

host-tools:
	cmake -S $(ROOT)/lib/N64ModernRuntime/N64Recomp -B $(HOST) -G Ninja -DCMAKE_BUILD_TYPE=Release
	cmake --build $(HOST) --target N64RecompCLI RSPRecomp -j
	g++ -std=c++17 -O2 -o $(HOST)/file_to_c $(ROOT)/lib/rt64/src/tools/file_to_c/file_to_c.cpp
	HOST=$(HOST) OUT=$(HOST)/mod_bake $(RT64_3DS)/tools/mod_bake/build.sh $(ROOT)

# The patches' ELF comes first: mod_bake reads which game functions they
# replace. The game's and the patches' recompilations then run on copies of
# their configs with mod_bake's hooks appended (a dispatch check at the entry
# of every function a baked mod replaces or hooks).
codegen:
	cd $(ROOT) && { [ -e $(DECOMP_ROM) ] || ln -s decompressed.us.v10.z64 $(DECOMP_ROM); }
	$(MAKE) patches-elf mods
	cd $(ROOT) && cat banjo.us.rev0.toml $(MODS_OUT)/game_hooks.toml > banjo.us.rev0.3ds.toml && $(HOST)/N64Recomp banjo.us.rev0.3ds.toml
	cd $(ROOT) && $(HOST)/RSPRecomp n_aspMain.us.rev0.toml
	$(MAKE) patches-recomp

# The patches alone (after editing patches/*.c). BK_3DS selects the 3DS
# variants in the patches; the desktop build leaves it undefined.
patches: patches-elf mods patches-recomp

patches-elf:
	cd $(ROOT)/patches && $(MAKE) clean && $(if $(LLD_LIBS),LD_LIBRARY_PATH=$(LLD_LIBS)) $(MAKE) CC=$(PATCH_CC) LD=$(LLD) PLATFORM_CPPFLAGS=-DBK_3DS

mods:
	rm -rf $(MODS_OUT)
	$(HOST)/mod_bake bake $(ROOT)/BanjoRecompSyms/bk.us.rev0.syms.toml $(ROOT)/patches/patches.elf $(MODS_OUT) $(BAKE_MODS)

patches-recomp:
	cd $(ROOT) && cat patches.toml $(MODS_OUT)/patch_hooks.toml > patches.3ds.toml && $(HOST)/N64Recomp patches.3ds.toml
	$(HOST)/file_to_c $(ROOT)/patches/patches.bin bk_patches_bin $(ROOT)/RecompiledPatches/patches_bin.c $(ROOT)/RecompiledPatches/patches_bin.h
	@grep -m1 -E '^ *0x[0-9a-f]+ +0x[0-9a-f]+ +[0-9a-f]+ +[0-9]+ +\.bss' $(ROOT)/patches/patches.map | \
	    awk '{ printf "patch .bss: 0x%s + 0x%s bytes\n", $$1, $$3 }' || true

configure:
	cmake -S . -B $(BUILD) -G Ninja -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_TOOLCHAIN_FILE=$(RT64_3DS)/cmake/devkitARM-3ds.cmake -DRT64_3DS_DIR=$(RT64_3DS)

build:
	@[ -f $(BUILD)/build.ninja ] || $(MAKE) configure
	cmake --build $(BUILD) -j
	@$(DEVKITARM)/bin/arm-none-eabi-size $(BUILD)/$(TARGET) | tail -1

deploy-azahar: build
	mkdir -p $(APPDIR)
	cp -f $(BUILD)/$(TARGET).3dsx $(APPDIR)/BanjoRecomp.3dsx
	@[ -e $(AZAHAR_SD)/3ds/dspfirm.cdc ] || : > $(AZAHAR_SD)/3ds/dspfirm.cdc
	@echo "deployed to $(APPDIR) (the ROM goes there as BK.z64)"

run-azahar: deploy-azahar
	flatpak run --env=QT_QPA_PLATFORM=xcb org.azahar_emu.Azahar -w $(APPDIR)/BanjoRecomp.3dsx

cia: build
	cmake --build $(BUILD) --target $(TARGET)_cia
	@ls -l $(BUILD)/$(TARGET).cia

hw: build
	$(DEVKITPRO)/tools/bin/3dslink -a $(N3DS_IP) -s $(BUILD)/$(TARGET).3dsx

clean:
	rm -rf $(BUILD)

.PHONY: all nmr-patch host-tools codegen patches patches-elf mods patches-recomp configure build cia deploy-azahar run-azahar hw clean
