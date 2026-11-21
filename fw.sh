#!/usr/bin/env bash
# Builds the firmware with this UI, through Brewdeck's firmware/build.sh (and flashes it with --flash).
#   ./fw.sh ui              Sunton ESP32-8048S043 (ESP32-S3)      ./fw.sh ui-p4        Guition JC4880P443 (ESP32-P4)
#   ./fw.sh ui-debug        ... with the debug web server (screenshots, recorder, remote touch; trusted networks only)
#   ./fw.sh ui-p4 --flash   build and flash. Read Brewdeck's README, "Installing", first: flashing needs a backup of
#                           the board's original flash, and a Gaggiuino screen has encrypted flash (build.sh checks).
# Env as for build.sh: PORT, GAGGI_BACKUP (path to the backup), DRY_RUN=1 (show what --flash would do).
# The firmware lands in <brewdeck>/firmware/.pio/build/<env>/ (firmware.bin and friends).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
BD="$("$HERE/tools/brewdeck.sh")"
UI="$HERE/ui" exec "$BD/firmware/build.sh" "$@"   # the UI folder: ui/ (the checkout must stay outside it)
