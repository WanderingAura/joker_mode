:: This is a workaround for how slow Windows builds are due to CMake generating
:: bloated VS project files.

:: This only builds the game dll (which is all you need for hot reloading)

:: This script must be run inside of a developer terminal (i.e. it can see cl.exe)
:: And raylib.dll must already be built via the proper build command


@echo off
setlocal enabledelayedexpansion

pushd "%~dp0"

set "SRC_DIR=..\src"
set "UNITY_NAME=unity_windows"
set "UNITY_FILE=%UNITY_NAME%.c"

set "DLL_NAME=soc.dll"
set "EXE_NAME=society.exe"

del %UNITY_NAME%.c

echo ====================================
echo Generating %DLL_NAME% unity build...
echo ====================================
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

echo ====================================
echo Compiling %DLL_NAME% unity build...
echo ====================================

cl.exe !INCLUDES! ^
    /I"../build/debug/_deps/raylib-build/raylib/include" ^
    /LD ^
    /Zi ^
    /std:c11 ^
    /Fd"../" ^
    /W4 /we4716 /we4715 /wd4244 /wd4456 ^
    %UNITY_FILE% ^
    /link "../build/debug/_deps/raylib-build/raylib/raylib.lib" ^
    /OUT:"../%DLL_NAME%"

if %ERRORLEVEL% neq 0 (
    echo Build failed with error code %ERRORLEVEL%. Please check the logs above.
    exit /b %ERRORLEVEL%
)

del /Q %UNITY_NAME%.exp 2>nul
del /Q %UNITY_NAME%.lib 2>nul
del /Q %UNITY_NAME%.obj 2>nul
del /Q %UNITY_NAME%.c 2>nul

echo ====================================
echo Successfully compiled %DLL_NAME%
echo ====================================

if "%1" == "all" (
  echo Generating %EXE_NAME% unity build...
  set "VOS_DIR=..\src\vos"
  for %%F in ("!VOS_DIR!\*.c") do (

      :: Skip files containing "_linux"
      echo %%~nxF | findstr /I "_linux" >nul
      if errorlevel 1 (
          >> "%UNITY_FILE%" echo #include "%%~fF"
      )
  )

  set "BASED_DIR=..\src\based"
  for %%F in ("!BASED_DIR!\*.c") do (

      :: Skip files containing "_linux"
      echo %%~nxF | findstr /I "_linux" >nul
      if errorlevel 1 (
          >> "%UNITY_FILE%" echo #include "%%~fF"
      )
  )

  >> "%UNITY_FILE%" echo #include "..\src\app\main.c"

  echo Done

  echo ====================================
  echo Compiling %EXE_NAME% unity build...
  echo ====================================
  cl.exe !INCLUDES! ^
      /I"../build/debug/_deps/raylib-build/raylib/include" ^
      /Zi ^
      /std:c11 ^
      /Fd"../" ^
      /W4 /we4716 /we4715 /wd4244 /wd4456 ^
      %UNITY_FILE% ^
      /link "../build/debug/_deps/raylib-build/raylib/raylib.lib" ^
      /OUT:"../%EXE_NAME%"

  if %ERRORLEVEL% neq 0 (
      echo Build failed with error code %ERRORLEVEL%. Please check the logs above.
      exit /b %ERRORLEVEL%
  )
  del /Q %UNITY_NAME%.obj 2>nul
  del /Q %UNITY_NAME%.c 2>nul
  echo ====================================
  echo Successfully compiled %EXE_NAME%
  echo ====================================
)

popd