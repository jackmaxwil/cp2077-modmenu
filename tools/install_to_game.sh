#!/usr/bin/env bash
set -euo pipefail

GAME_DIR_DEFAULT="$HOME/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

GAME_DIR="${1:-$GAME_DIR_DEFAULT}"
BUILD_DYLIB="${2:-$ROOT_DIR/build/libModMenu.dylib}"

INSTALL_INPUT=0
if [[ "${3:-}" == "--install-input" ]]; then
  INSTALL_INPUT=1
fi

PLUGIN_DIR="$GAME_DIR/red4ext/plugins/ModMenu"
R6_SCRIPTS_DIR="$GAME_DIR/r6/scripts/ModMenu"

if [[ ! -d "$GAME_DIR" ]]; then
  echo "Game directory not found: $GAME_DIR" >&2
  exit 1
fi

if [[ ! -f "$BUILD_DYLIB" ]]; then
  echo "Built dylib not found: $BUILD_DYLIB" >&2
  echo "Build first: cmake -S . -B build && cmake --build build" >&2
  exit 1
fi

mkdir -p "$PLUGIN_DIR"
cp -f "$BUILD_DYLIB" "$PLUGIN_DIR/ModMenu.dylib"

rm -rf "$PLUGIN_DIR/Scripts" "$PLUGIN_DIR/config" "$PLUGIN_DIR/r6"
cp -R "$ROOT_DIR/scripts/Scripts" "$PLUGIN_DIR/Scripts"
cp -R "$ROOT_DIR/scripts/config" "$PLUGIN_DIR/config"
cp -R "$ROOT_DIR/scripts/r6" "$PLUGIN_DIR/r6"

echo "Installed ModMenu to: $PLUGIN_DIR"

# Also install REDscript sources into r6/scripts so the standalone redscript compiler (launcher)
# picks them up.
rm -rf "$R6_SCRIPTS_DIR"
mkdir -p "$(dirname "$R6_SCRIPTS_DIR")"
cp -R "$ROOT_DIR/scripts/Scripts/ModMenu" "$R6_SCRIPTS_DIR"
echo "Installed REDscript to: $R6_SCRIPTS_DIR"

if [[ "$INSTALL_INPUT" -eq 1 ]]; then
  "$ROOT_DIR/tools/patch_input.sh" "$GAME_DIR"
  rm -f "$GAME_DIR/r6/cache/inputUserMappings.xml" "$GAME_DIR/r6/cache/inputContexts.xml" || true
  echo "Patched input mappings + cleared input cache."
else
  echo "Hotkey not installed. Run: \"$ROOT_DIR/tools/patch_input.sh\" \"$GAME_DIR\""
fi

echo "Launch with: \"$GAME_DIR/launch_red4ext.sh\""

