# ModMenu macOS: agent notes

In-game mod settings menu for Cyberpunk 2077 2.3.1 (Steam, Apple silicon), a RED4ext plugin with a REDscript overlay. Branch `main`, remote `jackmaxwil/cp2077-modmenu`. User docs: `README.md`.

## Rules

- **macOS first.** macOS arm64 is the only target. No Win32 code.
- **Verified addresses only.** ModMenu adds no addresses of its own; it uses RED4ext.SDK APIs, whose addresses come from the SDK's canonical DB, where only entries marked verified resolve. RED4ext refuses a plugin that needs an unverified hash. `python3 vendor/RED4ext.SDK/scripts/plugin_requirements.py build-dev/libModMenu.dylib` must report 0 unverified (CI runs it). See `docs/ADDRESS_ANCHORS.md`.
- **Native hooks only.** ModMenu patches no game code: it registers script natives through RTTI, and the UI hooks game controllers from REDscript (`@wrapMethod`/`@addMethod`). Any future native hook goes through RED4ext's native hook engine.
- **Never launch the game or Steam from tooling.** In-game testing is done by the user, or by RED4ext's `tools/cp-run` / `tools/cp-regress` (scenarios `modmenu`, `modmenu-ui`) when asked.
- **Commits:** one logical change per commit, plain messages, no attribution or co-author lines. Work on `main`; never force-push.

## Layout

- `include/modmenu/modmenu_api.h` the public C API other plugins call (`ModMenu_GetApi`, optional `ModMenu_Register` export).
- `src/main.cpp` plugin entry, the script natives (`ModMenu_*`, declared in `scripts/Scripts/ModMenu/Natives.reds`) and ModMenu's own demo page. `src/modmenu_backend.*` the model, settings persistence (`red4ext/plugins/ModMenu/settings/<modId>.json`) and discovery of plugins exporting `ModMenu_Register`.
- `scripts/Scripts/ModMenu/` REDscript, installed as `red4ext/plugins/ModMenu/Scripts`. `InkHooks.reds` is the whole overlay and input listener.
- `scripts/r6/input/modmenu.xml` the `` ` ``/F10 binding, installed into the game's `r6/input/`.

RED4ext's `tools/cp-dev`, `tools/cp-gate` and `scripts/create_release.sh` use `libModMenu.dylib` (from `build-dev/` or `build-release/`), `scripts/Scripts/ModMenu` and `scripts/r6`. Keep those paths stable. The autotest scenarios call `ModMenu_Toggle`, `ModMenu_SelectMod`, `ModMenu_FlipToggle` and `ModMenu_IsModal`; keep them public.

## Build

```bash
cmake -S . -B build-dev -DCMAKE_BUILD_TYPE=Release
cmake --build build-dev -j8
```

Needs the submodule (`git submodule update --init --recursive`). `-DRED4EXT_SDK_DIR=<path>` overrides the SDK (cp-dev and create_release pass the workspace `../RED4ext.SDK`).

No offline REDscript compiler is available: keep `.reds` edits minimal and re-read the whole class after editing. A script error stops the game at script initialization.

## Debugging

- Script log: `<game>/red4ext/logs/modmenu.log`, appended by the `ModMenu_Log` native (overlay attach, toggle, modal, clicks).
- Loader log: `<game>/red4ext/logs/red4ext-*.log` names a refused plugin and the reason; the backend's own messages go through RED4ext's logger into `red4ext/logs/` too.
