#!/usr/bin/env bash
# Brewdeck's shared UI checks against this UI: its emulator runs every scenario (a fake machine, no window) and the
# results go to checks/: status.md (each feature of Brewdeck's docs/ui-features.md: done, partial, failed, missing,
# omitted, not checked yet or by hand, with ui/ui-features.txt for what this UI leaves out), sheet.html (screenshots,
# for the checks done by looking) and the logs. Needs SDL2, like ./emu.sh.
#   ./checks.sh         every scenario (about 10 minutes: some wait through a fast-forwarded half hour)
#   ./checks.sh w1      only the scenarios whose file name starts with w1
# Exit code 1 when a scenario failed or ui/ui-features.txt breaks the rules; a missing feature is not a failure.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BD="$("$HERE/tools/brewdeck.sh")"
if [[ ! -x "$BD/tests/ui/run.sh" ]]; then   # the checks came after v0.2.0
  echo "Brewdeck $(tr -d '[:space:]' < "$HERE/BREWDECK_REF") has no shared UI checks yet (tests/ui/run.sh):" \
       "they need a newer Brewdeck in BREWDECK_REF (or BREWDECK=<a Brewdeck checkout>). Skipped."
  exit 0
fi
UI_OUT="$HERE/checks" SCENARIO="${1:-}" exec "$BD/tests/ui/run.sh" "$HERE/ui"
