@echo off
REM Unified configure + build script for BanjoRecompiled.
REM
REM   build.bat          - configure (only if build-cmake has no cache) then build
REM   build.bat clean    - delete the CMake cache first, forcing a fresh configure
REM
REM Use the `clean` form if a CMakeLists change leaves the cache in a bad state
REM (symptom: regen picks the wrong lld-link and fails with an x86/x64 machine
REM type mismatch).
setlocal

REM Repo root = this script's directory (strip trailing backslash).
set "REPO=%~dp0"
if "%REPO:~-1%"=="\" set "REPO=%REPO:~0,-1%"
set "BLD=%REPO%\build-cmake"
set "PATCHES_LLVM=%REPO%\..\portable-llvm\LLVM-19.1.3-Windows-X64\bin"

if /i "%~1"=="clean" (
    echo [build] Clean requested - removing CMake cache...
    if exist "%BLD%\CMakeCache.txt" del /q "%BLD%\CMakeCache.txt"
    if exist "%BLD%\CMakeFiles" rmdir /s /q "%BLD%\CMakeFiles"
)

REM Bring in the VS x64 developer environment. Prepend the x64 LLVM toolchain
REM (NOT the 32-bit ...\Llvm\bin) so any in-build CMake regen keeps using the
REM x64 lld-link and doesn't trip the machine-type mismatch.
set "VSINSTALLER=C:\Program Files (x86)\Microsoft Visual Studio\Installer"
set "PATH=%VSINSTALLER%;%PATH%"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set "PATH=C:\ProgramData\chocolatey\bin;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin;%PATH%"

if not exist "%BLD%\CMakeCache.txt" (
    echo [build] Configuring CMake...
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl ^
        -DPATCHES_C_COMPILER="%PATCHES_LLVM%\clang.exe" ^
        -DPATCHES_LD="%PATCHES_LLVM%\ld.lld.exe" ^
        -S "%REPO%" -B "%BLD%"
    if errorlevel 1 exit /b %errorlevel%
)

echo [build] Building BanjoRecompiled...
cmake --build "%BLD%" --target BanjoRecompiled --config Release
exit /b %errorlevel%
