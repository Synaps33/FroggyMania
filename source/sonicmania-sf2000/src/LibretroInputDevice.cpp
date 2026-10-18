#include "libretro_shared.h"

namespace RSDK
{
namespace SKU
{

void InputDeviceLibretro::UpdateInput()
{
    this->anyPress = false;
    if (s_input_state_cb) {
        static const unsigned s_buttons[] = {
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
        };
        uint16_t mask = 0;
        for (size_t i = 0; i < sizeof(s_buttons) / sizeof(s_buttons[0]); i++) {
            if (s_input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, s_buttons[i])) {
                mask |= (1 << s_buttons[i]);
            }
        }
        s_input_state = mask;

    }
    if (s_input_state)
        this->anyPress = true;

    ProcessInput(CONT_ANY);
    ProcessInput(CONT_P1);
}

void InputDeviceLibretro::ProcessInput(int32 controllerID)
{
    bool up    = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_UP)) != 0;
    bool down  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN)) != 0;
    bool left  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT)) != 0;
    bool right = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT)) != 0;
    bool btnA  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_A)) != 0;
    bool btnB  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_B)) != 0;
    bool btnX  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_X)) != 0;
    bool btnY  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_Y)) != 0;
    bool start = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_START)) != 0;
    bool select= (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT)) != 0;
    bool btnL  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_L)) != 0;
    bool btnR  = (s_input_state & (1 << RETRO_DEVICE_ID_JOYPAD_R)) != 0;

    controller[controllerID].keyUp.press     |= up;
    controller[controllerID].keyDown.press   |= down;
    controller[controllerID].keyLeft.press   |= left;
    controller[controllerID].keyRight.press  |= right;
    controller[controllerID].keyA.press      |= (btnA || btnX);
    controller[controllerID].keyB.press      |= (btnB || btnY);
    controller[controllerID].keyC.press      |= (btnB || btnA);
    controller[controllerID].keyX.press      |= btnX;
    controller[controllerID].keyY.press      |= btnY;
    controller[controllerID].keyStart.press  |= start;
    controller[controllerID].keySelect.press |= select;

    stickL[controllerID].keyUp.press    |= up;
    stickL[controllerID].keyDown.press  |= down;
    stickL[controllerID].keyLeft.press  |= left;
    stickL[controllerID].keyRight.press |= right;
    stickL[controllerID].hDelta = right ? 32767 : (left ? -32767 : 0);
    stickL[controllerID].vDelta = down ? -32767 : (up ? 32767 : 0);

#if RETRO_REV02
    triggerL[controllerID].keyBumper.press |= btnL;
    triggerR[controllerID].keyBumper.press |= btnR;
#else
    controller[controllerID].keyBumperL.press |= btnL;
    controller[controllerID].keyBumperR.press |= btnR;
#endif
}

void InputDeviceLibretro::CloseDevice()
{
    this->active = false;
}

InputDeviceLibretro *InitLibretroInputDevice(uint32 id, uint8 controllerID)
{
    if (inputDeviceCount >= INPUTDEVICE_COUNT)
        return NULL;

    inputDeviceList[inputDeviceCount] = new InputDeviceLibretro();
    InputDeviceLibretro *device = (InputDeviceLibretro *)inputDeviceList[inputDeviceCount];

    device->id          = id;
    device->gamepadType = (DEVICE_API_NONE << 16) | (DEVICE_TYPE_CONTROLLER << 8) | (0 << 0);
    device->active      = true;
    inputDeviceCount++;

    AssignInputSlotToDevice(controllerID, id);
    return device;
}

void InitLibretroInputAPI()
{
    InitLibretroInputDevice(0x1337, CONT_P1);
}

} // namespace SKU
} // namespace RSDK
