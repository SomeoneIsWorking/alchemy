#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ig_controller.h"
#include "ig_sdl_controller.h"

static int s_connect_count;
static int s_disconnect_count;

static void on_connect(ig_controller_manager *manager, ig_controller *controller)
{
    (void)manager;
    assert(controller->connected);
    assert(controller->device_id != 0);
    ++s_connect_count;
}

static void on_disconnect(ig_controller_manager *manager, ig_controller *controller)
{
    (void)manager;
    assert(controller->connected);
    ++s_disconnect_count;
}

static ig_controller_device device(uint32_t id)
{
    ig_controller_device value = {
        .device_id = id,
        .backend_handle = (void *)(uintptr_t)id,
        .type = IG_CONTROLLER_TYPE_UNKNOWN,
        .is_console = 1,
    };
    return value;
}

static void test_mapping(void)
{
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_SOUTH) ==
           IG_CONTROLLER_BUTTON_FACE_DOWN);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_EAST) ==
           IG_CONTROLLER_BUTTON_FACE_RIGHT);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_WEST) ==
           IG_CONTROLLER_BUTTON_FACE_LEFT);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_NORTH) ==
           IG_CONTROLLER_BUTTON_FACE_UP);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_BACK) ==
           IG_CONTROLLER_BUTTON_SELECT);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_START) ==
           IG_CONTROLLER_BUTTON_START);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_LEFT_STICK) ==
           IG_CONTROLLER_BUTTON_LEFT_STICK);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_RIGHT_STICK) ==
           IG_CONTROLLER_BUTTON_RIGHT_STICK);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) ==
           IG_CONTROLLER_BUTTON_LEFT_SHOULDER);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) ==
           IG_CONTROLLER_BUTTON_RIGHT_SHOULDER);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_DPAD_UP) ==
           IG_CONTROLLER_BUTTON_DPAD_UP);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_DPAD_DOWN) ==
           IG_CONTROLLER_BUTTON_DPAD_DOWN);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_DPAD_LEFT) ==
           IG_CONTROLLER_BUTTON_DPAD_LEFT);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_DPAD_RIGHT) ==
           IG_CONTROLLER_BUTTON_DPAD_RIGHT);
    assert(ig_sdl_controller_button_to_ig(SDL_GAMEPAD_BUTTON_MISC1) ==
           IG_CONTROLLER_BUTTON_UNMAPPED);
    printf("mapping: 15 mapped buttons and an unmapped discriminator passed\n");
}

static void test_state_model(void)
{
    ig_controller_manager manager;
    ig_controller_device info = device(10);
    ig_controller *controller;
    float x;
    float y;

    s_connect_count = 0;
    s_disconnect_count = 0;
    ig_controller_manager_init(&manager);
    ig_controller_manager_set_callbacks(&manager, on_connect, on_disconnect, NULL);
    controller = ig_controller_manager_connect(&manager, &info);
    assert(controller);
    assert(s_connect_count == 1);
    assert(ig_controller_manager_get_count(&manager) == 1);

    ig_controller_set_button_state(controller, IG_CONTROLLER_BUTTON_START, 1);
    ig_controller_set_button_pressure(controller, IG_CONTROLLER_BUTTON_LEFT_TRIGGER, 2.0f);
    ig_controller_set_joystick(controller, 1, 1.0f, -1.0f);
    assert(ig_controller_get_button_state(controller, IG_CONTROLLER_BUTTON_START));
    assert(ig_controller_get_button_pressure(controller, IG_CONTROLLER_BUTTON_LEFT_TRIGGER) ==
           1.0f);
    ig_controller_get_joystick(controller, 1, &x, &y);
    assert(x == 1.0f && y == -1.0f);

    ig_controller_manager_disconnect(&manager, 10);
    assert(s_disconnect_count == 1);
    assert(ig_controller_manager_get_count(&manager) == 0);
    ig_controller_manager_shutdown(&manager);
    printf("state model: callback validity, buttons, pressure and sticks passed\n");
}

static void test_stable_slots(void)
{
    ig_controller_manager manager;
    ig_controller_device first = device(10);
    ig_controller_device second = device(11);
    ig_controller_device replacement = device(12);
    ig_controller *controller0;
    ig_controller *controller1;
    ig_controller *controller2;

    ig_controller_manager_init(&manager);
    controller0 = ig_controller_manager_connect(&manager, &first);
    controller1 = ig_controller_manager_connect(&manager, &second);
    assert(controller0 && controller1);
    assert(controller0->id == 0 && controller1->id == 1);

    ig_controller_manager_disconnect(&manager, 10);
    assert(ig_controller_manager_find(&manager, 11) == controller1);
    assert(controller1->id == 1);
    controller2 = ig_controller_manager_connect(&manager, &replacement);
    assert(controller2 == controller0);
    assert(controller2->id == 0);
    assert(ig_controller_manager_get(&manager, 0) == controller2);
    assert(ig_controller_manager_get(&manager, 1) == controller1);
    ig_controller_manager_shutdown(&manager);
    printf("stable slots: removing one device did not move another device\n");
}

static SDL_JoystickID attach_virtual_gamepad(void)
{
    SDL_VirtualJoystickDesc descriptor;

    SDL_INIT_INTERFACE(&descriptor);
    descriptor.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    descriptor.naxes = SDL_GAMEPAD_AXIS_COUNT;
    descriptor.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    descriptor.nhats = 1;
    return SDL_AttachVirtualJoystick(&descriptor);
}

static void add_virtual_mapping(SDL_JoystickID id, const char *name)
{
    SDL_GUID guid = SDL_GetJoystickGUIDForID(id);
    char guid_text[64];
    char mapping[512];

    SDL_GUIDToString(guid, guid_text, sizeof(guid_text));
    snprintf(mapping, sizeof(mapping),
             "%s,%s,a:b0,b:b1,x:b2,y:b3,back:b4,start:b6,"
             "leftshoulder:b9,rightshoulder:b10,leftstick:b7,rightstick:b8,"
             "dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,"
             "leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,",
             guid_text, name);
    assert(SDL_AddGamepadMapping(mapping) >= 0);
}

static int forward_pending_events(ig_controller_manager *manager)
{
    SDL_Event event;
    int forwarded = 0;

    while (SDL_PollEvent(&event)) {
        ig_sdl_controller_handle_event(manager, &event);
        ++forwarded;
    }
    return forwarded;
}

static void test_sdl_event_and_snapshot_path(void)
{
    ig_controller_manager manager;
    SDL_JoystickID startup_id;
    SDL_JoystickID late_id;
    SDL_Joystick *joystick;
    ig_controller *startup;
    ig_controller *late;
    SDL_Event unrelated;
    float x;
    float y;

    s_connect_count = 0;
    s_disconnect_count = 0;
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        printf("SKIP SDL controller path: %s\n", SDL_GetError());
        return;
    }

    startup_id = attach_virtual_gamepad();
    if (startup_id == 0) {
        printf("SKIP SDL virtual controller: %s\n", SDL_GetError());
        return;
    }
    add_virtual_mapping(startup_id, "Startup Virtual Controller");

    ig_controller_manager_init(&manager);
    ig_controller_manager_set_callbacks(&manager, on_connect, on_disconnect, NULL);
    assert(ig_sdl_controller_initialize(&manager) == 0);
    assert(ig_controller_manager_get_count(&manager) == 1);
    assert(s_connect_count == 1);
    startup = ig_controller_manager_find(&manager, startup_id);
    assert(startup && startup->connected);

    memset(&unrelated, 0, sizeof(unrelated));
    unrelated.type = SDL_EVENT_USER;
    ig_sdl_controller_handle_event(&manager, &unrelated);
    assert(ig_controller_manager_get_count(&manager) == 1);

    joystick = SDL_OpenJoystick(startup_id);
    assert(joystick);
    assert(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, 1));
    assert(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 32767));
    forward_pending_events(&manager);
    ig_sdl_controller_update(&manager);
    assert(ig_controller_get_button_state(startup, IG_CONTROLLER_BUTTON_FACE_DOWN));
    ig_controller_get_joystick(startup, 1, &x, &y);
    assert(x > 0.99f && y == 0.0f);

    late_id = attach_virtual_gamepad();
    assert(late_id != 0);
    add_virtual_mapping(late_id, "Late Virtual Controller");
    assert(forward_pending_events(&manager) > 0);
    assert(ig_controller_manager_get_count(&manager) == 2);
    late = ig_controller_manager_find(&manager, late_id);
    assert(late && late->connected);
    assert(s_connect_count == 2);

    SDL_CloseJoystick(joystick);
    assert(SDL_DetachVirtualJoystick(startup_id));
    assert(forward_pending_events(&manager) > 0);
    assert(ig_controller_manager_get_count(&manager) == 1);
    assert(ig_controller_manager_find(&manager, late_id) == late);
    assert(s_disconnect_count == 1);

    assert(SDL_DetachVirtualJoystick(late_id));
    forward_pending_events(&manager);
    ig_sdl_controller_shutdown(&manager);
    ig_controller_manager_shutdown(&manager);
    assert(s_disconnect_count == 2);
    printf("SDL path: startup enumeration, central event forwarding, snapshot and late attach passed\n");
}

int main(void)
{
    test_mapping();
    test_state_model();
    test_stable_slots();
    test_sdl_event_and_snapshot_path();
    printf("all controller tests passed\n");
    return 0;
}
