#include "ig_controller.h"

#include <stddef.h>
#include <string.h>

void ig_controller_manager_init(ig_controller_manager *manager)
{
    memset(manager, 0, sizeof(*manager));
}

void ig_controller_manager_shutdown(ig_controller_manager *manager)
{
    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        ig_controller *controller = &manager->controllers[i];
        if (controller->connected && manager->on_disconnect) {
            manager->on_disconnect(manager, controller);
        }
    }
    memset(manager->controllers, 0, sizeof(manager->controllers));
    manager->count = 0;
}

void ig_controller_manager_set_callbacks(ig_controller_manager *manager,
                                         ig_controller_connection_cb on_connect,
                                         ig_controller_disconnection_cb on_disconnect,
                                         void *userdata)
{
    manager->on_connect = on_connect;
    manager->on_disconnect = on_disconnect;
    manager->userdata = userdata;
}

int ig_controller_manager_get_count(const ig_controller_manager *manager)
{
    return manager->count;
}

ig_controller *ig_controller_manager_get(const ig_controller_manager *manager, int index)
{
    if (index < 0 || index >= manager->count) {
        return NULL;
    }

    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        if (!manager->controllers[i].connected) {
            continue;
        }
        if (index-- == 0) {
            return (ig_controller *)&manager->controllers[i];
        }
    }
    return NULL;
}

ig_controller *ig_controller_manager_find(const ig_controller_manager *manager,
                                          uint32_t device_id)
{
    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        ig_controller *controller = (ig_controller *)&manager->controllers[i];
        if (controller->connected && controller->device_id == device_id) {
            return controller;
        }
    }
    return NULL;
}

ig_controller *ig_controller_manager_connect(ig_controller_manager *manager,
                                             const ig_controller_device *device)
{
    ig_controller *controller;

    if (!device || device->device_id == 0 ||
        ig_controller_manager_find(manager, device->device_id)) {
        return NULL;
    }
    for (int i = 0; i < IG_CONTROLLER_MAX_COUNT; ++i) {
        controller = &manager->controllers[i];
        if (controller->connected) {
            continue;
        }

        memset(controller, 0, sizeof(*controller));
        controller->id = (uint16_t)i;
        controller->device_id = device->device_id;
        controller->backend_handle = device->backend_handle;
        controller->type = device->type;
        controller->is_console = device->is_console;
        controller->connected = 1;
        ++manager->count;
        if (manager->on_connect) {
            manager->on_connect(manager, controller);
        }
        return controller;
    }
    return NULL;
}

void ig_controller_manager_disconnect(ig_controller_manager *manager, uint32_t device_id)
{
    ig_controller *controller = ig_controller_manager_find(manager, device_id);
    if (!controller) {
        return;
    }
    if (manager->on_disconnect) {
        manager->on_disconnect(manager, controller);
    }
    memset(controller, 0, sizeof(*controller));
    --manager->count;
}

int ig_controller_is_connected(const ig_controller *controller)
{
    return controller && controller->connected;
}

uint32_t ig_controller_get_buttons_state(const ig_controller *controller)
{
    return controller ? controller->button_state : 0;
}

int ig_controller_get_button_state(const ig_controller *controller,
                                   ig_controller_button button)
{
    if (!controller || button >= IG_CONTROLLER_BUTTON_MAX) {
        return 0;
    }
    return ((controller->button_state >> button) & 1u) == 1u;
}

float ig_controller_get_button_pressure(const ig_controller *controller,
                                        ig_controller_button button)
{
    if (!controller || button >= IG_CONTROLLER_BUTTON_MAX) {
        return 0.0f;
    }
    return controller->pressure[button];
}

void ig_controller_set_button_state(ig_controller *controller, ig_controller_button button,
                                    int pressed)
{
    if (!controller || button >= IG_CONTROLLER_BUTTON_MAX) {
        return;
    }
    if (pressed) {
        controller->button_state |= (1u << button);
    } else {
        controller->button_state &= ~(1u << button);
    }
}

void ig_controller_set_button_pressure(ig_controller *controller, ig_controller_button button,
                                       float pressure)
{
    if (!controller || button >= IG_CONTROLLER_BUTTON_MAX) {
        return;
    }
    if (pressure < 0.0f) {
        pressure = 0.0f;
    } else if (pressure > 1.0f) {
        pressure = 1.0f;
    }
    controller->pressure[button] = pressure;
}

void ig_controller_get_joystick(const ig_controller *controller, unsigned int stick,
                                float *x, float *y)
{
    if (!x || !y) {
        return;
    }
    if (!controller || stick > 1) {
        *x = 0.0f;
        *y = 0.0f;
        return;
    }
    *x = controller->joystick[stick][0];
    *y = controller->joystick[stick][1];
}

void ig_controller_set_joystick(ig_controller *controller, unsigned int stick, float x, float y)
{
    if (!controller || stick > 1) {
        return;
    }
    controller->joystick[stick][0] = x;
    controller->joystick[stick][1] = y;
}
