#!/bin/bash
set -e

echo "=== Czekam na wykrycie karty SD w czytniku... ==="
for i in $(seq 1 60); do
  if lsblk 2>/dev/null | grep -q mmcblk0p1; then
    echo "Karta wykryta po ${i}s!"
    break
  fi
  sleep 1
done

if ! lsblk 2>/dev/null | grep -q mmcblk0p1; then
  echo "BŁĄD: Karta nie została włożona w ciągu 60 sekund."
  exit 1
fi

sleep 1
echo "=== Montowanie karty... ==="
udevil mount /dev/mmcblk0p1 /media/Sajnaps/GB300 2>&1 || true

D="/media/Sajnaps/GB300"
if ! findmnt -T "$D" -o SOURCE --noheadings | grep -q mmc; then
  echo "BŁĄD: $D nie jest zamontowany z karty SD!"
  exit 1
fi

echo "=== Krok 1: Kopiowanie logu z konsoli na dysk lokalny ==="
if [ -f "$D/system/logs/Multicore.log" ]; then
  cp -vf "$D/system/logs/Multicore.log" /home/Sajnaps/Multicore_console.log
  echo "Log z konsoli zapisany do /home/Sajnaps/Multicore_console.log ($(stat -c %s /home/Sajnaps/Multicore_console.log) B)"
else
  echo "Brak pliku Multicore.log na karcie."
fi

echo "=== Krok 2: Wgrywanie nowego rdzenia na kartę SD ==="
SRC="/home/Sajnaps/gb300/sf2000_multicore/build/core_87000000"
REF=$(md5sum "$SRC" | cut -d' ' -f1)
echo "Wgrywam build: $(stat -c %s "$SRC") B  md5=$REF"

TARGETS=(
  "system/Deimos/cores/sonicmania.sf2k"
  "system/Deimos/cores/sonicmania.gb3k"
  "cores/sonicmania.sf2k"
  "cores/sonicmania.gb3k"
  "ROMS/sonicmania/sonicmania.sf2k"
)

ok=0
for rel in "${TARGETS[@]}"; do
  dst="$D/$rel"
  for attempt in 1 2 3 4 5; do
    dd if="$SRC" of="$dst" bs=1M conv=fsync status=none 2>/dev/null || { sleep 1; continue; }
    sync
    if [ "$(md5sum "$dst" 2>/dev/null | cut -d' ' -f1)" = "$REF" ]; then
      echo "OK    $rel"
      ok=$((ok+1))
      break
    fi
  done
done

echo "Zgodnych kopii: $ok/${#TARGETS[@]}"
if [ "$ok" -ne "${#TARGETS[@]}" ]; then
  echo "BŁĄD: Nie wszystkie kopie zostały prawidłowo zapisane!"
  exit 1
fi

echo "=== Krok 3: Czyszczenie logu na karcie i konfiguracja ==="
: > "$D/system/logs/Multicore.log" 2>/dev/null || true
sed -i 's/^pixWidth=.*/pixWidth=384/' "$D/ROMS/sonicmania/Settings.ini" 2>/dev/null || true
sed -i 's/^sf2000_scaling_mode = .*/sf2000_scaling_mode = "core-provided"/' "$D/system/configs/multicore.opt" 2>/dev/null || true
sed -i 's/^sf2000_tearing_fix = .*/sf2000_tearing_fix = "fast"/' "$D/system/configs/multicore.opt" 2>/dev/null || true

echo "=== Krok 4: Bezpieczna synchronizacja i odmontowanie ==="
sync
pumount /dev/mmcblk0p1 2>/dev/null || true
echo "=== GOTOWE: Karta bezpiecznie zsynchronizowana i odmontowana! ==="
echo "Możesz wyjąć kartę z laptopa i włożyć do konsoli."
