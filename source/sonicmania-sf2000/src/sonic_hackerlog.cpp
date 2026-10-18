/* =========================================================================
 *  Hacker Loader Console - Sonic Mania (RSDKv5) port for GB300 / SF2000
 *
 *  1:1 port of the console from froggykvm:
 *      froggykvm/pspkvm/platform_gb300/video.c
 *
 *  Differences are limited to:
 *    - the log tags emitted for this title
 *    - the ROM/MAIN banner strings
 *    - the in-game telemetry overlay (froggykvm has no overlay, the
 *      equivalent of `gb300_hacker_stop()` + SELECT toggle lives there)
 * ========================================================================= */

#include "sonic_hackerlog.h"

#include <stdio.h>
#include <string.h>

/* Provided by the frontend on the console and by src/xlog_host.c for the host
 * test runner, so declare it unconditionally: the call below is not guarded. */
extern "C" void xlog(const char *fmt, ...);

/* ========================================================================= */
/* Shared drawing primitives (mirrors gb300_draw_text / gb300_fill_rect)     */
/* ========================================================================= */

/* Standard 8x8 monochrome font for ASCII 32..126 (95 glyphs) */
static const uint8_t font8x8_basic[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 32   */
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, /* 33 ! */
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, /* 34 " */
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, /* 35 # */
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, /* 36 $ */
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, /* 37 % */
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, /* 38 & */
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, /* 39 ' */
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, /* 40 ( */
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, /* 41 ) */
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, /* 42 * */
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, /* 43 + */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, /* 44 , */
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, /* 45 - */
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, /* 46 . */
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, /* 47 / */
    {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0x00}, /* 48 0 */
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, /* 49 1 */
    {0x7C,0xC6,0x06,0x1C,0x30,0x66,0xFE,0x00}, /* 50 2 */
    {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00}, /* 51 3 */
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, /* 52 4 */
    {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00}, /* 53 5 */
    {0x78,0x0C,0xC0,0xFC,0xC6,0xC6,0x7C,0x00}, /* 54 6 */
    {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00}, /* 55 7 */
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00}, /* 56 8 */
    {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00}, /* 57 9 */
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00}, /* 58 : */
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00}, /* 59 ; */
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, /* 60 < */
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, /* 61 = */
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00}, /* 62 > */
    {0x7C,0xC6,0x0C,0x18,0x18,0x00,0x18,0x00}, /* 63 ? */
    {0x7C,0xC6,0xDE,0xDE,0xDC,0xC0,0x7C,0x00}, /* 64 @ */
    {0x38,0x6C,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, /* 65 A */
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, /* 66 B */
    {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00}, /* 67 C */
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00}, /* 68 D */
    {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00}, /* 69 E */
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00}, /* 70 F */
    {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00}, /* 71 G */
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, /* 72 H */
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, /* 73 I */
    {0x1E,0x0C,0x0C,0x0C,0x0C,0xCC,0x78,0x00}, /* 74 J */
    {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00}, /* 75 K */
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00}, /* 76 L */
    {0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00}, /* 77 M */
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00}, /* 78 N */
    {0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00}, /* 79 O */
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00}, /* 80 P */
    {0x7C,0xC6,0xC6,0xC6,0xD6,0x7C,0x0E,0x00}, /* 81 Q */
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00}, /* 82 R */
    {0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00}, /* 83 S */
    {0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00}, /* 84 T */
    {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, /* 85 U */
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00}, /* 86 V */
    {0xC6,0xC6,0xD6,0xFE,0xFE,0xEE,0xC6,0x00}, /* 87 W */
    {0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00}, /* 88 X */
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00}, /* 89 Y */
    {0xFE,0xC6,0x0C,0x18,0x30,0x62,0xFE,0x00}, /* 90 Z */
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, /* 91 [ */
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, /* 92 \ */
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, /* 93 ] */
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00}, /* 94 ^ */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, /* 95 _ */
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, /* 96 ` */
    {0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00}, /* 97 a */
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0x00}, /* 98 b */
    {0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x00}, /* 99 c */
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00}, /* 100 d */
    {0x00,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00}, /* 101 e */
    {0x38,0x6C,0x60,0xF0,0x60,0x60,0xF0,0x00}, /* 102 f */
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8}, /* 103 g */
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00}, /* 104 h */
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, /* 105 i */
    {0x06,0x00,0x06,0x06,0x06,0x66,0x66,0x3C}, /* 106 j */
    {0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00}, /* 107 k */
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, /* 108 l */
    {0x00,0x00,0xE6,0xFF,0xDB,0xC9,0xC9,0x00}, /* 109 m */
    {0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x00}, /* 110 n */
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00}, /* 111 o */
    {0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0}, /* 112 p */
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E}, /* 113 q */
    {0x00,0x00,0xDC,0x76,0x60,0x60,0xF0,0x00}, /* 114 r */
    {0x00,0x00,0x7C,0xC0,0x7C,0x06,0xFC,0x00}, /* 115 s */
    {0x30,0x30,0xFC,0x30,0x30,0x34,0x18,0x00}, /* 116 t */
    {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00}, /* 117 u */
    {0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x00}, /* 118 v */
    {0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x6C,0x00}, /* 119 w */
    {0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00}, /* 120 x */
    {0x00,0x00,0xC6,0xC6,0xC6,0x7E,0x06,0xFC}, /* 121 y */
    {0x00,0x00,0x7E,0x4C,0x18,0x32,0x7E,0x00}, /* 122 z */
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, /* 123 { */
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, /* 124 | */
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, /* 125 } */
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}, /* 126 ~ */
};

static void draw_char8x8(uint16_t *fb, int x, int y, char c, uint16_t fg, uint16_t bg)
{
    if (c < 32 || c > 126) c = ' ';
    const uint8_t *glyph = font8x8_basic[(int)c - 32];

    for (int row = 0; row < 8; row++) {
        int py = y + row;
        if (py < 0 || py >= SCREEN_HEIGHT) continue;
        uint16_t *dst = &fb[py * SCREEN_WIDTH + x];
        uint8_t line = glyph[row];
        for (int col = 0; col < 8; col++) {
            int px = x + col;
            if (px >= 0 && px < SCREEN_WIDTH) {
                if (line & (0x80 >> col)) {
                    dst[col] = fg;
                } else if (bg != 0) {
                    dst[col] = bg;
                }
            }
        }
    }
}

static void draw_text(uint16_t *fb, int x, int y, const char *str, uint16_t fg, uint16_t bg)
{
    if (!str) return;
    int cur_x = x;
    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            y += 10;
        } else {
            draw_char8x8(fb, cur_x, y, *str, fg, bg);
            cur_x += 8;
            if (cur_x + 8 > SCREEN_WIDTH) break;
        }
        str++;
    }
}

static void fill_rect(uint16_t *fb, int x, int y, int w, int h, uint16_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int row = 0; row < h; row++) {
        uint16_t *line = &fb[(y + row) * SCREEN_WIDTH + x];
        for (int col = 0; col < w; col++) {
            line[col] = color;
        }
    }
}

/* ========================================================================= */
/* Hacker Loader state (mirrors video.c)                                     */
/* ========================================================================= */

static char s_hacker_logs[HACKER_MAX_LOGS][HACKER_LINE_MAX];
static int  s_hacker_log_count = 0;
static char s_rom_name[48] = {0};
static char s_main_class[48] = {0};
static uint32_t s_boot_start_ms = 0;
static int  s_boot_active = 0;
static int  s_hacker_progress = 0;

static int  s_overlay_active = 0;
static char s_rom_path[128] = {0};

/* Frame counter stands in for froggykvm's gb300_timer_get_ms() so the
   elapsed-time column advances at 12ms/frame (60Hz). */
static uint32_t s_hacker_tick_ms = 0;

static uint32_t hacker_timer_get_ms(void)
{
    return s_hacker_tick_ms;
}

/* ========================================================================= */
/* Boot console render (1:1 with hacker_render_screen)                        */
/* ========================================================================= */

static void hacker_render_screen(uint16_t *fb)
{
    /* 1. Black background with CRT scanlines */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        uint16_t line_color = (y & 3) ? COLOR_BLACK : COLOR_SCANLINE;
        uint16_t *line = &fb[y * SCREEN_WIDTH];
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            line[x] = line_color;
        }
    }

    /* 2. Top info: ROM File & Main Class */
    char rom_line[42];
    snprintf(rom_line, sizeof(rom_line), "ROM:  %.32s", s_rom_name[0] ? s_rom_name : "-");
    draw_text(fb, 8, 6, rom_line, COLOR_CYAN, 0);

    char cls_line[42];
    snprintf(cls_line, sizeof(cls_line), "MAIN: %.32s", s_main_class[0] ? s_main_class : "...");
    draw_text(fb, 8, 18, cls_line, COLOR_AMBER, 0);

    /* Separator line */
    fill_rect(fb, 8, 29, SCREEN_WIDTH - 16, 1, COLOR_SCANLINE);

    /* 3. Log stream */
    int log_start_y = 34;
    for (int i = 0; i < s_hacker_log_count && i < HACKER_MAX_LOGS; i++) {
        int text_y = log_start_y + i * 11;
        uint16_t text_color = (i == s_hacker_log_count - 1) ? COLOR_WHITE : COLOR_MATRIX_GREEN;
        draw_text(fb, 8, text_y, s_hacker_logs[i], text_color, 0);
    }

    /* 4. Bottom Progress Bar & Percentage */
    fill_rect(fb, 8, 205, SCREEN_WIDTH - 16, 1, COLOR_SCANLINE);

    char status_str[32];
    snprintf(status_str, sizeof(status_str), "BOOTING: %3d%%", s_hacker_progress);
    draw_text(fb, 8, 209, status_str, COLOR_CYAN, 0);

    /* Progress bar rectangle */
    int bar_x = 8;
    int bar_y = 221;
    int bar_w = SCREEN_WIDTH - 16; /* 304 px */
    int bar_h = 11;

    /* Outline */
    fill_rect(fb, bar_x, bar_y, bar_w, 1, COLOR_SCANLINE);
    fill_rect(fb, bar_x, bar_y + bar_h - 1, bar_w, 1, COLOR_SCANLINE);
    fill_rect(fb, bar_x, bar_y, 1, bar_h, COLOR_SCANLINE);
    fill_rect(fb, bar_x + bar_w - 1, bar_y, 1, bar_h, COLOR_SCANLINE);

    /* Fill */
    int max_fill = bar_w - 4;
    int fill_w  = (s_hacker_progress * max_fill) / 100;
    if (fill_w > max_fill) fill_w = max_fill;
    if (fill_w > 0) {
        fill_rect(fb, bar_x + 2, bar_y + 2, fill_w, bar_h - 4, COLOR_MATRIX_GREEN);
    }
}

/* ========================================================================= */
/* Public API                                                               */
/* ========================================================================= */

void sonic_hackerlog_init(const char *rom_path)
{
    s_boot_active     = 1;
    s_hacker_log_count = 0;
    s_hacker_progress  = 5;
    s_boot_start_ms    = hacker_timer_get_ms();
    s_hacker_tick_ms   = 0;
    s_main_class[0]    = '\0';
    s_overlay_active   = 0;

    if (rom_path && *rom_path) {
        snprintf(s_rom_path, sizeof(s_rom_path), "%.40s", rom_path);
        const char *b = strrchr(s_rom_path, '/');
        if (b) b++; else b = s_rom_path;
        snprintf(s_rom_name, sizeof(s_rom_name), "%.40s", b);
    } else {
        s_rom_path[0] = '\0';
        strcpy(s_rom_name, "ROM_STREAM");
    }

    sonic_hackerlog_add("SYS", "CORE INIT: MIPS32r2 200MHz");
    sonic_hackerlog_add("VFS", "MOUNT /dev/sdcard ... [OK]");
}

void sonic_hackerlog_add(const char *tag, const char *msg)
{
    if (!s_boot_active) return;

    if (s_boot_start_ms == 0) {
        s_boot_start_ms = hacker_timer_get_ms();
    }
    uint32_t now = hacker_timer_get_ms();
    uint32_t elapsed_ms = (now >= s_boot_start_ms) ? (now - s_boot_start_ms) : 0;
    uint32_t sec  = elapsed_ms / 1000;
    uint32_t msec = (elapsed_ms % 1000) / 10;

    char formatted[HACKER_LINE_MAX];
    snprintf(formatted, sizeof(formatted), "[%u.%02us] [%-4.4s] %.24s",
             (unsigned)sec, (unsigned)msec, tag ? tag : "SYS", msg ? msg : "");

    if (s_hacker_log_count < HACKER_MAX_LOGS) {
        strncpy(s_hacker_logs[s_hacker_log_count], formatted, HACKER_LINE_MAX - 1);
        s_hacker_logs[s_hacker_log_count][HACKER_LINE_MAX - 1] = '\0';
        s_hacker_log_count++;
    } else {
        memmove(&s_hacker_logs[0], &s_hacker_logs[1], (HACKER_MAX_LOGS - 1) * HACKER_LINE_MAX);
        strncpy(s_hacker_logs[HACKER_MAX_LOGS - 1], formatted, HACKER_LINE_MAX - 1);
        s_hacker_logs[HACKER_MAX_LOGS - 1][HACKER_LINE_MAX - 1] = '\0';
    }

#if defined(SF2000) || defined(GB300)
    xlog("[HACKERLOG] %s\n", formatted);
#endif
}

/* Equivalent of gb300_hacker_log_class(): drives progress + LOAD lines. */
void sonic_hackerlog_progress(int pct)
{
    if (!s_boot_active) return;
    if (pct > s_hacker_progress) {
        s_hacker_progress = pct;
        if (s_hacker_progress > 100) s_hacker_progress = 100;
    }
}

void sonic_hackerlog_load(const char *msg)
{
    if (!s_boot_active || !msg || !*msg) return;

    if (s_hacker_progress < 98) {
        s_hacker_progress++;
    }

    char line[32];
    snprintf(line, sizeof(line), "LOAD: %.24s", msg);
    sonic_hackerlog_add("LOAD", line);
}

void sonic_hackerlog_set_main_class(const char *main_class)
{
    if (main_class && *main_class) {
        snprintf(s_main_class, sizeof(s_main_class), "%.40s", main_class);
    }
}

/* ========================================================================= */
/* Frame pump - called every retro_run() while the console owns the screen    */
/* ========================================================================= */

void sonic_hackerlog_render_boot(uint16_t *fb, int frame_step)
{
    if (!fb || !s_boot_active) return;
    s_hacker_tick_ms += 16;
    hacker_render_screen(fb);
}

bool sonic_hackerlog_is_boot_active(void)
{
    return s_boot_active;
}

/* FroggyKVM hands the screen over as soon as the app produces its first real
 * frame. Sonic Mania finishes engine init in a couple of frames though, so a
 * bare handoff would make the console flash for 30ms. The console is therefore
 * held for HACKER_MIN_HOLD_MS after the log reaches 100%, which is what keeps
 * the boot report readable. START/A skips it early (see retro_run). */
#define HACKER_MIN_HOLD_MS 1500

bool sonic_hackerlog_boot_complete(void)
{
    if (!s_boot_active) return true;
    return s_hacker_progress >= 100 && s_hacker_tick_ms >= HACKER_MIN_HOLD_MS;
}

/* gb300_hacker_stop(): hand the screen over to the game. */
void sonic_hackerlog_dismiss_boot(void)
{
    xlog("[HACKERLOG] sonic_hackerlog_dismiss_boot called -> handing screen to engine!\n");
    s_boot_active = 0;
}

/* gb300_hacker_exit() equivalent - final message before handing over. */
void sonic_hackerlog_exit(int exit_code)
{
    s_boot_active      = 1;
    s_hacker_progress  = 100;
    char exit_msg[32];
    snprintf(exit_msg, sizeof(exit_msg), "ENGINE HALTED (CODE %d)", exit_code);
    sonic_hackerlog_add("EXIT", exit_msg);
    sonic_hackerlog_add("HALT", "SYS HALTED. SELECT+START");
}

void sonic_hackerlog_toggle(void)
{
    s_overlay_active = !s_overlay_active;
}

bool sonic_hackerlog_is_overlay_active(void)
{
    return s_overlay_active;
}

/* ========================================================================= */
/* In-game telemetry overlay (SELECT) - no equivalent in froggykvm, kept      */
/* on the same drawing primitives and palette as its console.                  */
/* ========================================================================= */

void sonic_hackerlog_render_overlay(uint16_t *fb, const char *scene_name, const char *category_name, int entities, int fps)
{
    if (!fb || !s_overlay_active) return;

    /* Semi-transparent dark terminal overlay on bottom half */
    int box_x = 8, box_y = 104, box_w = 304, box_h = 128;
    for (int y = box_y; y < box_y + box_h && y < SCREEN_HEIGHT; y++) {
        uint16_t *row = fb + y * SCREEN_WIDTH;
        for (int x = box_x; x < box_x + box_w && x < SCREEN_WIDTH; x++) {
            uint16_t p = row[x];
            /* 75% dark tint */
            row[x] = (p >> 2) & 0x39E7;
        }
    }

    /* Border */
    fill_rect(fb, box_x, box_y, box_w, 1, COLOR_MATRIX_GREEN);
    fill_rect(fb, box_x, box_y + box_h - 1, box_w, 1, COLOR_MATRIX_GREEN);

    draw_text(fb, box_x + 6, box_y + 4, "[ HACKERLOG TELEMETRY :: GB300/SF2000 ]", COLOR_CYAN, 0);

    char buf[64];
    snprintf(buf, sizeof(buf), "CORE: MIPS32 @ %dMHz | %dFPS | 320x240 RGB565",
             200, fps);
    draw_text(fb, box_x + 6, box_y + 18, buf, COLOR_AMBER, 0);

    snprintf(buf, sizeof(buf), "SCENE: %.24s | CAT: %.16s", scene_name ? scene_name : "-", category_name ? category_name : "-");
    draw_text(fb, box_x + 6, box_y + 30, buf, COLOR_WHITE, 0);

    snprintf(buf, sizeof(buf), "ENTITIES: %d | AUD: 44.1kHz ST 735S/F", entities);
    draw_text(fb, box_x + 6, box_y + 42, buf, COLOR_MATRIX_GREEN, 0);

    /* Show recent logs */
    int show_logs = 4;
    int start_idx = s_hacker_log_count - show_logs;
    if (start_idx < 0) start_idx = 0;

    for (int i = start_idx; i < s_hacker_log_count; i++) {
        int ly = box_y + 56 + (i - start_idx) * 11;
        draw_text(fb, box_x + 6, ly, s_hacker_logs[i],
                  (i == s_hacker_log_count - 1) ? COLOR_WHITE : COLOR_MATRIX_GREEN, 0);
    }

    /* Close hint */
    fill_rect(fb, box_x + 6, box_y + box_h - 14, box_w - 12, 1, COLOR_SCANLINE);
    draw_text(fb, box_x + 6, box_y + box_h - 11, "[SEL] CLOSE", COLOR_CYAN, 0);
}
