# Brewdeck UI template

Start your own touchscreen UI for Gaggiuino machines on the [Brewdeck](https://github.com/pongells/brewdeck) core.
The core does Wi-Fi, finding the machine, its REST and WebSocket APIs, settings storage and every command sent to the
machine (with its safety checks); your UI only draws and reacts to touches. This repository is a working starting
point: a small LVGL 9 UI (live values, the active profile, the running shot, a profile list where a tap selects one),
scripts that build it into Brewdeck's emulator and firmware, and CI for both supported boards.

Unofficial: Brewdeck and UIs built on it are not made by or affiliated with the Gaggiuino project. Please say so in
your UI too (this one does, at the bottom of its screen).

## Start

1. Press **Use this template** on GitHub (or clone this repository). Call yours `brewdeck-ui-<name>` and add the
   topic `brewdeck-ui`, so people can find it.
2. In `ui/myui.cpp`, set `BD_UI`'s name to `<name>` (lower case, at most 15 characters: it is also where your UI's
   settings are saved, so keep it once people use it). Rename the file if you like: every `.c` / `.cpp` file in
   `ui/` is built.
3. Run it in the emulator: `./emu.sh` (a fake machine with recorded data, in a window; mouse = touch). Needs a C++
   compiler and SDL2 (macOS: `brew install sdl2`; Debian/Ubuntu: `sudo apt install libsdl2-dev`).
4. Build the firmware: `./fw.sh ui` (Sunton ESP32-8048S043, ESP32-S3) or `./fw.sh ui-p4` (Guition JC4880P443,
   ESP32-P4). Needs [uv](https://docs.astral.sh/uv/) (it runs PlatformIO); the first build downloads the toolchain
   and takes several minutes.
5. Flash it: read Brewdeck's README, "Installing", first (a backup of the board's original flash is required, and a
   Gaggiuino screen has encrypted flash), then point `GAGGI_BACKUP` at your backup:
   `GAGGI_BACKUP=/path/to/backup.bin ./fw.sh ui --flash` (or `ui-p4`). `DRY_RUN=1` shows what it would do.

The first `./emu.sh` or `./fw.sh` fetches Brewdeck into `.brewdeck/` (git-ignored).

## Check it

`./checks.sh` runs Brewdeck's shared UI scenarios against your UI in the emulator (no window, about 10 minutes) and
writes `checks/` (git-ignored): `status.md`, your UI's status for every feature in Brewdeck's
[feature list](https://github.com/pongells/brewdeck/blob/main/docs/ui-features.md) (done, partial, failed, missing,
omitted, not checked yet, by hand), and `sheet.html`, screenshots of every scenario with the features to check by
looking at the top. The scenarios find your objects by tags (LVGL object names, the list's tag table; `myui.cpp`
names two); a feature whose tag is missing counts as missing, not failed. What your UI leaves out on purpose goes in
`ui/ui-features.txt` (it starts as the example's list). CI runs the checks on every push and keeps `checks/` as the
`ui-checks` artifact. They need a Brewdeck release that has them (after v0.2.0); with an older one they are skipped.

## Which Brewdeck

`BREWDECK_REF` names the Brewdeck release your UI builds against (a tag such as `v0.2.0`). Within the core's API
version 1, every newer release keeps your UI compiling and working, so updating is a one-line change: put the new tag
in `BREWDECK_REF` and build again. Brewdeck's release notes say what the API gained. To build against a Brewdeck
checkout of your own (e.g. while changing Brewdeck itself), set `BREWDECK=/path/to/brewdeck`.

## Learn the API

- [`brewdeck_core.h`](https://github.com/pongells/brewdeck/blob/main/firmware/src/core/brewdeck_core.h): the whole
  API, one header (in `.brewdeck/firmware/src/core/` after the first build).
- [Writing your own UI](https://github.com/pongells/brewdeck/blob/main/docs/custom-ui.md): the guide (rules, reading
  state, events, the screen, memory and settings, testing).
- Brewdeck's own UI (`firmware/src/ui` there) is the large example: every screen, the settings, the shot review.
- [Ledger](https://github.com/pongells/brewdeck-ui-ledger): a complete UI in its own repository, built like this
  template (a light look, its own fonts, every screen in one file).
- The emulator's keys and script commands (`tap x y`, `key s` for a shot, `save f.raw` for a screenshot, ...):
  Brewdeck's [development notes](https://github.com/pongells/brewdeck/blob/main/docs/development.md).

## Fonts

LVGL 9's Montserrat 14 / 20 / 48 are available. Other fonts are converted to C files with
[lv_font_conv](https://github.com/lvgl/lv_font_conv) and put in `ui/` like any source file; a converted font holds only
the characters you list, so list every character your texts can show (digits, `°`, `·`, letters with accents).
Example: `npx lv_font_conv --bpp 4 --size 24 --font MyFont.ttf -r 0x20-0x7E,0xB0 --format lvgl --no-compress
--lv-font-name my_24 --lv-include lvgl.h -o ui/my_24.c`, then `LV_FONT_DECLARE(my_24);` in your code.

## License

This template is under the MIT license (`LICENSE`), so your UI can use any license you choose. The Brewdeck core it
is built with is under the PolyForm Noncommercial License 1.0.0.
