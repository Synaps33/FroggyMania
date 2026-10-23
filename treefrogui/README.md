# TreeFrogUI build

This directory adapts the upstream SF2000/GB300 source to TreeFrogUI's
MIPS32r2 hard-float Libretro target. It supplies the RSDK lifecycle and device
backends, then links Ogg and Theora statically.

Build from any directory:

```sh
./treefrogui/build.sh /path/to/froggymania_libretro.so
```

The script pins RSDKv5 and libretro-deps revisions. Set `CC`, `CXX`, and `AR`
to override its default MIPS cross tools. It downloads dependencies into a
temporary directory and removes that directory on exit.

The framebuffer backend cannot render RSDK's YUV video textures, so it skips
video cutscenes. Game data remains user supplied. TreeFrogUI device runtime
must be verified on hardware.
