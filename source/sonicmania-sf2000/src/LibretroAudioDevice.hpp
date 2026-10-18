#ifndef AUDIO_DEVICE_LIBRETRO_H
#define AUDIO_DEVICE_LIBRETRO_H

#define LockAudioDevice()
#define UnlockAudioDevice()

namespace RSDK
{

class AudioDevice : public AudioDeviceBase
{
public:
    static bool32 Init();
    static void Release();
    static void FrameInit() {}
    static void HandleStreamLoad(ChannelInfo *channel, bool32 async);
};

} // namespace RSDK

#endif
