# ModMenu macOS — Agent Guidelines

## Project Context

ModMenu is a Cyberpunk 2077 mod that provides an in-game settings overlay. Native C++ backend with REDscript UI, loaded as a RED4ext `.dylib` plugin.

## Current Status (Canonical)

See `docs/STATUS.md` for runtime validation checklist and architecture details.

## Development Practices

### Architecture

1. **REDscript-only UI.** All UI is in `scripts/Scripts/ModMenu/InkHooks.reds`. No native C++ UI hooks.
2. **Native bridge.** 22 functions registered via RED4ext RTTI for REDscript to call into C++.
3. **Settings persistence.** Per-mod JSON files in `settings/<modId>.json`.
4. **F10 toggle.** Input binding in `scripts/r6/input/modmenu.xml`.

### Platform Awareness

1. **macOS ARM64 only.** No Windows compatibility needed.
2. **No RTTI address guessing.** UI hooks via REDscript `@addMethod`, not native address resolution.
3. **No native function hooks.** ModMenu registers script natives through RTTI; it does not patch game code.

### Code Standards

1. **C++20** for plugin code.
2. **spdlog** for all logging.
3. **PascalCase** for public functions, `camelCase` for private.

## Key Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Plugin entry + bridge registrations |
| `src/modmenu_backend.cpp` | Data model, persistence, discovery |
| `scripts/Scripts/ModMenu/InkHooks.reds` | Full overlay UI |
| `scripts/Scripts/ModMenu/ModMenuUI.reds` | Data classes |
| `scripts/r6/input/modmenu.xml` | Input binding |

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Testing

1. Copy `libModMenu.dylib` to `<game>/red4ext/plugins/ModMenu/ModMenu.dylib`
2. Copy `scripts/Scripts/` and `scripts/r6/` to plugin directory
3. Launch via `launch_red4ext.sh`
4. Press F10 to toggle overlay

## Common Pitfalls

1. **Input binding not merged.** `modmenu.xml` must be merged into `inputUserMappings.xml`.
2. **REDscript not installed.** Scripts must be in the correct plugin subdirectory.
3. **Bridge function mismatch.** If REDscript calls a function that isn't registered, it silently fails.
