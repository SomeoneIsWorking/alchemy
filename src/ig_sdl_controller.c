#include "ig_sdl_controller.h"

#include <SDL3/SDL.h>

#define IG_STICK_DEADZONE 0.1f
#define IG_TRIGGER_BUTTON_THRESHOLD 0.5f

ig_controller_button ig_sdl_controller_button_to_ig(SDL_GamepadButton button)
{
    switch (button) {
    case SDL_GAMEPAD_BUTTON_SOUTH:
        return IG_CONTROLLER_BUTTON_FACE_DOWN;
    case SDL_GAMEPAD_BUTTON_EAST:
        return IG_CONTROLLER_BUTTON_FACE_RIGHT;
    case SDL_GAMEPAD_BUTTON_WEST:
        return IG_CONTROLLER_BUTTON_FACE_LEFT;
    case SDL_GAMEPAD_BUTTON_NORTH:
        return IG_CONTROLLER_BUTTON_FACE_UP;
    case SDL_GAMEPAD_BUTTON_BACK:
        return IG_CONTROLLER_BUTTON_SELECT;
    case SDL_GAMEPAD_BUTTON_START:
        return IG_CONTROLLER_BUTTON_START;
    case SDL_GAMEPAD_BUTTON_LEFT_STICK:
        return IG_CONTROLLER_BUTTON_LEFT_STICK;
    case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
        return IG_CONTROLLER_BUTTON_RIGHT_STICK;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
        return IG_CONTROLLER_BUTTON_LEFT_SHOULDER;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
        return IG_CONTROLLER_BUTTON_RIGHT_SHOULDER;
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        return IG_CONTROLLER_BUTTON_DPAD_UP;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
        return IG_CONTROLLER_BUTTON_DPAD_DOWN;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
        return IG_CONTROLLER_BUTTON_DPAD_LEFT;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        return IG_CONTROLLER_BUTTON_DPAD_RIGHT;
    case SDL_GAMEPAD_BUTTON_GUIDE:
        return IG_CONTROLLER_BUTTON_16;
    default:
        return IG_CONTROLLER_BUTTON_UNMAPPED;
    }
}

static float normalize_stick(Sint16 value)
{
    float normalized = value >= 0 ? (float)value / 32767.0f : (float)value / 32768.0f;
    if (normalized > -IG_STICK_DEADZONE && normalized < IG_STICK_DEADZONE) {
        return 0.0f;
    }
    return normalized;
}

static float normalize_trigger(Sint16 value)
{
    return value <= 0 ? 0.0f : (float)value / 32767.0f;
}

static ig_controller_type detect_type(SDL_Gamepad *gamepad)
{
    switch (SDL_GetGamepadType(gamepad)) {
    case SDL_GAMEPAD_TYPE_XBOX360:
    case SDL_GAMEPAD_TYPE_XBOXONE:
        return IG_CONTROLLER_TYPE_XBOX360_MICROSOFT_10BUTTONS_POV;
    default:
        return IG_CONTROLLER_TYPE_UNKNOWN;
    }
}

static void add_controller(ig_controller_manager *manager, SDL_JoystickID which)
{
    SDL_Gamepad *gamepad = SDL_OpenGamepad(which);
    ig_controller_device device;

    if (!gamepad) {
        return;
    }
    device.device_id = SDL_GetGamepadID(gamepad);
    if (ig_controller_manager_find(manager, device.device_id)) {
        SDL_CloseGamepad(gamepad);
        return;
    }
    device.backend_handle = gamepad;
    device.type = detect_type(gamepad);
    device.is_console = 1;
    if (!ig_controller_manager_connect(manager, &device)) {
        SDL_CloseGamepad(gamepad);
    }
}

static void remove_controller(ig_controller_manager *manager, SDL_JoystickID which)
{
    ig_controller *controller = ig_controller_manager_find(manager, which);
    SDL_Gamepad *gamepad;

    if (!controller) {
        return;
    }
    gamepad = controller->backend_handle;
    ig_controller_manager_disconnect(manager, which);
    if (gamepad) {
        SDL_CloseGamepad(gamepad);
    }
}

int ig_sdl_controller_initialize(ig_controller_manager *manager)
{
    SDL_JoystickID *gamepads;
    int count = 0;

    if (SDL_WasInit(SDL_INIT_GAMEPAD) == 0 && !SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        return -1;
    }
    gamepads = SDL_GetGamepads(&count);
    if (!gamepads && count != 0) {
        return -1;
    }
    for (int i = 0; i < count; ++i) {
        add_controller(manager, gamepads[i]);
    }
    SDL_free(gamepads);
    return 0;
}

static void set_button(ig_controller *controller, SDL_GamepadButton button, int pressed)
{
    ig_controller_button mapped = ig_sdl_controller_button_to_ig(button);
    if (mapped == IG_CONTROLLER_BUTTON_UNMAPPED) {
        return;
    }
    ig_controller_set_button_state(controller, mapped, pressed);
    ig_controller_set_button_pressure(controller, mapped, pressed ? 1.0f : 0.0f);
}

static void set_axis(ig_controller *controller, SDL_GamepadAxis axis, Sint16 value)
{
    float pressure;

    switch (axis) {
    case SDL_GAMEPAD_AXIS_LEFTX:
        ig_controller_set_joystick(controller, 0, normalize_stick(value),
                                   controller->joystick[0][1]);
        break;
    case SDL_GAMEPAD_AXIS_LEFTY:
        ig_controller_set_joystick(controller, 0, controller->joystick[0][0],
                                   normalize_stick(value));
        break;
    case SDL_GAMEPAD_AXIS_RIGHTX:
        ig_controller_set_joystick(controller, 1, normalize_stick(value),
                                   controller->joystick[1][1]);
        break;
    case SDL_GAMEPAD_AXIS_RIGHTY:
        ig_controller_set_joystick(controller, 1, controller->joystick[1][0],
                                   normalize_stick(value));
        break;
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
        pressure = normalize_trigger(value);
        ig_controller_set_button_pressure(controller, IG_CONTROLLER_BUTTON_LEFT_TRIGGER,
                                          pressure);
        ig_controller_set_button_state(controller, IG_CONTROLLER_BUTTON_LEFT_TRIGGER,
                                       pressure > IG_TRIGGER_BUTTON_THRESHOLD);
        break;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
        pressure = normalize_trigger(value);
        ig_controller_set_button_pressure(controller, IG_CONTROLLER_BUTTON_RIGHT_TRIGGER,
                                          pressure);
        ig_controller_set_button_state(controller, IG_CONTROLLER_BUTTON_RIGHT_TRIGGER,
                                       pressure > IG_TRIGGER_BUTTON_THRESHOLD);
        break;
    default:
        break;
    }
}

void ig_sdl_controller_handle_event(ig_controller_manager *manager, const SDL_Event *event)
{
    ig_controller *controller;

    if (!manager || !event) {
        return;
    }
    switch (event->type) {
    case SDL_EVENT_GAMEPAD_ADDED:
        add_controller(manager, event->gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_REMOVED:
        remove_controller(manager, event->gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        controller = ig_controller_manager_find(manager, event->gbutton.which);
        if (controller) {
            set_button(controller, event->gbutton.button,
                       event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
        break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        controller = ig_controller_manager_find(manager, event->gaxis.which);
        if (controller) {
            set_axis(controller, event->gaxis.axis, event->gaxis.value);
        }
        break;
    case SDL_EVENT_GAMEPAD_REMAPPED:
        controller = ig_controller_manager_find(manager, event->gdevice.which);
        if (controller) {
            controller->type = detect_type(controller->backend_handle);
        }
        break;
    default:
        break;
    }
}

void ig_sdl_controller_update(ig_controller_manager *manager)
{
    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        ig_controller *controller = &manager->controllers[i];
        SDL_Gamepad *gamepad;

        if (!controller->connected || !controller->backend_handle) {
            continue;
        }
        gamepad = controller->backend_handle;
        for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
            set_button(controller, (SDL_GamepadButton)button,
                       SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)button));
        }
        for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) {
            set_axis(controller, (SDL_GamepadAxis)axis,
                     SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)axis));
        }
    }
}

void ig_sdl_controller_shutdown(ig_controller_manager *manager)
{
    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        ig_controller *controller = &manager->controllers[i];
        SDL_Gamepad *gamepad;
        uint32_t device_id;

        if (!controller->connected) {
            continue;
        }
        gamepad = controller->backend_handle;
        device_id = controller->device_id;
        ig_controller_manager_disconnect(manager, device_id);
        if (gamepad) {
            SDL_CloseGamepad(gamepad);
        }
    }
}

void ig_sdl_controller_set_rumble(ig_controller *controller, int motor, float speed)
{
    SDL_Gamepad *gamepad;
    uint16_t low = 0;
    uint16_t high = 0;

    if (!controller || !controller->backend_handle) {
        return;
    }
    if (speed < 0.0f) {
        speed = 0.0f;
    } else if (speed > 1.0f) {
        speed = 1.0f;
    }
    if (motor == 0) {
        low = (uint16_t)(speed * 65535.0f);
    } else {
        high = (uint16_t)(speed * 65535.0f);
    }
    gamepad = controller->backend_handle;
    SDL_RumbleGamepad(gamepad, low, high, 0);
}
