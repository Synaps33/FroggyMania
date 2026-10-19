## FroggyMania v0.1.0 — Sonic Mania on SF2000 / GB300 (early preview)

First public snapshot of the Sonic Mania port for the SF2000/GB300 handheld.
The game boots, plays and is controllable entirely from the gamepad.

### What works

* Full boot sequence — developer logos, SEGA screen, the animated title screen
* Menu, Mania Mode save slot screen and gameplay (tested through Green Hill
  Zone and Angel Island Zone)
* Gamepad input, HUD, save data
* Rendering: the engine renders at 384x240 and is downscaled in the core to
  the console's native 320x240 panel

### Install

```sh
tar xzf froggymania-gb300v2-v0.1.0.tar.gz
cd froggymania-gb300v2-v0.1.0
./install.sh /media/$USER/GB300 gb300v2
```

Then copy your own `Data.rsdk` (Sonic Mania Plus 1.06.0503) to
`ROMS/sonicmania/Data.rsdk` on the card. The game data is not distributed.

### Known issues

* **Frame rate.** The MIPS CPU has no FPU and the engine is soft-float; busy
  scenes drop frames. Physics stays at 60 Hz, rendering skips frames.
* Early preview quality — occasional glitches and crashes are still being
  worked on.

### Requires

* GB300 V2 (only variant built so far)
* Sonic Mania Plus `Data.rsdk`, version 1.06.0503
