/* Standalone host harness for the Sonic Mania libretro core.
 *
 * The console build defers engine startup into retro_run() (see libretro.cpp),
 * so retro_load_game() returns immediately and nothing is initialised yet.
 * This runner therefore drives retro_run() and captures a series of frames
 * instead of inspecting state right after load.
 *
 * Data.rsdk is byte-identical between this host and the GB300 card, so scene
 * loading, object matching and rendering here reproduce the console's
 * behaviour -- which makes this the fast path for diagnosing issues like the
 * missing title-screen logo, without a card round-trip per attempt.
 *
 * Usage: test_runner [data.rsdk] [frames] [outdir]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libretro.h"
#include "RSDK/Core/RetroEngine.hpp"

#define FB_W 320
#define FB_H 240

static uint16_t s_frame_buffer[FB_W * FB_H];
static int s_frame_count = 0;
static int s_last_width = 0, s_last_height = 0;
static uint64_t s_audio_frames = 0;
static uint16_t s_simulated_input = 0;

static const char *s_outdir = "/tmp/opencode/shots";

static void video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch)
{
    if (s_frame_count % 60 == 0) {
        printf("[DEBUG SCREEN] f=%d: size=(%d,%d) pitch=%d pos=(%d,%d) center=(%d,%d) passed_w=%u passed_h=%u passed_pitch=%zu\n",
               s_frame_count, RSDK::screens[0].size.x, RSDK::screens[0].size.y, RSDK::screens[0].pitch,
               RSDK::screens[0].position.x, RSDK::screens[0].position.y,
               RSDK::screens[0].center.x, RSDK::screens[0].center.y,
               width, height, pitch);
    }
    if (!data) return;
    s_frame_count++;
    s_last_width = (int)width;
    s_last_height = (int)height;

    const uint8_t *src = (const uint8_t *)data;
    for (unsigned y = 0; y < height && y < FB_H; y++) {
        unsigned row = (width < FB_W ? width : FB_W);
        memcpy(&s_frame_buffer[y * FB_W], src + y * pitch, row * sizeof(uint16_t));
    }
}

static void audio_sample_cb(int16_t left, int16_t right) { (void)left; (void)right; }

static size_t audio_sample_batch_cb(const int16_t *data, size_t frames)
{
    if (!data) return 0;
    s_audio_frames += frames;
    return frames;
}

static void input_poll_cb(void) {}

static int16_t input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id)
{
    (void)port; (void)index;
    if (device != RETRO_DEVICE_JOYPAD) return 0;
    return (s_simulated_input & (1 << id)) ? 1 : 0;
}

static bool environ_cb(unsigned cmd, void *data)
{
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
        enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;
        return (*fmt == RETRO_PIXEL_FORMAT_RGB565);
    }
    return true;
}

static void save_ppm(const char *path, const uint16_t *buffer, int width, int height)
{
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "  ! cannot write %s\n", path); return; }
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int i = 0; i < width * height; i++) {
        uint16_t pixel = buffer[i];
        uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
        uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
        uint8_t b = (pixel & 0x1F) * 255 / 31;
        fputc(r, f); fputc(g, f); fputc(b, f);
    }
    fclose(f);
}

static int non_zero_pixels(void)
{
    int n = 0;
    for (int i = 0; i < FB_W * FB_H; i++)
        if (s_frame_buffer[i] != 0) n++;
    return n;
}

/* Count distinct colours: a flat/garbage frame has very few, a real scene many. */
static int distinct_colours(void)
{
    static uint8_t seen[65536];
    memset(seen, 0, sizeof(seen));
    int n = 0;
    for (int i = 0; i < FB_W * FB_H; i++) {
        uint16_t p = s_frame_buffer[i];
        if (!seen[p]) { seen[p] = 1; n++; }
    }
    return n;
}

static void capture(const char *label, int frame)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%03d_%s.ppm", s_outdir, frame, label);
    save_ppm(path, s_frame_buffer, FB_W, FB_H);
    printf("  [shot] frame=%3d %-22s fbref=%d video=%dx%d scene='%s' nonzero=%d colours=%d\n",
           frame, label, s_frame_count, s_last_width, s_last_height,
           (RSDK::sceneInfo.listData && RSDK::sceneInfo.listPos >= 0)
               ? RSDK::sceneInfo.listData[RSDK::sceneInfo.listPos].name : "?",
           non_zero_pixels(), distinct_colours());
}

int main(int argc, char **argv)
{
    const char *data_path = "/home/Sajnaps/sonicmania-portmaster/sonicmania/Data.rsdk";
    int total_frames = 2500;
    if (argc > 1) data_path = argv[1];
    if (argc > 2) total_frames = atoi(argv[2]);
    if (argc > 3) s_outdir = argv[3];
    int jump_scene = (argc > 4) ? atoi(argv[4]) : -1;

    mkdir(s_outdir, 0755);

    printf("=== Sonic Mania host harness ===\n");
    printf("data:    %s\n", data_path);
    printf("frames:  %d\n", total_frames);
    printf("outdir:  %s\n\n", s_outdir);

    FILE *tf = fopen(data_path, "rb");
    if (!tf) { fprintf(stderr, "ERROR: cannot open %s\n", data_path); return 1; }
    fclose(tf);

    retro_set_environment(environ_cb);
    retro_set_video_refresh(video_refresh_cb);
    retro_set_audio_sample(audio_sample_cb);
    retro_set_audio_sample_batch(audio_sample_batch_cb);
    retro_set_input_poll(input_poll_cb);
    retro_set_input_state(input_state_cb);

    retro_init();

    struct retro_game_info gi;
    memset(&gi, 0, sizeof(gi));
    gi.path = data_path;
    if (!retro_load_game(&gi)) { fprintf(stderr, "ERROR: retro_load_game failed\n"); return 2; }
    printf("retro_load_game ok (engine now starts inside retro_run)\n\n");

    /* Frame plan: sample the boot animation densely, then the loaded scene. */
    for (int f = 0; f < total_frames; f++) {
        retro_run();

        /* The engine starts on the first retro_run and loads its first scene
         * a few frames later; capture around those transitions. */
        if (f == 900 && jump_scene >= 0) {
            /* Jump straight to a scene by index so a level or menu can be
             * verified without driving the title screen by simulated input. */
            int c = 0, idx = jump_scene;
            for (c = 0; c < RSDK::sceneInfo.categoryCount; c++) {
                int start = RSDK::sceneInfo.listCategory[c].sceneOffsetStart;
                if (idx >= start && idx < start + RSDK::sceneInfo.listCategory[c].sceneCount) {
                    RSDK::sceneInfo.activeCategory = c;
                    break;
                }
            }
            printf("[jump] -> scene %d '%s' folder '%s' (category %d)\n", idx,
                   RSDK::sceneInfo.listData[idx].name,
                   RSDK::sceneInfo.listData[idx].folder, c);
            RSDK::sceneInfo.listPos = idx;
            RSDK::sceneInfo.state   = RSDK::ENGINESTATE_LOAD;
        }

        if (f == 900) {
            printf("\n--- scene list (categories=%d) ---\n", RSDK::sceneInfo.categoryCount);
            for (int c = 0; c < RSDK::sceneInfo.categoryCount; c++) {
                printf("category[%d] '%s' scenes=%d\n", c,
                       RSDK::sceneInfo.listCategory[c].name,
                       RSDK::sceneInfo.listCategory[c].sceneCount);
                for (int sc = 0; sc < RSDK::sceneInfo.listCategory[c].sceneCount; sc++) {
                    int idx = RSDK::sceneInfo.listCategory[c].sceneOffsetStart + sc;
                    printf("   scene[%2d] '%s' folder='%s' id='%s'\n", idx,
                           RSDK::sceneInfo.listData[idx].name,
                           RSDK::sceneInfo.listData[idx].folder,
                           RSDK::sceneInfo.listData[idx].id);
                }
            }
            printf("--- end scene list ---\n\n");
        }

        if (jump_scene < 0 && f >= 900 && f < 1700)
            s_simulated_input = ((f % 30) < 5) ? (1 << RETRO_DEVICE_ID_JOYPAD_START) : 0;

        switch (f) {
            /* Boot sequence. */
            case 2:   capture("f002_bootlog", f); break;
            case 80:  capture("f080_sega", f); break;
            case 180: capture("f180_sega", f); break;
            case 850: capture("f850_title_full", f); break;

            /* The title becomes interactive around frame 850 and falls back to
             * its attract video if nothing is pressed within 800 frames, so the
             * button has to come early -- pressing after ~1650 is too late and
             * only ever exercises the video path. */
            /* keyStart.press is an edge, so a single tap can land while the
             * title is still in an animation state and be missed entirely.
             * Tap repeatedly across the whole interactive window instead. */
            case 950:
                if (jump_scene < 0 && (f % 30) < 5) s_simulated_input = (1 << RETRO_DEVICE_ID_JOYPAD_START);
                else if (jump_scene < 0) s_simulated_input = 0;
                break;
            case 1200: capture("f1200_after_start", f); break;
            case 1500: capture("f1500_after_start", f); break;
            case 1900: capture("f1900_after_start", f); break;
            case 2499: capture("f2499_after_start", f); break;

            /* Jump mode: skip the level's title card, then run right. */
            case 1000: if (jump_scene >= 0) capture("f1000_after_jump", f); break;
            case 1050:
                if (jump_scene >= 0) s_simulated_input = (1 << RETRO_DEVICE_ID_JOYPAD_START);
                break;
            case 1065:
                s_simulated_input = 0;
                break;
            case 1600:
                if (jump_scene >= 0) s_simulated_input = (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
                break;
            case 2000: if (jump_scene >= 0) capture("f2000_level", f); break;
            default: break;
        }

        /* Reproduce the console: the hackerlog is dismissed after ~45 frames by
         * libretro.cpp, so nothing to do here. */
    }

    printf("\n--- summary ---\n");
    printf("video frames delivered: %d\n", s_frame_count);
    printf("last reported geometry: %dx%d\n", s_last_width, s_last_height);
    printf("audio frames: %llu\n", (unsigned long long)s_audio_frames);
    printf("final non-zero pixels:  %d / %d\n", non_zero_pixels(), FB_W * FB_H);
    printf("final distinct colours: %d\n", distinct_colours());

    retro_deinit();
    printf("deinit ok\n");
    return 0;
}
