#!/usr/bin/env bash
# build_mods.sh <RecompModTool> <out dir> [CC] [LD]: builds the code mods in
# mods.list from their submodules (mods/) into .nrm files in <out dir>.
#
# Each repository is copied to build-mods/ and built there with its Makefile
# and RecompModTool, against the bk-decomp headers and BanjoRecompSyms it
# pins. Those submodules are not checked out: one blobless clone of each
# serves every mod, and only the pinned commit's files a mod build reads
# (the decomp's include/) are exported.
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
tool=${1:?RecompModTool}; out=${2:?out dir}
cc=${3:-clang}; ld=${4:-ld.lld}
work=$root/build-mods
mkdir -p "$work" "$out"
rm -f "$out"/*.nrm

# pinned <mod repo> <submodule> <paths...>: a directory holding <paths> of
# the commit the mod pins for <submodule>.
pinned() {
    local src=$1 sub=$2; shift 2
    local url commit dir
    url=$(git -C "$src" config -f .gitmodules "submodule.$sub.url")
    commit=$(git -C "$src" ls-tree HEAD "$sub" | awk '{print $3}')
    dir=$work/pinned/$sub-$commit
    if [ ! -d "$dir" ]; then
        local repo=$work/pinned/$sub.git
        [ -d "$repo" ] || git clone -q --bare --filter=blob:none "$url" "$repo"
        git -C "$repo" cat-file -e "$commit^{commit}" 2>/dev/null || git -C "$repo" fetch -q origin "$commit"
        mkdir -p "$dir.tmp"
        git -C "$repo" archive "$commit" "$@" | tar -x -C "$dir.tmp"
        mv "$dir.tmp" "$dir"
    fi
    echo "$dir"
}

grep -v '^\s*#' "$here/mods.list" | while read -r repo package; do
    [ -n "$repo" ] || continue
    src=$here/mods/$repo
    if [ ! -f "$src/Makefile" ]; then
        echo "build_mods: $src is empty: git submodule update --init --checkout platform/3ds/mods" >&2
        exit 1
    fi
    dst=$work/$repo
    # A work copy, so the submodule stays clean.
    mkdir -p "$dst"
    rsync -a --delete --exclude .git --exclude build --exclude bk-decomp --exclude BanjoRecompSyms "$src/" "$dst/"
    ln -sfn "$(pinned "$src" bk-decomp include)" "$dst/bk-decomp"
    ln -sfn "$(pinned "$src" BanjoRecompSyms .)" "$dst/BanjoRecompSyms"
    # A thumbnail the mod lists but its repository lacks (BanjoBeeMod): the
    # 3DS shows none, so any image does.
    [ -e "$dst/thumb.png" ] || cp "$here/icon.png" "$dst/thumb.png"
    if [ -n "$package" ]; then
        # MAKE=true: its ELF rule ends in a post-build step that copies the
        # .nrm into a macOS application folder.
        make -C "$dst" -s CC="$cc" LD="$ld" MOD="$package" MAKE=true "build/packages/$package/mod.elf" >/dev/null
        (cd "$dst" && "$tool" "packages/$package/mod.toml" "build/packages/$package" >/dev/null)
        cp "$dst/build/packages/$package"/*.nrm "$out/"
    else
        make -C "$dst" -s CC="$cc" LD="$ld" >/dev/null
        (cd "$dst" && "$tool" mod.toml build >/dev/null)
        cp "$dst/build"/*.nrm "$out/"
    fi
    echo "build_mods: $repo${package:+/$package}"
done
