:: This is a workaround for how slow Windows builds are due to CMake generating
:: bloated VS project files.

:: This only builds the game dll (which is all you need for hot reloading)

:: This script must be run inside of a developer terminal (i.e. it can see cl.exe)
:: And raylib.dll must already be built via the proper build command

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do (
    call "%%i\Common7\Tools\VsDevCmd.bat"
)

:: Your build commands here

powershell -NoProfile -ExecutionPolicy Bypass -File generate_unity.ps1 ..\src

cl.exe /I"../src/based" /I"../src/efs" /I"../src/game" /I"../src/gameplay" /I"../src/http" /I"../src/level" /I"../src/render" /I"../src/vos" ^
    /I"build/debug/_deps/raylib-build/raylib/include" ^
    /LD ^
    /std:c11 /permissive- ^
    "src\soc_dll_unity_windows.c" ^
    /link "build/debug/_deps/raylib-build/raylib/raylib.lib" ^
    /OUT:"soc.dll"

del /Q soc_dll_unity_windows.exp 2>nul
del /Q soc_dll_unity_windows.lib 2>nul
del /Q soc_dll_unity_windows.obj 2>nul