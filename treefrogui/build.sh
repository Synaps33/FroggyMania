#!/bin/sh
set -eu

TREEFROG_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE_DIR=$(dirname "$TREEFROG_DIR")
SOURCE_CORE="$SOURCE_DIR/source/sonicmania-sf2000"
OUTPUT=${1:-"$SOURCE_DIR/froggymania_libretro.so"}
RSDK_REV=191d7aea6c262d891244172a8609ac69fbfd7571
DEPS_REV=7e6e34f0319f4c7448d72f0e949e76265ccf55a1
RSDK_URL=https://github.com/RSDKModding/RSDKv5-Decompilation.git
DEPS_URL=https://github.com/libretro/libretro-deps.git

: "${CC:=mips-mti-linux-gnu-gcc}"
: "${CXX:=mips-mti-linux-gnu-g++}"
: "${AR:=mips-mti-linux-gnu-gcc-ar}"
JOBS=${JOBS:-2}

if [ ! -f "$SOURCE_CORE/Makefile" ]; then
    echo "FroggyMania source submodule is missing; clone with --recurse-submodules" >&2
    exit 1
fi
command -v "$CC" >/dev/null || { echo "cross compiler not found: $CC" >&2; exit 1; }
command -v "$CXX" >/dev/null || { echo "cross C++ compiler not found: $CXX" >&2; exit 1; }
command -v "$AR" >/dev/null || { echo "cross archiver not found: $AR" >&2; exit 1; }

BUILD_ROOT=$(mktemp -d "${TMPDIR:-/tmp}/froggymania-build.XXXXXX")
trap 'rm -rf "$BUILD_ROOT"' EXIT HUP INT TERM

fetch_revision() {
    repo_url=$1
    revision=$2
    repo_dir=$3
    local_source=$4
    if [ -n "$local_source" ]; then
        git clone --quiet --no-checkout "$local_source" "$repo_dir"
        git -C "$repo_dir" fetch --quiet --depth 1 origin "$revision"
    else
        git init --quiet "$repo_dir"
        git -C "$repo_dir" remote add origin "$repo_url"
        git -C "$repo_dir" fetch --quiet --depth 1 origin "$revision"
    fi
    git -C "$repo_dir" checkout --quiet --detach FETCH_HEAD
    actual=$(git -C "$repo_dir" rev-parse HEAD)
    [ "$actual" = "$revision" ] || {
        echo "wrong source revision: expected $revision, got $actual" >&2
        exit 1
    }
}

fetch_revision "$RSDK_URL" "$RSDK_REV" "$BUILD_ROOT/RSDKv5" "${RSDK_SOURCE_DIR:-}"
git -C "$BUILD_ROOT/RSDKv5" submodule update --init --recursive --depth 1 --quiet
fetch_revision "$DEPS_URL" "$DEPS_REV" "$BUILD_ROOT/libretro-deps" "${LIBRETRO_DEPS_SOURCE_DIR:-}"
git -C "$BUILD_ROOT/RSDKv5" apply "$TREEFROG_DIR/rsdk-libretro.patch"

BUILD_CORE="$BUILD_ROOT/sonicmania-sf2000"
cp -a "$SOURCE_CORE" "$BUILD_CORE"
mkdir -p "$BUILD_CORE/dependencies"
ln -s "$BUILD_ROOT/RSDKv5" "$BUILD_CORE/dependencies/RSDKv5"
git -C "$BUILD_CORE" apply "$TREEFROG_DIR/froggymania-libretro.patch"

# RSDK includes backend translation units relative to its own directories.
cp "$BUILD_CORE/src/LibretroRenderDevice.cpp" "$BUILD_CORE/src/LibretroRenderDevice.hpp" \
    "$BUILD_ROOT/RSDKv5/RSDKv5/RSDK/Graphics/"
cp "$BUILD_CORE/src/LibretroAudioDevice.cpp" "$BUILD_CORE/src/LibretroAudioDevice.hpp" \
    "$BUILD_ROOT/RSDKv5/RSDKv5/RSDK/Audio/"
cp "$BUILD_CORE/src/LibretroInputDevice.cpp" "$BUILD_CORE/src/LibretroInputDevice.hpp" \
    "$BUILD_ROOT/RSDKv5/RSDKv5/RSDK/Input/"

OGG_SRC="$BUILD_ROOT/libretro-deps/libogg/src"
THEORA_SRC="$BUILD_ROOT/libretro-deps/theora/lib"
CODEC_OBJECTS="$BUILD_ROOT/codecs"
mkdir -p "$CODEC_OBJECTS"
for source in "$OGG_SRC/bitwise.c" "$OGG_SRC/framing.c"; do
    "$CC" -EL -march=mips32r2 -mtune=74kc -mfp32 -mhard-float -fPIC -O2 \
        -I"$BUILD_ROOT/libretro-deps/libogg/include" -I"$OGG_SRC" \
        -c "$source" -o "$CODEC_OBJECTS/$(basename "$source" .c)_ogg.o"
done
for source in \
    "$THEORA_SRC/bitpack.c" "$THEORA_SRC/decinfo.c" "$THEORA_SRC/decode.c" \
    "$THEORA_SRC/dequant.c" "$THEORA_SRC/fragment.c" "$THEORA_SRC/huffdec.c" \
    "$THEORA_SRC/idct.c" "$THEORA_SRC/info.c" "$THEORA_SRC/internal.c" \
    "$THEORA_SRC/quant.c" "$THEORA_SRC/state.c"; do
    "$CC" -EL -march=mips32r2 -mtune=74kc -mfp32 -mhard-float -fPIC -O2 \
        -I"$BUILD_ROOT/libretro-deps/theora/include" \
        -I"$BUILD_ROOT/libretro-deps/libogg/include" -I"$THEORA_SRC" \
        -c "$source" -o "$CODEC_OBJECTS/$(basename "$source" .c)_theora.o"
done

COMMON_FLAGS="-EL -march=mips32r2 -mtune=74kc -mfp32 -mhard-float -G0 -fPIC -fno-strict-aliasing -O2 -fpermissive -I$BUILD_ROOT/libretro-deps/theora/include -I$BUILD_ROOT/libretro-deps/libogg/include"
make -j"$JOBS" -C "$BUILD_CORE" platform=sf2000 \
    CC="$CC" CXX="$CXX" AR="$AR" COMMON_FLAGS="$COMMON_FLAGS"

"$CXX" -EL -shared -Wl,--no-undefined \
    -o "$OUTPUT" \
    -Wl,--whole-archive "$BUILD_CORE/sonicmania_libretro_sf2000.a" \
    -Wl,--no-whole-archive "$CODEC_OBJECTS"/*.o -ldl -lm

echo "Built $OUTPUT"
