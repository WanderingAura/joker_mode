@echo off
setlocal

echo Checking for CMake...

:: Check if cmake.exe is available in PATH
where cmake >nul 2>&1
if %errorlevel%==0 (
    echo CMake is already installed.
) else (
    echo CMake not found. Installing...

    :: Define download URL and installer name
    set "CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2-windows-x86_64.msi"
    set "INSTALLER=cmake_installer.msi"

    echo Downloading CMake installer...
    powershell -command "(New-Object Net.WebClient).DownloadFile('%CMAKE_URL%', '%INSTALLER%')"

    if not exist "%INSTALLER%" (
        echo Failed to download CMake installer.
        exit /b 1
    )

    echo Installing CMake silently...
    msiexec /i "%INSTALLER%" /qn ADD_CMAKE_TO_PATH=System

    if %errorlevel% neq 0 (
        echo CMake installation failed.
        exit /b 1
    )

    echo CMake installed successfully.
)

echo Running CMake workflow...

cmake --workflow --preset %1

echo Done.

