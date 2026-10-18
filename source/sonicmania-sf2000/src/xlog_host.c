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

#if !defined(SF2000) && !defined(GB300) && !defined(GB300V2)

void xlog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fflush(stdout);
}

#endif
