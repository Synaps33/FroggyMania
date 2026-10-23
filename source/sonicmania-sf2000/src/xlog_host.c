/* xlog() is the frontend's logging hook.
 *
 * On the console it is provided by the FroggyKVM multicore frontend. The host
 * test runner links the core sources directly, so it needs a definition too --
 * otherwise every xlog() call (Scene.cpp, Storage.cpp, Audio.cpp, Reader.cpp,
 * libretro.cpp ...) fails to resolve.
 *
 * When building for the console this file compiles to an empty object: the real
 * xlog lives in the frontend and must not be shadowed here.
 */
#include <stdio.h>
#include <stdarg.h>
#include "libretro.h"

#if !defined(SF2000) && !defined(GB300) && !defined(GB300V2)

static retro_log_printf_t frontend_log;

void xlog_set_environment(retro_environment_t cb)
{
    struct retro_log_callback logging = { 0 };
    frontend_log = cb && cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging) ? logging.log : NULL;
}

void xlog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (frontend_log) {
        char message[2048];
        vsnprintf(message, sizeof(message), fmt, ap);
        frontend_log(RETRO_LOG_INFO, "[FROGGYMANIA] %s", message);
    }
    else {
        vfprintf(stdout, fmt, ap);
        fflush(stdout);
    }
    va_end(ap);
}

#endif
