#!/bin/bash

# this script generates the compile commands for this project to enable syntax highlighting for
# clangd. this is needed because we use CMAKE_UNITY_BUILD by default which messes up compile commands

cmake --preset compile_commands > tmp.log 2>&1
if [ $? -ne 0 ]; then
  echo "cmake configure failed for compile_commands"
  if [ -f tmp.log ]; then
    rm tmp.log
  fi
  exit 1
fi
BINARY_DIR=$(grep CMAKE_BINARY_DIR tmp.log | awk '{print $2}')
rm tmp.log
cp "${BINARY_DIR}/compile_commands.json" .
