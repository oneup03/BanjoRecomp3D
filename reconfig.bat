@echo off
set "VSINSTALLER=C:\Program Files (x86)\Microsoft Visual Studio\Installer"
set "PATH=%VSINSTALLER%;%PATH%"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\ProgramData\chocolatey\bin;%PATH%"
set "PATCHES_LLVM=C:\Users\oneup\source\repos\oneup03\portable-llvm\LLVM-19.1.3-Windows-X64\bin"
set "SRC=C:\Users\oneup\source\repos\oneup03\BanjoRecomp3D"
set "BLD=C:\Users\oneup\source\repos\oneup03\BanjoRecomp3D\build-cmake"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DPATCHES_C_COMPILER="%PATCHES_LLVM%\clang.exe" -DPATCHES_LD="%PATCHES_LLVM%\ld.lld.exe" -S "%SRC%" -B "%BLD%"
exit /b %ERRORLEVEL%
