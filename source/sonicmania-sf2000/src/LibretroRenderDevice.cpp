#include "libretro_shared.h"
#include "sonic_hackerlog.h"
#include "RSDK/Scene/Scene.hpp"
#include "RSDK/Input/Input.hpp"

extern "C" void xlog(const char *fmt, ...);

namespace RSDK
{

static_assert(sizeof(ScreenInfo) == 432 * 240 * sizeof(uint16) + 48,
              "ScreenInfo must match SonicMania/GameLink.h");

static RenderDevice::DisplayInfo s_libretro_display = { 0, 424, 240, 60 };

bool RenderDevice::Init()
{
    xlog("[RENDER] RenderDevice::Init enter\n");
    displayCount = 1;
    displayInfo.displays = &s_libretro_display;
    videoSettings.pixWidth = 424;
    videoSettings.pixHeight = 240;

    for (int32 s = 0; s < SCREEN_COUNT; ++s) {
        memset(&screens[s].frameBuffer, 0, sizeof(screens[s].frameBuffer));
        SetScreenSize(s, 424, 240);
        xlog("[RENDER] screen=%d size=%dx%d pitch=%d fb=%p\n", s,
             screens[s].size.x, screens[s].size.y, screens[s].pitch, (void *)screens[s].frameBuffer);
    }

    int32 size = videoSettings.pixWidth >= SCREEN_YSIZE ? videoSettings.pixWidth : SCREEN_YSIZE;
    scanlines = (ScanlineInfo *)malloc(size * sizeof(ScanlineInfo));
    if (scanlines)
        memset(scanlines, 0, size * sizeof(ScanlineInfo));

    videoSettings.screenCount = 1;
    currentScreen = &screens[0];
    videoSettings.windowState = WINDOWSTATE_ACTIVE;
    videoSettings.dimMax      = 1.0;
    videoSettings.dimPercent  = 1.0;
    videoSettings.refreshRate = 60;
    engine.inFocus            = 1;

    xlog("[RENDER] RenderDevice::Init: calling AudioDevice::Init...\n");
    if (!AudioDevice::Init()) {
        xlog("[RENDER] AudioDevice::Init failed!\n");
        return false;
    }

    /* The engine only registers input devices from a platform render device's
     * Init -- SDL2, Vulkan, DX9/11, GLFW and EGL each call this, and this file
     * did not, so inputDeviceList stayed empty and RSDK::ProcessInput never saw
     * a single button press: the game was completely unresponsive. */
    xlog("[RENDER] RenderDevice::Init: calling InitInputDevices...\n");
    InitInputDevices();

    RenderDevice::isRunning = true;
    xlog("[RENDER] RenderDevice::Init success, isRunning=true\n");
    return true;
}

void RenderDevice::CopyFrameBuffer()
{
}

void RenderDevice::FlipScreen()
{
    if (screens[0].frameBuffer) {
        /* FroggyKVM hands the screen to the game on its first real frame
         * (gb300_video_flush). Until then the console keeps owning it, so the
         * engine's output is not pushed while the boot log is visible. */
        if (sonic_hackerlog_is_boot_active())
            return;

        static unsigned flip_cnt = 0;
        static int last_scene = -1;
        if (flip_cnt < 4 || flip_cnt % 600 == 0 || last_scene != sceneInfo.listPos) {
            const auto &screen = screens[0];
            xlog("[RENDER] frame=%u scene=%d state=%d size=%dx%d center=%d,%d position=%d,%d pitch=%d clip=%d,%d-%d,%d screen_bytes=%zu\n",
                 flip_cnt, sceneInfo.listPos, sceneInfo.state,
                 screen.size.x, screen.size.y, screen.center.x, screen.center.y,
                 screen.position.x, screen.position.y, screen.pitch,
                 screen.clipBound_X1, screen.clipBound_Y1, screen.clipBound_X2, screen.clipBound_Y2, sizeof(ScreenInfo));
            last_scene = sceneInfo.listPos;
        }
        ++flip_cnt;

        sonic_present_frame(screens[0].frameBuffer, screens[0].size.x, screens[0].size.y, screens[0].pitch * sizeof(uint16));
    }
}

void RenderDevice::Release(bool32 isRefresh)
{
    AudioDevice::Release();
    if (scanlines) {
        free(scanlines);
        scanlines = NULL;
    }
    RenderDevice::isRunning = false;
}

void RenderDevice::GetWindowSize(int32 *width, int32 *height)
{
    if (width) *width = 424;
    if (height) *height = 240;
}

bool RenderDevice::ProcessEvents()
{
    if (s_input_poll_cb)
        s_input_poll_cb();
    return RenderDevice::isRunning;
}

} // namespace RSDK
