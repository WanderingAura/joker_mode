#!/bin/bash

# this script generates the compile commands for this project to enable syntax highlighting for
# clangd. this is needed because we use CMAKE_UNITY_BUILD by default which messes up compile commands

cmake --preset compile_commands
if [ $? -ne 0 ]; then
  echo "cmake configure failed for compile_commands"
  exit 1
fi
cp build/compile_commands/compile_commands.json .
rm -rf build/compile_commands
