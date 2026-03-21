@echo off
setlocal

echo Running compile commands preset...
cmake --preset compile_commands

echo Copying compile commands to project root...

copy .\build\compile_commands\compile_commands.json .
echo Done!