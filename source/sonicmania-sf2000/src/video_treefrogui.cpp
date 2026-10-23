#include "libretro_shared.h"
#include <time.h>

static uint64_t video_time_us()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

extern "C" void xlog(const char *fmt, ...);

static bool audio_active, underrun_likely, skipped_last_frame;
static unsigned audio_occupancy;
static unsigned frameskip_phase;
static int frameskip_mode;

void video_treefrogui_audio_status(bool active, unsigned occupancy, bool underrun)
{
    audio_active = active;
    audio_occupancy = occupancy;
    underrun_likely = underrun;
    if (!active || !underrun)
        skipped_last_frame = false;
}

void video_treefrogui_set_frameskip(int mode)
{
    if (mode < 0 || mode > 3)
        mode = 0;
    if (mode != frameskip_mode) {
        frameskip_mode = mode;
        frameskip_phase = 0;
        skipped_last_frame = false;
    }
}

void video_treefrogui_present(const void *src, unsigned width, unsigned height, size_t pitch)
{
    static unsigned frames, samples, presented, skipped;
    static uint64_t callback_us, interval_start;

    if (!src || !s_video_cb)
        return;

    if (frames < 8) {
        xlog("[TREEFROG_VIDEO] frame=%u source=%ux%u pitch=%zu output=native\n",
             frames, width, height, pitch);
    }
    bool skip = false;
    if (frameskip_mode == 0)
        skip = audio_active && underrun_likely && !skipped_last_frame;
    else if (frameskip_mode >= 2)
        skip = frameskip_phase++ % (frameskip_mode == 2 ? 2 : 3) < frameskip_mode - 1;
    ++frames;
    const uint64_t started = video_time_us();
    if (!samples) interval_start = started;
    s_video_cb(skip ? NULL : src, width, height, pitch);
    const uint64_t finished = video_time_us();
    callback_us += finished - started;
    if (skip) {
        ++skipped;
        skipped_last_frame = true;
    } else {
        ++presented;
        skipped_last_frame = false;
    }
    if (++samples == 300) {
        xlog("[PERF] frames=%u presented=%u skipped=%u elapsed_ms=%llu callback_avg_us=%llu frameskip=%d audio=%d occupancy=%u underrun=%d source=%ux%u\n",
             samples, presented, skipped,
             (unsigned long long)((finished - interval_start) / 1000),
             (unsigned long long)(callback_us / samples),
             frameskip_mode, audio_active, audio_occupancy, underrun_likely, width, height);
        samples = 0;
        presented = skipped = 0;
        callback_us = 0;
    }
}
