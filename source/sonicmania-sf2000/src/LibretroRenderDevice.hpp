#ifndef RENDER_DEVICE_LIBRETRO_H
#define RENDER_DEVICE_LIBRETRO_H

using ShaderEntry = ShaderEntryBase;

class RenderDevice : public RenderDeviceBase
{
public:
    struct DisplayInfo {
        uint32 _pad;
        int32 width;
        int32 height;
        int32 refresh_rate;
    };
    struct WindowInfo {
        DisplayInfo *displays;
    };
    static WindowInfo displayInfo;

    static bool Init();
    static void CopyFrameBuffer();
    static void FlipScreen();
    static void Release(bool32 isRefresh);

    static void RefreshWindow() {}
    static void GetWindowSize(int32 *width, int32 *height);

    static void SetupImageTexture(int32 width, int32 height, uint8 *imagePixels) {}
    static void SetupVideoTexture_YUV420(int32 width, int32 height, uint8 *yPlane, uint8 *uPlane, uint8 *vPlane, int32 strideY, int32 strideU, int32 strideV) {}
    static void SetupVideoTexture_YUV422(int32 width, int32 height, uint8 *yPlane, uint8 *uPlane, uint8 *vPlane, int32 strideY, int32 strideU, int32 strideV) {}
    static void SetupVideoTexture_YUV444(int32 width, int32 height, uint8 *yPlane, uint8 *uPlane, uint8 *vPlane, int32 strideY, int32 strideU, int32 strideV) {}

    static bool ProcessEvents();

    static void InitFPSCap() {}
    static bool CheckFPSCap() { return true; }
    static void UpdateFPSCap() {}

    static bool InitShaders() { return true; }
    static void LoadShader(const char *fileName, bool32 linear) {}

    static inline void ShowCursor(bool32 shown) {}
    static inline void SetWindowTitle() {}
    static inline void ProcessScanlines() {}
};

#endif
