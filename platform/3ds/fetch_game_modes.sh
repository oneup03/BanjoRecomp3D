#!/usr/bin/env bash
# fetch_game_modes.sh <out dir>: the game modes whose code is baked into the
# executable. Downloads each one's Thunderstore package (a pinned version,
# checked by SHA-256) and extracts its .nrm for mod_bake. Nothing from them is
# committed or packaged: players bring the .nrm, whose patch.bps makes the ROM
# hack, and the runtime checks it is the version baked here.
set -euo pipefail
out=${1:?out dir}
mkdir -p "$out"
while read -r url sha nrm; do
    [ -n "$url" ] || continue
    [ -f "$out/$nrm" ] && continue
    zip=$out/$(basename "$nrm" .nrm).zip
    curl -fsSL -o "$zip" "$url"
    echo "$sha  $zip" | sha256sum -c --quiet
    unzip -oq -d "$out" "$zip" "$nrm"
    rm -f "$zip"
    echo "fetch_game_modes: $nrm"
done <<'LIST'
https://thunderstore.io/package/download/KurkoMods/BK_The_Jiggies_of_Time/1.0.1/ 6fa9dbaf86c5dde7999c148a9e27a2cc1fb861b634c959b1707a4b3b346aee64 bk_jiggies_of_time.nrm
https://thunderstore.io/package/download/KurkoMods/BK_Nostalgia_64/1.0.1/ 926b8447421070fd6e43fd307d6b80a7370efcd76e3b445cfef35c930ee2ef4d bk_nostalgia_64.nrm
https://thunderstore.io/package/download/Loggo/Banjo_Dreamie/1.3.2/ 5c7a5cef344e7a82610ff59a730ab8c439cce0cc112a4f866c33d88150b2b9f3 Loggo_BanjoDreamie.nrm
LIST
