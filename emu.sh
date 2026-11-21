#!/usr/bin/env bash
# Builds this UI into Brewdeck's emulator and runs it: the real UI code in a window, against a fake machine with
# recorded data (no hardware, no machine). Needs SDL2 (macOS: brew install sdl2; Debian/Ubuntu: libsdl2-dev).
#   ./emu.sh            build and open the window (mouse = touch; keys and script commands: Brewdeck's
#                       docs/development.md, "Without hardware")
#   ./emu.sh build      build only
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BD="$("$HERE/tools/brewdeck.sh")"
OUT="$(UI="$HERE/ui" "$BD/sim/build-emu.sh" | tee /dev/stderr | tail -1)"   # its last line: "built sim/<binary>"
[[ "${1:-}" == build ]] || exec "$BD/${OUT#built }"
