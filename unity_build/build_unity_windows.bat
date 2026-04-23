:: This is a workaround for how slow Windows builds are due to CMake generating
:: bloated VS project files.

:: This only builds the game dll (which is all you need for hot reloading)

:: This script must be run inside of a developer terminal (i.e. it can see cl.exe)
:: And raylib.dll must already be built via the proper build command

:: Your build commands here


@echo off
setlocal enabledelayedexpansion

pushd "%~dp0"

echo Generating unity build...
set "SRC_DIR=..\src"
set "UNITY_FILE=soc_dll_unity_windows.c"

REM Normalize SRC_DIR to absolute form
for %%a in ("%SRC_DIR%") do set "SRC_DIR=%%~fa"

> "%UNITY_FILE%" (
  for /r "%SRC_DIR%" %%f in (*.c) do (
    set "name=%%~nxf"

    REM Exclude main.c
    if /i not "!name!"=="main.c" (

      REM Exclude *_linux.c
      echo "!name!" | findstr /i "_linux" >nul
      if errorlevel 1 (

        REM Full absolute path
        set "full=%%~ff"

        REM Convert to relative: replace base with ..\src
        set "rel=!full:%SRC_DIR%=..\src!"

        echo #include "!rel!"
      )
    )
  )
)


for %%a in ("%~dp0..\src") do set "SRC_DIR=%%~fa"

set "INCLUDES="

for /d %%d in ("%SRC_DIR%\*") do (
  set "dir=%%~fd"
  set "rel=!dir:%SRC_DIR%=..\src!"
  set "INCLUDES=!INCLUDES! /I"!rel!""
)

echo Done

echo Compiling unity build...

cl.exe !INCLUDES! ^
    /I"../build/debug/_deps/raylib-build/raylib/include" ^
    /LD ^
    /std:c11 /permissive- ^
    %UNITY_FILE% ^
    /link "../build/debug/_deps/raylib-build/raylib/raylib.lib" ^
    /OUT:"../soc.dll"

del /Q soc_dll_unity_windows.exp 2>nul
del /Q soc_dll_unity_windows.lib 2>nul
del /Q soc_dll_unity_windows.obj 2>nul
del /Q soc_dll_unity_windows.c 2>nul

echo Done
popd