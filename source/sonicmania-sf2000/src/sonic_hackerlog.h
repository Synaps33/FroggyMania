#ifndef SONIC_HACKERLOG_H
#define SONIC_HACKERLOG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define HACKER_MAX_LOGS 15
#define HACKER_LINE_MAX 42

/* Color constants in RGB565 from froggykvm */
#define COLOR_BLACK         0x0000
#define COLOR_SCANLINE      0x0080  /* Subtle CRT scanline */
#define COLOR_MATRIX_GREEN  0x07E0  /* Classic Terminal Green text */
#define COLOR_AMBER         0xFFE0  /* Amber / Yellow */
#define COLOR_WHITE         0xFFFF  /* White */
#define COLOR_CYAN          0x07FF  /* Cyan */

void sonic_hackerlog_init(const char *rom_path);
void sonic_hackerlog_add(const char *tag, const char *msg);
void sonic_hackerlog_progress(int pct);
void sonic_hackerlog_load(const char *msg);
void sonic_hackerlog_set_main_class(const char *main_class);
void sonic_hackerlog_render_boot(uint16_t *fb, int frame_step);
void sonic_hackerlog_render_overlay(uint16_t *fb, const char *scene_name, const char *category_name, int entities, int fps);
bool sonic_hackerlog_is_boot_active(void);
bool sonic_hackerlog_boot_complete(void);
void sonic_hackerlog_dismiss_boot(void);
void sonic_hackerlog_exit(int exit_code);
void sonic_hackerlog_toggle(void);
bool sonic_hackerlog_is_overlay_active(void);

#ifdef __cplusplus
}
#endif

#endif /* SONIC_HACKERLOG_H */
