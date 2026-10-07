# ModMenu (macOS)

In-game mod settings menu for Cyberpunk 2077 on macOS ARM64.

**Status:** Build validated — REDscript UI, 22 native bridge functions, F10 toggle.

## What it does

ModMenu provides an in-game overlay for configuring mod settings. Mods register toggles, sliders, and action buttons via a C ABI. The UI is built entirely in REDscript (no fragile native UI hooks), rendered on the HUD controller, and toggled with F10.

## Prerequisites

- RED4ext installed and functional
- CMake 3.24+, Clang 15+

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Install

```bash
cp build/libModMenu.dylib "<game>/red4ext/plugins/ModMenu/ModMenu.dylib"
cp -r scripts/Scripts/ "<game>/red4ext/plugins/ModMenu/Scripts/"
cp -r scripts/r6/ "<game>/red4ext/plugins/ModMenu/r6/"
```

Merge `scripts/r6/input/modmenu.xml` into the game's `r6/config/inputUserMappings.xml`.

## Key files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Plugin entry + 22 bridge function registrations |
| `src/modmenu_backend.cpp` | Data model, persistence, plugin discovery |
| `scripts/Scripts/ModMenu/InkHooks.reds` | Full overlay UI + input handling |
| `scripts/r6/input/modmenu.xml` | Input binding (` or F10) |
| `docs/STATUS.md` | Port status |

## Related projects

| Project | Description |
|---------|-------------|
| [RED4ext](../RED4ext) | Required mod loader |
| [RED4ext.SDK](../RED4ext.SDK) | SDK dependency |
| [CyberMod Studio](../cybermod-studio) | GUI mod manager |
