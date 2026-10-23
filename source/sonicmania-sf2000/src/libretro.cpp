#include "libretro.h"
#include "libretro_shared.h"
#include "sonic_hackerlog.h"
#include "RSDK/Core/RetroEngine.hpp"

#define EngineInfo RSDK::EngineInfo
#include <GameMain.h>
#define LinkGameLogic LinkGameLogicDLL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

retro_video_refresh_t s_video_cb = NULL;
retro_audio_sample_batch_t s_audio_batch_cb = NULL;
retro_environment_t s_environ_cb = NULL;
retro_input_poll_t s_input_poll_cb = NULL;
retro_input_state_t s_input_state_cb = NULL;
uint16_t s_input_state = 0;

static float s_mix_buffer[735 * 2];
static int16_t s_audio_buffer[735 * 2];

void retro_set_environment(retro_environment_t cb)
{
    s_environ_cb = cb;
#if !defined(SF2000) && !defined(GB300) && !defined(GB300V2)
    xlog_set_environment(cb);
#endif
    if (cb) {
        enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
        cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
    }
}

void retro_set_video_refresh(retro_video_refresh_t cb) { s_video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { (void)cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { s_audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { s_input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { s_input_state_cb = cb; }

void retro_init(void)
{
    RSDK::linkGameLogic = (RSDK::LogicLinkHandle)LinkGameLogic;
    RSDK::InitCoreAPI();
}

void retro_deinit(void)
{
    RSDK::DeinitRetroEngine();
    RSDK::ReleaseCoreAPI();
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "sonicmania";
    info->library_version  = "1.0.0";
    info->valid_extensions = "rsdk|gba";
    info->need_fullpath    = true;
    info->block_extract    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    info->timing.fps            = 60.0;
    info->timing.sample_rate    = 44100.0;
    info->geometry.base_width   = 640;
    info->geometry.base_height  = 480;
    info->geometry.max_width    = 640;
    info->geometry.max_height   = 480;
    info->geometry.aspect_ratio = 4.0f / 3.0f;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;
}

void retro_reset(void)
{
}

static int s_boot_frame = 0;
static uint16_t s_prev_input = 0;

/* Dedicated fallback boot framebuffer so HackerLog can render even before screens[0] is set up */
static uint16_t s_hacker_boot_fb[320 * 240];

enum CoreInitState {
    CORE_INIT_START = 0,
    CORE_INIT_SHOW_LOG,
    CORE_INIT_RUN_ENGINE_INIT,
    CORE_INIT_DONE
};

static int s_core_init_state = CORE_INIT_START;
static bool s_engine_init_ok = false;

extern "C" void xlog(const char *fmt, ...);

void sonic_present_frame(uint16_t *src, unsigned width, unsigned height, size_t pitch)
{
    video_treefrogui_present(src, width, height, pitch);
}

void retro_run(void)
{
    if (s_input_poll_cb)
        s_input_poll_cb();

    uint16_t just_pressed = s_input_state & ~s_prev_input;
    s_prev_input = s_input_state;

    // SELECT button toggles telemetry overlay
    if (just_pressed & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT)) {
        sonic_hackerlog_toggle();
    }

    uint16_t *boot_fb = RSDK::screens[0].frameBuffer ? RSDK::screens[0].frameBuffer : s_hacker_boot_fb;

    // State machine for stepwise startup
    switch (s_core_init_state) {
        case CORE_INIT_START:
            xlog("[LIBRETRO] retro_run: CORE_INIT_START (frame %d)\n", s_boot_frame);
            s_core_init_state = CORE_INIT_SHOW_LOG;
            break;

        case CORE_INIT_SHOW_LOG:
            // Allow the console to display the initial telemetry for several frames
            if (s_boot_frame >= 6) {
                s_core_init_state = CORE_INIT_RUN_ENGINE_INIT;
            }
            break;

        case CORE_INIT_RUN_ENGINE_INIT:
            xlog("[LIBRETRO] retro_run: CORE_INIT_RUN_ENGINE_INIT\n");
            sonic_hackerlog_add("ENG", "InitRetroEngine running...");
            sonic_hackerlog_progress(40);
            if (boot_fb && s_video_cb) {
                sonic_hackerlog_render_boot(boot_fb, s_boot_frame);
                sonic_present_frame(boot_fb, 320, 240, 320 * sizeof(uint16_t));
            }

            {
                char *argv[] = { (char *)"RSDKv5", NULL };
                s_engine_init_ok = RSDK::InitRetroEngine(1, argv);
            }

            if (s_engine_init_ok) {
                xlog("[LIBRETRO] InitRetroEngine SUCCEEDED!\n");
                if (RSDK::useDataPack) {
                    sonic_hackerlog_add("VFS", "RSDK DATA PACK MOUNTED");
                    sonic_hackerlog_progress(80);
                } else {
                    sonic_hackerlog_add("ERR", "RSDK DATA PACK NOT MOUNTED");
                }
                if (RSDK::sceneInfo.listData) {
                    sonic_hackerlog_add("CFG", "GAMECONFIG.BIN PARSED");
                    sonic_hackerlog_progress(95);
                } else {
                    sonic_hackerlog_add("ERR", "GAMECONFIG.BIN MISSING");
                }
                sonic_hackerlog_add("ENG", "ENGINE READY -> STARTING GAME");
                sonic_hackerlog_progress(100);
            } else {
                xlog("[LIBRETRO] InitRetroEngine FAILED!\n");
                sonic_hackerlog_add("ERR", "InitRetroEngine FAILED!");
            }
            s_core_init_state = CORE_INIT_DONE;
            break;

        case CORE_INIT_DONE:
        default:
            break;
    }

    if (sonic_hackerlog_is_boot_active()) {
        s_boot_frame++;

        // Quick skip or auto-dismiss after hold time
        if ((just_pressed & ((1 << RETRO_DEVICE_ID_JOYPAD_START) | (1 << RETRO_DEVICE_ID_JOYPAD_A))) && s_boot_frame > 10) {
            sonic_hackerlog_dismiss_boot();
        }
        else if (s_core_init_state == CORE_INIT_DONE && s_engine_init_ok && s_boot_frame > 45) {
            sonic_hackerlog_dismiss_boot();
        }

        if (sonic_hackerlog_is_boot_active() && boot_fb) {
            sonic_hackerlog_render_boot(boot_fb, s_boot_frame);
            sonic_present_frame(boot_fb, 320, 240, 320 * sizeof(uint16_t));
        }
    }

    // Step engine and render game frames
    if (s_engine_init_ok && RSDK::RenderDevice::isRunning) {
        RSDK::StepRetroEngine();

        // Audio output
        RSDK::AudioDeviceBase::ProcessAudioMixing(s_mix_buffer, 735 * 2);
        // Audio output (branchless clamp and convert)
        for (int i = 0; i < 735 * 2; i += 2) {
            float s0 = s_mix_buffer[i + 0];
            float s1 = s_mix_buffer[i + 1];
            s_audio_buffer[i + 0] = (int16_t)(s0 >= 1.0f ? 32767 : (s0 <= -1.0f ? -32767 : (int32_t)(s0 * 32767.0f)));
            s_audio_buffer[i + 1] = (int16_t)(s1 >= 1.0f ? 32767 : (s1 <= -1.0f ? -32767 : (int32_t)(s1 * 32767.0f)));
        }
        if (s_audio_batch_cb) {
            s_audio_batch_cb(s_audio_buffer, 735);
        }
    }
}

static bool check_file_readable(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

static void resolve_rsdk_paths(const char *raw_path)
{
    char clean_path[256] = { 0 };
    if (raw_path && raw_path[0]) {
        strncpy(clean_path, raw_path, sizeof(clean_path) - 1);
        size_t l = strlen(clean_path);
        while (l > 0 && (clean_path[l - 1] == '\r' || clean_path[l - 1] == '\n' || clean_path[l - 1] == ' ' || clean_path[l - 1] == '\t')) {
            clean_path[--l] = '\0';
        }
    }
    const char *game_path = clean_path[0] ? clean_path : NULL;

    sonic_hackerlog_init(game_path);

    char candidate[256];
    char base_dir[256] = { 0 };

    if (game_path && game_path[0]) {
        sonic_hackerlog_add("ROM", game_path);
        const char *last_slash = strrchr(game_path, '/');
        if (last_slash) {
            size_t dirlen = last_slash - game_path + 1;
            if (dirlen < sizeof(base_dir)) {
                strncpy(base_dir, game_path, dirlen);
                base_dir[dirlen] = '\0';
            }
        }

        // Check if launched via .gba shortcut file
        size_t len = strlen(game_path);
        if (len > 4) {
            const char *ext = game_path + len - 4;
            if (ext[0] == '.' &&
                (ext[1] == 'g' || ext[1] == 'G') &&
                (ext[2] == 'b' || ext[2] == 'B') &&
                (ext[3] == 'a' || ext[3] == 'A')) {
                FILE *f = fopen(game_path, "r");
                if (f) {
                    char line[256];
                    if (fgets(line, sizeof(line), f)) {
                        char *core_name = strtok(line, ";\r\n");
                        char *subfolder = strtok(NULL, ";\r\n");
                        (void)core_name;
                        if (subfolder && subfolder[0]) {
                            snprintf(candidate, sizeof(candidate), "%s%s/", base_dir, subfolder);
                            char test_file[256];
                            snprintf(test_file, sizeof(test_file), "%sData.rsdk", candidate);
                            if (check_file_readable(test_file)) {
                                strncpy(RSDK::SKU::userFileDir, candidate, sizeof(RSDK::SKU::userFileDir) - 1);
                                sonic_hackerlog_load(candidate);
                                sonic_hackerlog_add("GBA", "STUB OK (3-FIELD)");
                                sonic_hackerlog_set_main_class("RSDKv5 LINK / MANIA GAME LOGIC");
                                fclose(f);
                                return;
                            }
                        }
                    }
                    fclose(f);
                }
            }
        }
    }

    if (base_dir[0]) {
        snprintf(candidate, sizeof(candidate), "%sData.rsdk", base_dir);
        if (check_file_readable(candidate)) {
            strncpy(RSDK::SKU::userFileDir, base_dir, sizeof(RSDK::SKU::userFileDir) - 1);
            sonic_hackerlog_load(base_dir);
            sonic_hackerlog_add("VFS", "DATAPACK IN ROM DIR");
            sonic_hackerlog_set_main_class("RSDKv5 LINK / MANIA GAME LOGIC");
            return;
        }

        snprintf(candidate, sizeof(candidate), "%ssonicmania/Data.rsdk", base_dir);
        if (check_file_readable(candidate)) {
            snprintf(candidate, sizeof(candidate), "%ssonicmania/", base_dir);
            strncpy(RSDK::SKU::userFileDir, candidate, sizeof(RSDK::SKU::userFileDir) - 1);
            sonic_hackerlog_load(candidate);
            sonic_hackerlog_add("VFS", "DATAPACK IN SUBDIR");
            sonic_hackerlog_set_main_class("RSDKv5 LINK / MANIA GAME LOGIC");
            return;
        }
    }

    static const char *fallback_dirs[] = {
        "/mnt/sda1/ROMS/sonicmania/",
        "/mnt/sda1/ROMS/mania/",
        "/mnt/sda1/ROMS/",
        "/mnt/sda1/",
        "./sonicmania/",
        "./"
    };

    for (size_t i = 0; i < sizeof(fallback_dirs) / sizeof(fallback_dirs[0]); i++) {
        snprintf(candidate, sizeof(candidate), "%sData.rsdk", fallback_dirs[i]);
        if (check_file_readable(candidate)) {
            strncpy(RSDK::SKU::userFileDir, fallback_dirs[i], sizeof(RSDK::SKU::userFileDir) - 1);
            sonic_hackerlog_load(fallback_dirs[i]);
            sonic_hackerlog_add("VFS", "DATAPACK VIA FALLBACK");
            sonic_hackerlog_set_main_class("RSDKv5 LINK / MANIA GAME LOGIC");
            return;
        }
    }

    sonic_hackerlog_add("ERR", "DATAPACK NOT FOUND ON SD");
    sonic_hackerlog_add("ERR", "COPY Data.rsdk -> ROMS/sonicmania/");
}

bool retro_load_game(const struct retro_game_info *game)
{
    xlog("[LIBRETRO] retro_load_game enter (game=%s)\n", game ? game->path : "NULL");
    resolve_rsdk_paths(game ? game->path : NULL);
    xlog("[LIBRETRO] resolve_rsdk_paths finished, returning true immediately for non-blocking start\n");
    return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    (void)game_type; (void)info; (void)num_info;
    return false;
}

void retro_unload_game(void)
{
    RSDK::DeinitRetroEngine();
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void *data, size_t size) { (void)data; (void)size; return false; }
bool retro_unserialize(const void *data, size_t size) { (void)data; (void)size; return false; }
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) { (void)index; (void)enabled; (void)code; }
