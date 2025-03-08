#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_executable>"
    exit 1
fi

EXECUTABLE="$1"
BASENAME=$(basename "$EXECUTABLE")  # Extracts filename from path
DATA_FILE="perf_${BASENAME}.data"   # Custom perf data file

echo "Ensuring proper permissions for perf profiling..."
echo 0 | sudo tee /proc/sys/kernel/kptr_restrict
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid

echo "Running perf profiling for $EXECUTABLE..."

# Step 1: Record performance data with a unique output file
perf record -F 99 -g -o "$DATA_FILE" -- "$EXECUTABLE"

echo "Perf profiling complete. Generating report..."

# Step 2: Show function-level performance analysis using the custom data file
perf report -i "$DATA_FILE" --stdio > "perf_${BASENAME}_report.txt"
echo "Perf function-level report saved to perf_${BASENAME}_report.txt"

# Step 3: Show line-level performance analysis using the custom data file
perf annotate -i "$DATA_FILE" > "perf_${BASENAME}_annotate.txt"
echo "Perf line-level analysis saved to perf_${BASENAME}_annotate.txt"

echo "Analysis complete! Check perf_${BASENAME}_report.txt and perf_${BASENAME}_annotate.txt for results."
