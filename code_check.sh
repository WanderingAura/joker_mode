#!/bin/env bash

SCRIPT_PATH=${0%/*}
if [ "$0" != "$SCRIPT_PATH" ] && [ "$SCRIPT_PATH" != "" ]; then 
    cd $SCRIPT_PATH
fi

if [ ! -f "./compile_commands.json" ] ; then
    ./generate_compilecommands.sh
fi

CODE_CHECK_LOG=clang-tidy-diag.log
echo "Running clang-tidy..."
run-clang-tidy src/**/*.c > ${CODE_CHECK_LOG}
echo "Clang tidy output has been stored in ${CODE_CHECK_LOG}"