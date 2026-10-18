TARGET_NAME := sonicmania

ifeq ($(platform), sf2000)
    TARGET := $(TARGET_NAME)_libretro_$(platform).a
    CC = /opt/frog-toolchain/mipsel-mti-elf_stable/bin/mipsel-mti-elf-gcc
    CXX = /opt/frog-toolchain/mipsel-mti-elf_stable/bin/mipsel-mti-elf-g++
    AR = /opt/frog-toolchain/mipsel-mti-elf_stable/bin/mipsel-mti-elf-gcc-ar
    COMMON_FLAGS := -EL -march=mips32 -mtune=mips32r2 -msoft-float -O3 -fomit-frame-pointer -ffast-math -finline-functions -G0 -mno-abicalls -fno-pic \
                    -ffunction-sections -fdata-sections -DSF2000 -DGB300
else
    TARGET := test_runner
    CC := /usr/bin/gcc -m32
    CXX := /usr/bin/g++ -m32
    AR := ar
    COMMON_FLAGS := -O1 -g
endif

INCLUDES := -Iinclude \
            -Isrc \
            -ISonicMania \
            -ISonicMania/Objects \
            -Idependencies/RSDKv5/RSDKv5 \
            -Idependencies/RSDKv5/dependencies/all \
            -Idependencies/RSDKv5/dependencies/all/iniparser \
            -Idependencies/RSDKv5/dependencies/all/tinyxml2 \
            -Idependencies/RSDKv5/dependencies/all/stb_vorbis

DEFINES := -DRSDK_USE_LIBRETRO=1 \
           -DRETRO_STANDALONE=0 \
           -DGAME_STATIC=1 \
           -DRETRO_REVISION=2 \
           -DRETRO_DISABLE_PLUS=0 \
           -DRETRO_USE_MOD_LOADER=0 \
           -DRETRO_NO_VIDEO=1

# The bundled Data.rsdk is ambiguous: it ships Data/Sprites/Title/PlusLogo.bin
# (a Plus asset) yet its Title scene references the base-game Options/SaveGame
# objects, which a Plus build (GAME_VERSION >= VER_105) does not register --
# they end up as UNMATCHED and the title screen loses its logo and background.
#
# GAME_VERSION alone cannot be lowered: LinkGameLogicDLL's ABI follows
# MANIA_USE_PLUS while the engine calls it per RETRO_REV02, so the two must
# agree. MANIA_PREPLUS fixes that pair properly (Game.h then sets
# GAME_VERSION=VER_103 and RETRO_REVISION=1).
MANIA_PREPLUS ?= 0
ifeq ($(MANIA_PREPLUS),1)
DEFINES += -DMANIA_PREPLUS=1
endif

CFLAGS := $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) -std=c99
CXXFLAGS := $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) -std=c++14 -fno-exceptions -fno-rtti

# C Sources
C_SRCS := SonicMania/Game.c \
          SonicMania/Objects/All.c \
          src/xlog_host.c \
          dependencies/RSDKv5/dependencies/all/miniz/miniz.c

# C++ Sources
CXX_SRCS := src/libretro.cpp \
            src/sonic_hackerlog.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Core/RetroEngine.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Core/Math.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Core/Reader.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Core/Link.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Core/ModAPI.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Dev/Debug.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Storage/Storage.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Storage/Text.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Drawing.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Scene3D.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Animation.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Sprite.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Palette.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Video.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Audio/Audio.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Input/Input.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Scene/Scene.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Scene/Collision.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Scene/Object.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Scene/Objects/DefaultObject.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/Scene/Objects/DevOutput.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserAchievements.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserCore.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserLeaderboards.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserPresence.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserStats.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserStorage.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyAchievements.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyCore.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyLeaderboards.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyPresence.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyStats.cpp \
            dependencies/RSDKv5/RSDKv5/RSDK/User/Dummy/DummyStorage.cpp \
            dependencies/RSDKv5/dependencies/all/iniparser/dictionary.cpp \
            dependencies/RSDKv5/dependencies/all/iniparser/iniparser.cpp \
            dependencies/RSDKv5/dependencies/all/tinyxml2/tinyxml2.cpp

OBJS_DIR := build_$(if $(platform),$(platform),host)
C_OBJS := $(patsubst %.c,$(OBJS_DIR)/%.o,$(C_SRCS))
CXX_OBJS := $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(CXX_SRCS))
ALL_OBJS := $(C_OBJS) $(CXX_OBJS)

ifeq ($(platform), sf2000)
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	@echo "Creating static library $(TARGET)..."
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $(ALL_OBJS)

else
all: $(TARGET)

$(TARGET): $(ALL_OBJS) $(OBJS_DIR)/runner/test_runner.o
	@echo "Linking test runner $@..."
	$(CXX) $(COMMON_FLAGS) $^ -lm -lpthread -o $@

$(OBJS_DIR)/runner/test_runner.o: runner/test_runner.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

endif

$(OBJS_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# The libretro backends are #included by the RSDKv5 platform translation units,
# so edits to them must invalidate those objects too.
DRAWING_OBJ := $(OBJS_DIR)/dependencies/RSDKv5/RSDKv5/RSDK/Graphics/Drawing.o
AUDIO_OBJ   := $(OBJS_DIR)/dependencies/RSDKv5/RSDKv5/RSDK/Audio/Audio.o
INPUT_OBJ   := $(OBJS_DIR)/dependencies/RSDKv5/RSDKv5/RSDK/Input/Input.o
STORAGE_OBJ := $(OBJS_DIR)/dependencies/RSDKv5/RSDKv5/RSDK/User/Core/UserStorage.o

$(DRAWING_OBJ): src/LibretroRenderDevice.cpp src/LibretroRenderDevice.hpp
$(AUDIO_OBJ):   src/LibretroAudioDevice.cpp src/LibretroAudioDevice.hpp
$(INPUT_OBJ):   src/LibretroInputDevice.cpp src/LibretroInputDevice.hpp
$(STORAGE_OBJ): src/sonic_hackerlog.cpp

clean:
	rm -rf build_host build_sf2000 test_runner *.a

.PHONY: all clean
