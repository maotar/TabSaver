#!/bin/bash

# TabSaver macOS Installation Script
# This script removes quarantine flags and installs the plugins

set -e

echo "TabSaver Installation Script"
echo "=============================="
echo ""

# Check if running on macOS
if [[ "$(uname)" != "Darwin" ]]; then
    echo "Error: This script is for macOS only"
    exit 1
fi

# Function to install a plugin
install_plugin() {
    local plugin_path="$1"
    local dest_dir="$2"
    local plugin_name=$(basename "$plugin_path")

    if [ ! -e "$plugin_path" ]; then
        echo "Warning: $plugin_name not found, skipping..."
        return
    fi

    echo "Installing $plugin_name..."

    # Remove quarantine flag
    xattr -cr "$plugin_path" 2>/dev/null || true

    # Create destination directory if it doesn't exist
    mkdir -p "$dest_dir"

    # Copy plugin
    cp -r "$plugin_path" "$dest_dir/"

    echo "  ✓ $plugin_name installed to $dest_dir"
}

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Install VST3
if [ -d "$SCRIPT_DIR/TabSaver.vst3" ]; then
    install_plugin "$SCRIPT_DIR/TabSaver.vst3" "$HOME/Library/Audio/Plug-Ins/VST3"
fi

# Install AU
if [ -d "$SCRIPT_DIR/TabSaver.component" ]; then
    install_plugin "$SCRIPT_DIR/TabSaver.component" "$HOME/Library/Audio/Plug-Ins/Components"
fi

echo ""
echo "Installation complete!"
echo ""
echo "Please restart your DAW to load the plugin."
echo ""
