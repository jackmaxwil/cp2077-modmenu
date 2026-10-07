# Address anchors (v0.1)

ModMenu v0.1 **does not introduce new SDK hashes** or new `cyberpunk2077_addresses.json` entries.

## Why

- The UI is implemented in **REDscript**, attached via script-level hooks to existing game UI controllers.
- The native backend only uses **public RED4ext APIs** (SDK + loader), plus RTTI registration.

## What ModMenu relies on

These come from the SDK's canonical address DB (`cyberpunk2077_addresses.json`), which resolves only entries marked verified:

- RTTI system availability (`CRTTISystem::Get()`) for native function registration
- Scripts API (`sdk->scripts->Add(...)`) so ModMenu can ship its own `.reds` sources

## Validation

`RED4ext.SDK/scripts/plugin_requirements.py ModMenu.dylib` lists the hashes compiled into ModMenu and whether each is verified. All of them are verified, so RED4ext loads the plugin.

