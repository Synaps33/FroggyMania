## FroggyMania v0.1.1 — Sonic Mania on SF2000 / GB300

Maintenance release: the core is rebuilt on the latest frontend sources.
Gameplay and rendering are unchanged from v0.1.0.

### Fixed in the frontend (included in this core)

* **Directory listing on FatFS.** The end of a directory is reported as a
  successful read with an empty name, not as an error. The listing helper now
  terminates on it instead of looping forever, which could make the core look
  hung when it enumerated `ROMS/` or a save directory.
* **Save-RAM access on cores without batteries.** Loading/saving the `.srm`
  is now skipped when the core exposes no save RAM or RTC (both sizes 0). This
  removes a pointless directory create + SD-card sync from the boot path.

### Install

```sh
tar xzf froggymania-gb300v2-v0.1.1.tar.gz
cd froggymania-gb300v2-v0.1.1
./install.sh /media/$USER/GB300 gb300v2
```

Then copy your own `Data.rsdk` (Sonic Mania Plus 1.06.0503) to
`ROMS/sonicmania/Data.rsdk` on the card. The game data is not distributed.

### Requires

* GB300 V2 (only variant built so far)
* Sonic Mania Plus `Data.rsdk`, version 1.06.0503
