FroggyMania - Sonic Mania for SF2000 / GB300
============================================

Sonic Mania (the official RSDKv5 decompilation) running on the Data Frog
GB300 / SF2000 handheld, driven entirely from the gamepad.

The game data (Data.rsdk) is NOT included - see REQUIREMENTS below.


INSTALL
-------
1. Insert the SD card into your computer and note the mount point
   (for example /media/$USER/GB300 or /Volumes/GB300 on macOS).
2. Run:

       ./install.sh <mount point> [gb300v2|sf2000]

   The variant defaults to gb300v2. The installer is idempotent; re-run it
   to update an existing install.
3. Safely eject the card, put it in the console, and start Sonic Mania from
   ROMS/sonicmania in FrogUI.


REQUIREMENTS
------------
* A copy of Sonic Mania / Sonic Mania Plus (the PC release) and its Data.rsdk,
  copied to:

      ROMS/sonicmania/Data.rsdk

  The installer never touches this file. Without it the core will abort at
  boot (the hackerlog shows "COPY Data.rsdk -> ROMS/sonicmania/").
  The tested data version is 1.06.0503 (Sonic Mania Plus).


WHAT IS INSTALLED
-----------------
    system/Deimos/cores/sonicmania.sf2k
    system/Deimos/cores/sonicmania.gb3k
    cores/sonicmania.sf2k
    cores/sonicmania.gb3k
    ROMS/sonicmania/sonicmania.sf2k     the libretro core (same file 5x,
                                        covering all known loader paths)
    ROMS/sonicmania/sonicmania          launcher stubs
    ROMS/sonicmania/sonicmania.gba

No firmware modification is required.


CONTROLS
--------
  D-pad                move / navigate menus
  A / B                jump (accept / back in menus)
  X / Y                extra action buttons
  START                pause / confirm
  SELECT               select
  L / R                shoulder buttons


KNOWN ISSUES
------------
* Early preview - expect rough edges and occasional glitches.
* The port is CPU-bound (the MIPS CPU has no FPU and everything is
  software-emulated). On busy scenes the frame rate drops; physics stays at
  60 Hz while rendering may skip frames.
* Tested on GB300 V2. A plain SF2000 build is not included in this package.

Built from source revision 2026-10-24. See the repository README for build
instructions.
