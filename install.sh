#!/usr/bin/env bash

set -e

echo "⛵ Installing Taskmanager-Harbor..."

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 1. Build Project
echo "🛠 Building C++ executable with CMake & Ninja..."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. Determine Install Targets (System-wide vs User-local)
if [ "$EUID" -eq 0 ]; then
    BIN_DIR="/usr/local/bin"
    ICON_DIR="/usr/share/icons/hicolor/scalable/apps"
    DESKTOP_DIR="/usr/share/applications"
    echo "📌 Installing system-wide..."
else
    BIN_DIR="$HOME/.local/bin"
    ICON_DIR="$HOME/.local/share/icons/hicolor/scalable/apps"
    DESKTOP_DIR="$HOME/.local/share/applications"
    echo "📌 Installing for current user ($USER)..."
fi

# Ensure directories exist
mkdir -p "$BIN_DIR"
mkdir -p "$ICON_DIR"
mkdir -p "$DESKTOP_DIR"

# 3. Copy Binary
echo "📦 Installing binary to $BIN_DIR/taskmanager-harbor..."
cp -f "$SCRIPT_DIR/build/Taskmanager-Harbor" "$BIN_DIR/taskmanager-harbor"
chmod +x "$BIN_DIR/taskmanager-harbor"

# 4. Copy Icon
echo "🎨 Installing custom SVG icon to $ICON_DIR/taskmanager-harbor.svg..."
cp -f "$SCRIPT_DIR/assets/taskmanager-harbor.svg" "$ICON_DIR/taskmanager-harbor.svg"

# 5. Copy Desktop Entry
echo "🚀 Installing desktop launcher to $DESKTOP_DIR/taskmanager-harbor.desktop..."
cp -f "$SCRIPT_DIR/taskmanager-harbor.desktop" "$DESKTOP_DIR/taskmanager-harbor.desktop"
chmod +x "$DESKTOP_DIR/taskmanager-harbor.desktop"

# 6. Update Caches if available
if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "${ICON_DIR%/*/*/*}" 2>/dev/null || true
fi

if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
fi

echo "✅ Taskmanager-Harbor installation complete!"
echo "You can now start it from your application menu or by running: taskmanager-harbor"
