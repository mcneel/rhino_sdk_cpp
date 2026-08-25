#!/usr/bin/env bash
#
# Regenerate the macOS .tbd link stubs in lib/ from a Rhino.app bundle.
#
# The plug-in SDK only needs these frameworks at *link* time; at runtime a
# plug-in resolves them from inside Rhino via their @rpath install names.  So
# rather than committing the full multi-MB framework dylibs, we commit small
# text-based .tbd stubs (the macOS equivalent of Windows import libraries),
# generated with Apple's `tapi stubify`.
#
# Run this whenever the Mac frameworks are updated to a new Rhino build.
#
# Usage:
#   script/stubify_frameworks.sh /Applications/RhinoBETA.app
#
set -euo pipefail

APP="${1:-}"
if [ -z "$APP" ] || [ ! -d "$APP" ]; then
  echo "Usage: $0 <path-to-Rhino.app>" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_DIR="$SCRIPT_DIR/../lib"
FW_DIR="$APP/Contents/Frameworks"

for f in RhCore OpenNURBS RhMaterialEditor; do
  src="$FW_DIR/$f.framework/Versions/A/$f"
  if [ ! -f "$src" ]; then
    echo "Missing framework binary: $src" >&2
    exit 1
  fi
  xcrun tapi stubify "$src" -o "$LIB_DIR/$f.tbd"
  echo "wrote lib/$f.tbd  (archs: $(xcrun lipo -archs "$src" | tr '\n' ' '))"
done

echo "Done. Regenerated .tbd stubs in lib/ from: $APP"
