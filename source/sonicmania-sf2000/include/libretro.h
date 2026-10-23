#ifndef LIBRETRO_H
#define LIBRETRO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RETRO_API_VERSION                 1

#define RETRO_DEVICE_TYPE_SHIFT           8
#define RETRO_DEVICE_MASK                 ((1 << RETRO_DEVICE_TYPE_SHIFT) - 1)
#define RETRO_DEVICE_SUBCLASS(base, id)   (((id + 1) << RETRO_DEVICE_TYPE_SHIFT) | base)

#define RETRO_DEVICE_NONE                 0
#define RETRO_DEVICE_JOYPAD               1

#define RETRO_DEVICE_ID_JOYPAD_B          0
#define RETRO_DEVICE_ID_JOYPAD_Y          1
#define RETRO_DEVICE_ID_JOYPAD_SELECT     2
#define RETRO_DEVICE_ID_JOYPAD_START      3
#define RETRO_DEVICE_ID_JOYPAD_UP         4
#define RETRO_DEVICE_ID_JOYPAD_DOWN       5
#define RETRO_DEVICE_ID_JOYPAD_LEFT       6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT      7
#define RETRO_DEVICE_ID_JOYPAD_A          8
#define RETRO_DEVICE_ID_JOYPAD_X          9
#define RETRO_DEVICE_ID_JOYPAD_L          10
#define RETRO_DEVICE_ID_JOYPAD_R          11

#define RETRO_REGION_NTSC                 0
#define RETRO_REGION_PAL                  1

#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10
#define RETRO_ENVIRONMENT_GET_VARIABLE 15
#define RETRO_ENVIRONMENT_SET_VARIABLES 16
#define RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE 17
#define RETRO_ENVIRONMENT_GET_LOG_INTERFACE 27
#define RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK 62

enum retro_log_level { RETRO_LOG_DEBUG, RETRO_LOG_INFO, RETRO_LOG_WARN, RETRO_LOG_ERROR };
typedef void (*retro_log_printf_t)(enum retro_log_level level, const char *fmt, ...);
struct retro_log_callback { retro_log_printf_t log; };

enum retro_pixel_format
{
   RETRO_PIXEL_FORMAT_0RGB1555 = 0,
   RETRO_PIXEL_FORMAT_XRGB8888 = 1,
   RETRO_PIXEL_FORMAT_RGB565   = 2,
   RETRO_PIXEL_FORMAT_UNKNOWN  = 0xffff
};

struct retro_game_info
{
   const char *path;
   const void *data;
   size_t      size;
   const char *meta;
};

struct retro_system_info
{
   const char *library_name;
   const char *library_version;
   const char *valid_extensions;
   bool        need_fullpath;
   bool        block_extract;
};

struct retro_game_geometry
{
   unsigned base_width;
   unsigned base_height;
   unsigned max_width;
   unsigned max_height;
   float    aspect_ratio;
};

struct retro_system_timing
{
   double fps;
   double sample_rate;
};

struct retro_system_av_info
{
   struct retro_game_geometry geometry;
   struct retro_system_timing timing;
};

typedef bool (*retro_environment_t)(unsigned cmd, void *data);
struct retro_variable { const char *key; const char *value; };
typedef void (*retro_audio_buffer_status_callback_t)(bool active, unsigned occupancy, bool underrun_likely);
struct retro_audio_buffer_status_callback { retro_audio_buffer_status_callback_t callback; };
typedef void (*retro_video_refresh_t)(const void *data, unsigned width, unsigned height, size_t pitch);
typedef void (*retro_audio_sample_t)(int16_t left, int16_t right);
typedef size_t (*retro_audio_sample_batch_t)(const int16_t *data, size_t frames);
typedef void (*retro_input_poll_t)(void);
typedef int16_t (*retro_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);

void retro_init(void);
void retro_deinit(void);
unsigned retro_api_version(void);
void retro_get_system_info(struct retro_system_info *info);
void retro_get_system_av_info(struct retro_system_av_info *info);
void retro_set_environment(retro_environment_t);
void retro_set_video_refresh(retro_video_refresh_t);
void retro_set_audio_sample(retro_audio_sample_t);
void retro_set_audio_sample_batch(retro_audio_sample_batch_t);
void retro_set_input_poll(retro_input_poll_t);
void retro_set_input_state(retro_input_state_t);
void retro_set_controller_port_device(unsigned port, unsigned device);
void retro_reset(void);
void retro_run(void);
size_t retro_serialize_size(void);
bool retro_serialize(void *data, size_t size);
bool retro_unserialize(const void *data, size_t size);
void retro_cheat_reset(void);
void retro_cheat_set(unsigned index, bool enabled, const char *code);
bool retro_load_game(const struct retro_game_info *game);
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info);
void retro_unload_game(void);
unsigned retro_get_region(void);
void *retro_get_memory_data(unsigned id);
size_t retro_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
