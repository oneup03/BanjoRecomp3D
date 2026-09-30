# Banjo: Recompiled for the Nintendo 3DS.
#
#   cmake -S platform/3ds -B build-3ds -G Ninja \
#         -DCMAKE_TOOLCHAIN_FILE=<rt64-3ds>/cmake/devkitARM-3ds.cmake
#
# The recompiled sources (RecompiledFuncs/, RecompiledPatches/, rsp/) are
# generated on the host first; see the Makefile beside this file.
cmake_minimum_required(VERSION 3.20)
project(BanjoRecompiled3DS LANGUAGES C CXX ASM)

set(BK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(RT64_3DS_DIR "${BK_ROOT}/lib/rt64-3ds" CACHE PATH "rt64-3ds checkout")
set(RT64_3DS_NMR_DIR "${BK_ROOT}/lib/N64ModernRuntime")
# The patches' RAM (patches/patches.ld, from 0x80801000) holds the enlarged
# display-list buffers, the audio heap and the frame buffers: about 4 MB,
# past rt64-3ds's default heap start. The build checks the link map.
set(RT64_3DS_HEAP_START 0x80D00000)
# The runtime's mod system, for mods baked into the executable (below).
set(RT64_3DS_STATIC_MODS ON)
add_subdirectory("${RT64_3DS_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/rt64-3ds")

# The generated game code. bk_host_protos.h makes the originals weak so the
# patched functions in RecompiledPatches replace them (GCC's RECOMP_FUNC is
# not weak, clang's is).
file(GLOB FUNC_SOURCES "${BK_ROOT}/RecompiledFuncs/*.c")
add_library(RecompiledFuncs STATIC ${FUNC_SOURCES} "${BK_ROOT}/RecompiledFuncs/lookup.cpp")
target_include_directories(RecompiledFuncs PRIVATE
    "${BK_ROOT}/RecompiledFuncs" "${CMAKE_CURRENT_LIST_DIR}"
    "${RT64_3DS_NMR_DIR}/N64Recomp/include" "${RT64_3DS_NMR_DIR}/ultramodern/include" "${RT64_3DS_NMR_DIR}/librecomp/include")
target_compile_options(RecompiledFuncs PRIVATE
    -O2 -fno-strict-aliasing -w -Wno-implicit-function-declaration -Wno-error=implicit-function-declaration
    -include "${CMAKE_CURRENT_LIST_DIR}/bk_host_protos.h")

add_library(RecompiledPatches STATIC
    "${BK_ROOT}/RecompiledPatches/patches.c"
    "${BK_ROOT}/RecompiledPatches/patches_bin.c")
target_include_directories(RecompiledPatches PRIVATE
    "${BK_ROOT}/RecompiledPatches" "${CMAKE_CURRENT_LIST_DIR}"
    "${RT64_3DS_NMR_DIR}/N64Recomp/include" "${RT64_3DS_NMR_DIR}/ultramodern/include" "${RT64_3DS_NMR_DIR}/librecomp/include")
target_compile_options(RecompiledPatches PRIVATE
    -O2 -fno-strict-aliasing -w -Wno-implicit-function-declaration -Wno-error=implicit-function-declaration)

# Mods baked into the executable: rt64-3ds tools/mod_bake's output for the
# Makefile's BAKE_MODS (each mod's code, the dispatch table the recompiled
# functions check, and the registration).
file(GLOB BAKED_MOD_SOURCES CONFIGURE_DEPENDS "${BK_ROOT}/RecompiledMods/*.c")
add_library(BakedMods STATIC ${BAKED_MOD_SOURCES} "${BK_ROOT}/RecompiledMods/static_mods.cpp")
target_link_libraries(BakedMods PRIVATE librecomp ultramodern)
target_compile_options(BakedMods PRIVATE -O2 -fno-strict-aliasing $<$<COMPILE_LANGUAGE:C>:-w>)

add_executable(BanjoRecompiled3DS
    main_3ds.cpp
    recomp_api_3ds.cpp
    "${BK_ROOT}/rsp/n_aspMain.cpp"
    "${BK_ROOT}/src/game/rom_decompression.cpp"
    "${BK_ROOT}/src/game/recomp_extension_api.cpp"
    "${BK_ROOT}/src/game/recomp_data_api.cpp"
    "${BK_ROOT}/src/main/register_overlays.cpp"
    "${BK_ROOT}/src/main/register_patches.cpp"
)
# shim/ stands in for the desktop UI library's headers (message boxes only).
target_include_directories(BanjoRecompiled3DS PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/shim" "${BK_ROOT}/include" "${BK_ROOT}"
    "${BK_ROOT}/lib/SlotMap" "${BK_ROOT}/RecompiledFuncs" "${BK_ROOT}/RecompiledPatches"
    "${RT64_3DS_NMR_DIR}/N64Recomp/include" "${RT64_3DS_NMR_DIR}/thirdparty" "${RT64_3DS_NMR_DIR}/thirdparty/miniz")
target_compile_options(BanjoRecompiled3DS PRIVATE -O2 -Wno-unused-parameter)
# The audio microcode runs scalar on the ARM11 and is the hottest file.
set_source_files_properties("${BK_ROOT}/rsp/n_aspMain.cpp" PROPERTIES COMPILE_OPTIONS "-O3;-fno-strict-aliasing")
target_link_libraries(BanjoRecompiled3DS PRIVATE
    recomp_3ds rt64_3ds RecompiledPatches RecompiledFuncs BakedMods ultramodern librecomp miniz citro3d ctru m)
target_link_options(BanjoRecompiled3DS PRIVATE "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/BanjoRecompiled3DS.map")
rt64_3ds_check_patch_ram(BanjoRecompiled3DS "${BK_ROOT}/patches/patches.map")
# Bring-up: accept the display lists and draw nothing.
option(BK_NULL_RENDERER "Build with rt64-3ds's null renderer" OFF)
if(BK_NULL_RENDERER)
    target_compile_definitions(BanjoRecompiled3DS PRIVATE BK_NULL_RENDERER)
endif()

set(BK_TITLE "Banjo: Recompiled")
set(BK_DESCRIPTION "Banjo-Kazooie Recompiled for New 3DS")
add_3dsx(BanjoRecompiled3DS TITLE "${BK_TITLE}" DESCRIPTION "${BK_DESCRIPTION}"
         ICON "${CMAKE_CURRENT_LIST_DIR}/icon.png")
include("${RT64_3DS_DIR}/cmake/Cia.cmake")
add_cia(BanjoRecompiled3DS RSF "${RT64_3DS_DIR}/cia/template.rsf" TITLE "${BK_TITLE}" PRODUCT "BANJO-3DS" UNIQUEID 0xF5B4E
        ICON "${CMAKE_CURRENT_LIST_DIR}/icon.png" DESCRIPTION "${BK_DESCRIPTION}")
