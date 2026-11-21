#!/usr/bin/env bash
# Prints the path of the Brewdeck checkout this UI builds against, fetching it first if needed.
#   BREWDECK=/path/to/brewdeck   use that checkout as it is (e.g. while changing Brewdeck itself)
#   otherwise                    .brewdeck/ at the release named in BREWDECK_REF (cloned once, moved when the file
#                                changes; git-ignored)
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
if [[ -n "${BREWDECK:-}" ]]; then
  cd "$BREWDECK" && pwd
  exit 0
fi
REF="$(tr -d '[:space:]' < "$HERE/BREWDECK_REF")"
DIR="$HERE/.brewdeck"
URL="${BREWDECK_URL:-https://github.com/pongells/brewdeck.git}"
if [[ ! -d "$DIR/.git" ]]; then
  echo "Fetching Brewdeck $REF into .brewdeck/ ..." >&2
  git -c advice.detachedHead=false clone --quiet --depth 1 --branch "$REF" "$URL" "$DIR" >&2 2>/dev/null ||
    { git -c advice.detachedHead=false clone --quiet "$URL" "$DIR" >&2 && git -C "$DIR" checkout --quiet "$REF" >&2; }   # a commit, not a tag
elif [[ "$(git -C "$DIR" describe --tags --exact-match 2>/dev/null || git -C "$DIR" rev-parse HEAD)" != "$REF"* ]]; then
  echo "Moving .brewdeck/ to Brewdeck $REF ..." >&2
  git -C "$DIR" fetch --quiet --depth 1 origin "refs/tags/$REF:refs/tags/$REF" >&2 2>/dev/null ||
    git -C "$DIR" fetch --quiet origin >&2
  git -C "$DIR" checkout --quiet "$REF" >&2
fi
echo "$DIR"
