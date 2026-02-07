# ModMenu (v0.1)

An in-game mod menu for Cyberpunk 2077 on macOS ARM64, built as a RED4ext plugin.

## What this provides in v0.1

- Native backend (`ModMenu.dylib`) with:
  - A stable C ABI for other native plugins to register menu pages/entries
  - REDscript bridge functions (minimal: open/close + log tail)
- REDscript module (`Scripts/ModMenu/ModMenu.reds`) for future UI expansion
- Input mapping stub (`scripts/r6/input/modmenu.xml`) for a toggle hotkey

## Build

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

If CMake cannot find `RED4ext.SDK`, set:

```bash
cmake .. -DMODMENU_RED4EXT_SDK_DIR="/Users/jackmazac/Development/RED4ext.SDK"
```

## Install (layout)

Copy:

- `ModMenu.dylib` → `<game>/red4ext/plugins/ModMenu/ModMenu.dylib`
- `scripts/Scripts/` → `<game>/red4ext/plugins/ModMenu/Scripts/`
- `scripts/r6/input/modmenu.xml` → **merge snippet** for the game’s input mappings (see below)

Or use the helper installer:

```bash
./tools/install_to_game.sh
# optionally:
./tools/install_to_game.sh "$HOME/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077" ./build/libModMenu.dylib --install-input
```

## Make the hotkey work (adds the `modmenu_toggle` action)

ModMenu listens for `ListenerAction.IsAction(action, n"modmenu_toggle")`. To create that action:

```bash
./tools/patch_input.sh
rm -f "$HOME/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077/r6/cache/inputUserMappings.xml" \
      "$HOME/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077/r6/cache/inputContexts.xml"
```

This also writes an **overlay** file to:

- `<game>/r6/input/mods.xml`

…so tools like Input Loader can re-merge inputs without relying only on base-file edits.

Launch via:

```bash
cd "$HOME/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077"
./launch_red4ext.sh
```

