#ifndef LIBRETRO_SHARED_H
#define LIBRETRO_SHARED_H

#include "libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

extern retro_video_refresh_t s_video_cb;
extern retro_audio_sample_batch_t s_audio_batch_cb;
extern retro_environment_t s_environ_cb;
extern retro_input_poll_t s_input_poll_cb;
extern retro_input_state_t s_input_state_cb;
extern uint16_t s_input_state;

/* The engine renders 320x240 RGB565 and that is exactly what the frontend is
 * fed. Full-panel scaling is the OSD's job (sf2000_force_fullscreen in
 * video_sf2000.c); doing it here instead quadruples the bytes per frame and
 * previously cost ~10 fps. */
#define SONIC_SRC_W 384
#define SONIC_SRC_H 240
void sonic_present_frame(uint16_t *src, unsigned width, unsigned height, size_t pitch);

#ifdef __cplusplus
}
#endif

#endif
