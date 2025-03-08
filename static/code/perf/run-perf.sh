#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status.

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <executable-path>"
    exit 1
fi

EXECUTABLE=$1

echo "Running perf stat for $EXECUTABLE..."
perf stat $EXECUTABLE

echo "Checking available perf events..."
PERF_EVENTS="cache-references,cache-misses"
if ! perf list | grep -q "cache-references"; then
    echo "Warning: 'cache-references' not found. Using L1-dcache-loads and L1-dcache-load-misses instead."
    PERF_EVENTS="L1-dcache-loads,L1-dcache-load-misses"
fi

echo "Using perf events: $PERF_EVENTS"

echo "Measuring cache references and misses..."
perf stat -e $PERF_EVENTS $EXECUTABLE

echo "Measuring L1 cache loads and misses..."
perf stat -e L1-dcache-loads,L1-dcache-load-misses $EXECUTABLE

echo "Measuring LLC (Last-Level Cache) behavior..."
perf stat -e LLC-loads,LLC-load-misses $EXECUTABLE
