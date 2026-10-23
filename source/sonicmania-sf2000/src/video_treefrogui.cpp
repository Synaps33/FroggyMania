#include "libretro_shared.h"

extern "C" void xlog(const char *fmt, ...);

void video_treefrogui_present(const void *src, unsigned width, unsigned height, size_t pitch)
{
    static uint16_t panel[640 * 480];
    static unsigned frames;

    if (!src || !s_video_cb)
        return;

    if (width == 640 && height == 480 && pitch == 640 * sizeof(uint16_t)) {
        s_video_cb(src, width, height, pitch);
        return;
    }

    const auto *pixels = static_cast<const uint16_t *>(src);
    const unsigned src_pitch = pitch / sizeof(uint16_t);
    for (unsigned y = 0; y < 480; ++y) {
        const uint16_t *row = pixels + (static_cast<size_t>(y) * height / 480) * src_pitch;
        uint16_t *out = panel + y * 640;
        for (unsigned x = 0; x < 640; ++x)
            out[x] = row[static_cast<size_t>(x) * width / 640];
    }

    if (frames < 8) {
        xlog("[TREEFROG_VIDEO] frame=%u source=%ux%u pitch=%zu output=640x480 pitch=1280\n",
             frames++, width, height, pitch);
    }
    s_video_cb(panel, 640, 480, 640 * sizeof(uint16_t));
}
