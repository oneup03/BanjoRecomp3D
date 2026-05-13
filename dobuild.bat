@echo off
set "VSINSTALLER=C:\Program Files (x86)\Microsoft Visual Studio\Installer"
set "PATH=%VSINSTALLER%;%PATH%"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set "PATH=C:\ProgramData\chocolatey\bin;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin;%PATH%"
cmake --build "C:\Users\oneup\source\repos\oneup03\BanjoRecomp3D\build-cmake" --target BanjoRecompiled --config Release
exit /b %ERRORLEVEL%
