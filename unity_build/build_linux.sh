#!/usr/bin/env bash

CURRENT_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${CURRENT_SCRIPT_DIR}

echo "Generating unity build..."
find ../src -type f -name "*.c" ! -name "*_windows*" ! -name "main.c" | awk '{print "#include \"" $0 "\""}' > soc_dll_unity_linux.c
echo "Done"

INCLUDE_DIRECTORIES=$(find ../src -mindepth 1 -maxdepth 1 -type d | awk '{print "-I" $0}')

echo "Compiling..."
gcc $INCLUDE_DIRECTORIES -isystem "../build/debug/_deps/raylib-build/raylib/include" \
    -fPIC -shared -o ../libsoc.so soc_dll_unity_linux.c \
    -g -std=gnu11 -fPIC -Wall -Wextra -Werror -Wno-sign-compare
echo "Done"

rm soc_dll_unity_linux.c
cd - > /dev/null