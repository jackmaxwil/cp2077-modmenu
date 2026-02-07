#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="${1:-0.1.0}"

BUILD_DYLIB="${2:-$ROOT_DIR/build/libModMenu.dylib}"
OUT_DIR="$ROOT_DIR/dist"
STAGE_DIR="$OUT_DIR/.stage-ModMenu-$VERSION"
ZIP_NAME="ModMenu-v${VERSION}-macos-arm64.zip"

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/red4ext/plugins/ModMenu"

if [[ ! -f "$BUILD_DYLIB" ]]; then
  echo "Built dylib not found: $BUILD_DYLIB" >&2
  echo "Build first: cmake -S . -B build && cmake --build build" >&2
  exit 1
fi

cp -f "$BUILD_DYLIB" "$STAGE_DIR/red4ext/plugins/ModMenu/ModMenu.dylib"
cp -R "$ROOT_DIR/scripts/Scripts" "$STAGE_DIR/red4ext/plugins/ModMenu/Scripts"
cp -R "$ROOT_DIR/scripts/config" "$STAGE_DIR/red4ext/plugins/ModMenu/config"
cp -R "$ROOT_DIR/scripts/r6" "$STAGE_DIR/red4ext/plugins/ModMenu/r6"
cp -f "$ROOT_DIR/README.md" "$STAGE_DIR/red4ext/plugins/ModMenu/README.md"

mkdir -p "$OUT_DIR"
rm -f "$OUT_DIR/$ZIP_NAME"

(
  cd "$STAGE_DIR"
  /usr/bin/zip -r "$OUT_DIR/$ZIP_NAME" .
)

rm -rf "$STAGE_DIR"

echo "Wrote: $OUT_DIR/$ZIP_NAME"

