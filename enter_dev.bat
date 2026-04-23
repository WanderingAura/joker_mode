set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do (
    call "%%i\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
)