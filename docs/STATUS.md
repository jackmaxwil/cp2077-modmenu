# ModMenu macOS Status

> **Last updated:** 2026-02-21
> **Target:** Cyberpunk 2077 macOS arm64

## Current state

- Backend plugin (`ModMenu.dylib`) builds and loads on macOS.
- 22 REDscript bridge functions registered for menu state, mod/page/entry enumeration, value get/set, and diagnostics.
- Full REDscript-based overlay UI implemented (F10 toggle):
  - Sidebar with installed mod list
  - Page tab navigation per mod
  - Toggle (ON/OFF), slider (+/-), and action button entry types
  - All wired to native bridge functions
- Demo toggle callback wired and emits backend log on change.

## Architecture decision: REDscript-only UI

Native C++ UI hooks (`modmenu_ui.cpp`, `modmenu_ui_hooks.cpp`, `modmenu_ui_address.cpp`) have been removed from the build. All UI is implemented in REDscript via `@addMethod` on `inkGameController`, avoiding fragile RTTI-based address resolution.

## v0.1 scope

- C ABI registration API for native plugins (`ModMenu_Register`).
- In-memory model of mods/pages/entries with toggle/slider/button support.
- Settings persistence per-mod in `settings/<modId>.json`.
- Diagnostics helpers (`GetRed4extLogTail`, plugin log tails, incompatible plugin scan, report bundle generation).
- Full overlay UI via REDscript (InkHooks.reds).

## Input binding

- **F10** (`IK_F10`) toggles the mod menu overlay.
- Defined in `scripts/r6/input/modmenu.xml` (merge snippet for `inputUserMappings.xml`).

## Runtime validation checklist

- [ ] F10 opens/closes the overlay
- [ ] Mod list populates from registered mods
- [ ] Page tabs switch between pages
- [ ] Toggle entries flip ON/OFF and persist across game restarts
- [ ] Slider +/- adjustments work and values persist
- [ ] Action buttons trigger callbacks
- [ ] Status label visible on HUD when menu is closed

## Install quick reference

```bash
cp build/libModMenu.dylib "<game>/red4ext/plugins/ModMenu/ModMenu.dylib"
cp -r scripts/Scripts/ "<game>/red4ext/plugins/ModMenu/Scripts/"
cp -r scripts/r6/ "<game>/red4ext/plugins/ModMenu/r6/"
```

Merge `scripts/r6/input/modmenu.xml` into the game's `r6/config/inputUserMappings.xml` and clear input caches.

## Key files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Plugin entry + 22 bridge function registrations |
| `src/modmenu_backend.cpp` | Backend data model, persistence, plugin discovery |
| `scripts/Scripts/ModMenu/InkHooks.reds` | Full overlay UI + input handling |
| `scripts/Scripts/ModMenu/ModMenuUI.reds` | Data classes for UI callbacks |
| `scripts/Scripts/ModMenu/Events.reds` | ModMenuToggleEvent |
| `scripts/r6/input/modmenu.xml` | Input binding (F10) |
