#!/bin/bash

# Enable debugging mode
set -euo pipefail  # Exit on errors, unset variables, and pipe failures
LOG_FILE="/tmp/hugo.log"

echo "Navigating to ~/dev/algometix..."
cd ~/dev/algometix || { echo "Failed to navigate to root directory"; exit 1; }

echo "Starting Hugo server in the background..."
hugo server > "$LOG_FILE" 2>&1 &

# Wait a few seconds to allow Hugo to start
sleep 3

echo "Reading Hugo logs from $LOG_FILE..."
cat "$LOG_FILE"

echo "Extracting Hugo server URL..."
URL=$(grep -oE "http://localhost:[0-9]+" "$LOG_FILE" | head -n 1)

if [ -n "$URL" ]; then
    echo "Hugo server started at: $URL"
    echo "Opening in default browser..."
    xdg-open "$URL" &> /dev/null  # For Linux (Ubuntu)
else
    echo "Failed to extract Hugo server URL."
    echo "Debugging: Checking last 10 lines of log file..."
    tail -n 10 "$LOG_FILE"
    exit 1
fi

