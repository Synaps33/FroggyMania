#!/usr/bin/env bash
# FroggyMania - Sonic Mania (RSDKv5 decompilation) for SF2000 / GB300
# Copies the core and launcher stubs onto the console's SD card.
#
# Usage:  ./install.sh /path/to/sd/card [gb300v2|sf2000]
set -euo pipefail

TARGET="${1:-}"
VARIANT="${2:-gb300v2}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

die() { echo "error: $*" >&2; exit 1; }

case "$VARIANT" in
  gb300v2|sf2000) ;;
  *) die "unknown variant '$VARIANT' (use gb300v2 or sf2000)" ;;
esac

if [ -z "$TARGET" ]; then
  echo "No target given. Mount the SD card and pass its mount point, e.g.:"
  echo "    ./install.sh /media/\$USER/GB300 gb300v2"
  echo
  echo "Looking for a mounted card automatically..."
  for d in /media/*/* /media/* /Volumes/* /mnt/*; do
    [ -d "$d/ROMS" ] && [ -d "$d/system" ] && TARGET="$d" && break
  done
fi

[ -n "$TARGET" ] || die "could not find the SD card. Pass the mount point explicitly."
[ -d "$TARGET" ] || die "not a directory: $TARGET"

CORE="$HERE/payload/$VARIANT/sonicmania/core_87000000"
[ -f "$CORE" ] || die "no prebuilt core for variant '$VARIANT' in this package"

echo "Installing Sonic Mania ($VARIANT) into $TARGET"

# The loader looks for the core under several names/paths depending on the
# firmware build; write them all so any of them resolves.
TARGETS=(
  "system/Deimos/cores/sonicmania.sf2k"
  "system/Deimos/cores/sonicmania.gb3k"
  "cores/sonicmania.sf2k"
  "cores/sonicmania.gb3k"
  "ROMS/sonicmania/sonicmania.sf2k"
)

for rel in "${TARGETS[@]}"; do
  dst="$TARGET/$rel"
  mkdir -p "$(dirname "$dst")"
  cp -f "$CORE" "$dst"
  echo "  wrote $rel"
done

# Launcher stubs: the console's menu resolves the core and the data file from
# the contents of these small text files.
STUB="sonicmania;sonicmania;Data.rsdk"
mkdir -p "$TARGET/ROMS/sonicmania"
printf '%s' "$STUB" > "$TARGET/ROMS/sonicmania/sonicmania"
printf '%s' "$STUB" > "$TARGET/ROMS/sonicmania/sonicmania.gba"
echo "  wrote ROMS/sonicmania/sonicmania and sonicmania.gba (launcher stubs)"

# The game data is a proprietary asset and is not distributed with this
# package. The user has to supply their own copy of Data.rsdk.
if [ ! -f "$TARGET/ROMS/sonicmania/Data.rsdk" ]; then
  echo
  echo "NOTE: ROMS/sonicmania/Data.rsdk is missing."
  echo "      Copy your own Sonic Mania (Plus 1.06.0503) Data.rsdk there"
  echo "      before launching the game."
fi

# Recommended frontend options for this core (the core outputs 320x240 itself).
OPT="$TARGET/system/configs/multicore.opt"
if [ -f "$OPT" ]; then
  sed -i 's/^sf2000_scaling_mode = .*/sf2000_scaling_mode = "core-provided"/' "$OPT" || true
  sed -i 's/^sf2000_tearing_fix = .*/sf2000_tearing_fix = "fast"/' "$OPT" || true
fi

sync
echo
echo "Done. Safely eject the card, insert it into the console and launch"
echo "Sonic Mania from ROMS/sonicmania in the menu."
