#ifndef INPUT_DEVICE_LIBRETRO_H
#define INPUT_DEVICE_LIBRETRO_H

namespace SKU
{

struct InputDeviceLibretro : InputDevice {
    void UpdateInput();
    void ProcessInput(int32 controllerID);
    void CloseDevice();
    ~InputDeviceLibretro() = default;
};

InputDeviceLibretro *InitLibretroInputDevice(uint32 id, uint8 controllerID);
void InitLibretroInputAPI();

} // namespace SKU

#endif
