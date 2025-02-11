#!/bin/bash

echo "Cleaning Hugo cache, generated resources, and public directory..."

# Remove Hugo cache
if [ -d "$HOME/.cache/hugo" ]; then
    rm -rf "$HOME/.cache/hugo"
    echo "Deleted Hugo cache: $HOME/.cache/hugo"
else
    echo "No Hugo cache found."
fi

# Remove generated resources (_gen directory)
if [ -d "resources/_gen" ]; then
    rm -rf "resources/_gen"
    echo "Deleted generated resources: resources/_gen"
else
    echo "No generated resources found."
fi

# Remove public directory
if [ -d "public" ]; then
    rm -rf "public"
    echo "Deleted public directory."
else
    echo "No public directory found."
fi

# Remove Hugo module cache (if using modules)
hugo mod clean
echo "Cleared Hugo module cache."

# Optionally start Hugo server with --ignoreCache
read -p "Do you want to start Hugo server with --ignoreCache? (y/n): " choice
if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
    hugo server --ignoreCache
fi

echo "Hugo cleanup complete."

