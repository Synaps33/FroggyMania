namespace RSDK
{

bool32 AudioDevice::Init()
{
    InitAudioChannels();
    audioState = 1;
    initializedAudioChannels = true;
    return true;
}

void AudioDevice::Release()
{
    audioState = 0;
}

void AudioDevice::HandleStreamLoad(ChannelInfo *channel, bool32 async)
{
    LoadStream(channel);
}

} // namespace RSDK
