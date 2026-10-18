#!/bin/bash
# Deploy the Sonic Mania core to the GB300 SD card.
#
# /media/Sajnaps/GB300 also exists as a plain directory on the laptop's root
# filesystem (ext4). When the card is removed, that directory stays behind and
# silently swallows writes, so a "successful" copy can land on the laptop disk
# instead of the card. Guard against it: refuse to write unless the target is a
# vfat filesystem mounted from a real mmc device.
set -u

CARD="/media/Sajnaps/GB300"
SRC="/home/Sajnaps/gb300/sf2000_multicore/build/core_87000000"

TARGETS=(
  "system/Deimos/cores/sonicmania.sf2k"
  "system/Deimos/cores/sonicmania.gb3k"
  "cores/sonicmania.sf2k"
  "cores/sonicmania.gb3k"
  "ROMS/sonicmania/sonicmania.sf2k"
)

if [ ! -f "$SRC" ]; then
  echo "brak zbudowanego rdzenia: $SRC"
  exit 1
fi

if ! findmnt -T "$CARD" -o SOURCE,FSTYPE --noheadings >/dev/null 2>&1; then
  echo "ODMOWA: $CARD nie jest punktem montowania."
  echo "        (to lokalny katalog na dysku laptopa)"
  exit 1
fi

read -r SRC_DEV FSTYPE <<<"$(findmnt -T "$CARD" -o SOURCE,FSTYPE --noheadings)"
case "$SRC_DEV" in
  /dev/mmcblk*|/dev/sd[a-z][0-9]*) ;;
  *) echo "ODMOWA: $CARD jest zamontowany z $SRC_DEV, nie z karty SD."; exit 1 ;;
esac
if [ "$FSTYPE" != "vfat" ]; then
  echo "ODMOWA: $CARD ma system plikow $FSTYPE, oczekiwano vfat (karta konsoli)."
  exit 1
fi

REF=$(md5sum "$SRC" | cut -d' ' -f1)
SIZE=$(stat -c %s "$SRC")
echo "card:  $SRC_DEV ($FSTYPE)"
echo "build: $SIZE B  md5=$REF"
echo

ok=0
fail=0
for rel in "${TARGETS[@]}"; do
  dst="$CARD/$rel"
  done_ok=0
  for attempt in 1 2 3 4 5 6; do
    dd if="$SRC" of="$dst" bs=1M conv=fsync status=none 2>/dev/null || { sleep 2; continue; }
    sync; sleep 1
    if [ "$(md5sum "$dst" 2>/dev/null | cut -d' ' -f1)" = "$REF" ]; then
      echo "OK    $rel"
      done_ok=1; ok=$((ok+1)); break
    fi
  done
  [ $done_ok = 0 ] && { echo "BŁĄD  $rel"; fail=$((fail+1)); }
done

sync
echo
echo "zgodnych: $ok/${#TARGETS[@]}  (ref=$REF)"
[ $fail = 0 ] || exit 1
