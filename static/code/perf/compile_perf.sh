#!/bin/bash

# Check if a source file is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <source_file.cpp>"
    exit 1
fi

# Extract the filename without extension
SRC_FILE="$1"
OUTPUT_FILE="${SRC_FILE%.*}"

# Detect if the file is C or C++
EXT="${SRC_FILE##*.}"
if [[ "$EXT" == "c" ]]; then
    COMPILER="gcc"
elif [[ "$EXT" == "cpp" ]]; then
    COMPILER="g++"
else
    echo "Error: Unsupported file type ($SRC_FILE). Only .c and .cpp are supported."
    exit 1
fi

# Compilation command
echo "Compiling $SRC_FILE -> $OUTPUT_FILE..."
$COMPILER -g -O2 -fno-omit-frame-pointer -march=native -o "$OUTPUT_FILE" "$SRC_FILE"

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Executable: $OUTPUT_FILE"
else
    echo "Compilation failed!"
    exit 1
fi
