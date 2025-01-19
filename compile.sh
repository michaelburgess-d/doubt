#!/bin/bash
# cspell:disable 

# Exit immediately if a command exits with a non-zero status
set -e

# Function to display error messages
function error_exit {
    echo "$1" 1>&2
    exit 1
}

# Clear the screen including scrollback
clear && printf '\e[3J'

# Remove the previous executable and debug symbols
rm -f doubt_test
rm -rf doubt_test.dSYM

# if --tidy is passed, skip clang-tidy checks
if [ "$1" == "--tidy" ]; then
    # Define clang-tidy checks (adjust as needed)
    CLANG_TIDY_CHECKS="-*,clang-analyzer-*,performance-*,bugprone-*,modernize-*,readability-*,cppcoreguidelines-*,misc-*"

    # Run clang-tidy and capture its output and exit code
    echo "Running clang-tidy... (this can take a minute)"
        # --fix \
    clang-tidy doubt.c \
        --use-color \
        --warnings-as-errors \
        --checks="${CLANG_TIDY_CHECKS}" \
        -- \
        -Wall -Wextra -Wpedantic -Werror \
        -Wno-unused-parameter -Wno-extra-semi \
        -fno-omit-frame-pointer -fsanitize=undefined,address \
        -fstack-protector-strong -DCOMPILING -std=c23
        

    CLANG_TIDY_EXIT_CODE=$?

    if [ "$CLANG_TIDY_EXIT_CODE" -ne 0 ]; then
        echo "Compilation aborted due to clang-tidy issues."
        exit "$CLANG_TIDY_EXIT_CODE"
    else
        echo "clang-tidy passed with no issues."
    fi
fi


# Compilation flags
COMPILER_FLAGS=(
    -Wall -Wextra -Wpedantic -Werror
    -Wno-unused-parameter -Wno-extra-semi
    -fno-omit-frame-pointer -fsanitize=undefined,address
    -fstack-protector-strong 
    -fcolor-diagnostics
    -g -O0
    -std=c23
    -DTEST
    -DEXAMPLES
    -DNATIVE_LOG_CALLS
    -DCOMPILING
    -o doubt_test
    doubt.c
)

# Compile the code
echo "Compiling the code..."
if clang "${COMPILER_FLAGS[@]}"; then
    echo "Compilation succeeded."
else
    error_exit "Compilation failed."
fi

# Run the debugger with optional custom script
CUSTOM_SCRIPT=""  # Define your custom script if needed
echo "Starting debugger..."
lldb "$CUSTOM_SCRIPT" -o 'r' -o 'bt' -- ./doubt_test

# Clean up executable and debug symbols after debugging
rm -f doubt_test
rm -rf doubt_test.dSYM
echo "Debugger exited and cleanup completed."
