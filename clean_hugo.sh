#!/bin/bash

# Clean previous Hugo build
echo "🧹 Cleaning previous Hugo build..."
rm -rf public/ resources/ .hugo_build.lock

# Generate a fresh Hugo build
echo "🚀 Running Hugo clean build..."
hugo --cleanDestinationDir

# Start Hugo server with full rebuild
echo "🌐 Starting Hugo server..."
hugo server --disableFastRender

