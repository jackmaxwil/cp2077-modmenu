# ModMenu for macOS

ModMenu is an in-game settings menu for Cyberpunk 2077 mods on macOS. It runs as a RED4ext plugin on Cyberpunk 2077 2.3.1 (Steam, Apple silicon). Other plugins add their settings pages to it (toggles, sliders, buttons), and ModMenu saves each value between sessions.

## Install

ModMenu is part of the RED4ext macOS release. Follow [RED4ext's install guide](https://github.com/jackmaxwil/RED4ext-macos/blob/main/docs/INSTALL_MACOS.md):

1. Unzip the release over the game folder (`~/Library/Application Support/Steam/steamapps/common/Cyberpunk 2077`).
2. Run the one-time setup, `red4ext/macos/scripts/install_macos.sh`, from the game folder.
3. Start the game with `launch_red4ext.sh` from the game folder. The Steam Play button starts the game without mods.

ModMenu ends up in `red4ext/plugins/ModMenu/`, and its key binding in `r6/input/modmenu.xml`.

## Use it

1. Load a save. In gameplay, press `` ` `` (backtick, the key left of 1) or F10. The menu opens as a popup, so the game stops sending input to gameplay and shows the mouse cursor.
2. Click a mod in the list on the left. Its settings appear on the right.
3. Click a toggle to switch it on or off. Use ` - ` and ` + ` for sliders, and `[Execute]` for buttons.
4. Close the menu with Esc, `` ` ``, F10 or the Close button.

Values are saved to `red4ext/plugins/ModMenu/settings/<modId>.json` and restored the next time the game starts.

## For mod authors

Another RED4ext plugin adds a settings page through the C API in [`include/modmenu/modmenu_api.h`](include/modmenu/modmenu_api.h). Copy that header into your plugin. RED4ext's plugin load order is not fixed, so register both ways; exactly one of them runs:

```c
#include <dlfcn.h>
#include "modmenu/modmenu_api.h"

static void OnGodMode(const ModMenuEntryPath* path, bool on) { /* apply on */ }

// ModMenu calls this when it loads, if your plugin loaded first. In C++, declare it extern "C".
__attribute__((visibility("default"))) bool ModMenu_Register(const ModMenuApi* api)
{
    ModMenuModInfo mod = {.modId = {"MyMod"}, .name = {"My Mod"}, .version = {"1.0"}};
    ModMenuPageInfo page = {.pageId = {"main"}, .title = {"Main"}};
    ModMenuToggleInfo god = {.entryId = {"god_mode"}, .title = {"God mode"}, .onChanged = OnGodMode};
    return api->RegisterMod(&mod) && api->RegisterPage("MyMod", &page) &&
           api->RegisterToggle("MyMod", "main", &god);
}

// In your RED4ext Main, on EMainReason::Load (covers ModMenu loading first):
const ModMenuApi* (*getApi)(void) = (const ModMenuApi* (*)(void))dlsym(RTLD_DEFAULT, "ModMenu_GetApi");
if (getApi)
    ModMenu_Register(getApi());
```

- Entry types: toggle (`RegisterToggle`), slider (`RegisterSlider`: min, max, step, default), button (`RegisterButton`). `MODMENU_ENTRY_TEXT` is shown as a plain label; there is no register call for it yet.
- IDs (mod, page, entry) are stable keys for the saved values. Do not rename them between versions.
- A saved value replaces `defaultValue` at registration. Your `onChanged` callback runs when the player changes it.

## Troubleshooting

- **The menu does not open.** Start the game with `launch_red4ext.sh`, not with Steam. Check that `r6/input/modmenu.xml` exists in the game folder. Check `red4ext/logs/red4ext-*.log`: if RED4ext refused ModMenu, it says why there.
- **The key does nothing in menus.** That is expected. The key works in gameplay and while ModMenu is open, not in the pause menu, inventory or other game menus.
- **My mod is not in the list.** Check that your plugin registers both ways shown above, and that `RegisterMod` returns true.
- **Game was updated.** RED4ext refuses to load plugins on a game build it does not know, so ModMenu does not load until a new release is out.

## Build from source

Requires Xcode command line tools and CMake (`brew install cmake`).

```bash
git clone --recursive https://github.com/jackmaxwil/cp2077-modmenu.git
cd cp2077-modmenu
cmake -S . -B build-dev -DCMAKE_BUILD_TYPE=Release
cmake --build build-dev -j8
```

The result is `build-dev/libModMenu.dylib` (installed as `red4ext/plugins/ModMenu/ModMenu.dylib`). CMake uses the `vendor/RED4ext.SDK` submodule; pass `-DRED4EXT_SDK_DIR=<path>` to use another [RED4ext.SDK-macos](https://github.com/jackmaxwil/RED4ext.SDK-macos) checkout. In the workspace, RED4ext's `tools/cp-dev install --plugins ModMenu` builds and installs ModMenu with its scripts, and `scripts/create_release.sh` packages it.

To check that every game address ModMenu uses is verified: `python3 vendor/RED4ext.SDK/scripts/plugin_requirements.py build-dev/libModMenu.dylib`.

## Credits and license

ModMenu is written for the macOS port of RED4ext. There is no license file in this repository yet.
