# Address anchors (v0.1)

ModMenu v0.1 **does not introduce new SDK hashes** or new `cyberpunk2077_addresses.json` entries.

## Why

- The UI is implemented in **REDscript**, attached via script-level hooks to existing game UI controllers.
- The native backend only uses **public RED4ext APIs** (SDK + loader), plus RTTI registration.

## What ModMenu relies on

These are already part of the existing SDK database (126 entries) and loader database (134 entries):

- RTTI system availability (`CRTTISystem::Get()`) for native function registration
- Scripts API (`sdk->scripts->Add(...)`) so ModMenu can ship its own `.reds` sources

## Validation

In the current workspace, the address databases validate cleanly:

- `scripts/check_addresses.py --strict`: `total=126`, `zero_offsets=0`
- `scripts/check_loader_addresses.py --strict`: `total=134`, `zero_offsets=0`, `missing_required_hooks=0`

