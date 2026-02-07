#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path


def scan_script_actions(root: Path) -> set[str]:
    actions: set[str] = set()
    # Hotkey pattern:
    #   ListenerAction.IsAction(action, n"my_action")
    # Keep it line-local to avoid false matches spanning multiple lines.
    pat_listener = re.compile(r"\bListenerAction\.IsAction\s*\(\s*[^,\n]+,\s*n\"([^\"]+)\"")

    for reds in root.rglob("*.reds"):
        try:
            text = reds.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        actions.update(pat_listener.findall(text))
    return actions


def main() -> int:
    game_dir = Path(sys.argv[1]) if len(sys.argv) >= 2 else Path.home() / "Library/Application Support/Steam/steamapps/common/Cyberpunk 2077"

    scripts_dir = game_dir / "r6" / "scripts"
    input_user = game_dir / "r6" / "config" / "inputUserMappings.xml"
    input_ctx = game_dir / "r6" / "config" / "inputContexts.xml"
    input_ctx_mac = game_dir / "r6" / "config" / "inputContexts_mac.xml"

    if not scripts_dir.exists():
        print(f"scripts_dir_missing={scripts_dir}")
        return 2

    actions = scan_script_actions(scripts_dir)
    print(f"found_actions={len(actions)}")
    if not actions:
        return 0

    def file_contains(p: Path, needle: str) -> bool:
        try:
            return needle in p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            return False

    missing_actions: list[str] = []
    missing_mappings: list[str] = []
    for a in sorted(actions):
        if input_ctx.exists() and not file_contains(input_ctx, f'action name="{a}"'):
            missing_actions.append(a)
        if input_ctx_mac.exists() and not file_contains(input_ctx_mac, f'action name="{a}"'):
            missing_actions.append(f"{a} (mac)")
        if input_user.exists() and not file_contains(input_user, f'mapping name="{a}"'):
            missing_mappings.append(a)

    if missing_actions:
        print("missing_actions=" + ",".join(missing_actions))
    if missing_mappings:
        print("missing_mappings=" + ",".join(missing_mappings))

    return 0 if not missing_actions and not missing_mappings else 1


if __name__ == "__main__":
    raise SystemExit(main())

