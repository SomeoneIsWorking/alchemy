#ifndef ALCHEMY_IG_CONTROLLER_H
#define ALCHEMY_IG_CONTROLLER_H

#include <stdint.h>

#define IG_CONTROLLER_MAX_COUNT 4
#define IG_CONTROLLER_BUTTON_COUNT 32

typedef enum ig_controller_button {
    IG_CONTROLLER_BUTTON_SELECT = 0,
    IG_CONTROLLER_BUTTON_LEFT_STICK = 1,
    IG_CONTROLLER_BUTTON_RIGHT_STICK = 2,
    IG_CONTROLLER_BUTTON_START = 3,
    IG_CONTROLLER_BUTTON_DPAD_UP = 4,
    IG_CONTROLLER_BUTTON_DPAD_RIGHT = 5,
    IG_CONTROLLER_BUTTON_DPAD_DOWN = 6,
    IG_CONTROLLER_BUTTON_DPAD_LEFT = 7,
    IG_CONTROLLER_BUTTON_LEFT_TRIGGER = 8,
    IG_CONTROLLER_BUTTON_RIGHT_TRIGGER = 9,
    IG_CONTROLLER_BUTTON_LEFT_SHOULDER = 10,
    IG_CONTROLLER_BUTTON_RIGHT_SHOULDER = 11,
    IG_CONTROLLER_BUTTON_FACE_UP = 12,
    IG_CONTROLLER_BUTTON_FACE_RIGHT = 13,
    IG_CONTROLLER_BUTTON_FACE_DOWN = 14,
    IG_CONTROLLER_BUTTON_FACE_LEFT = 15,
    IG_CONTROLLER_BUTTON_16 = 16,
    IG_CONTROLLER_BUTTON_17 = 17,
    IG_CONTROLLER_BUTTON_18 = 18,
    IG_CONTROLLER_BUTTON_19 = 19,
    IG_CONTROLLER_BUTTON_20 = 20,
    IG_CONTROLLER_BUTTON_21 = 21,
    IG_CONTROLLER_BUTTON_22 = 22,
    IG_CONTROLLER_BUTTON_23 = 23,
    IG_CONTROLLER_BUTTON_24 = 24,
    IG_CONTROLLER_BUTTON_25 = 25,
    IG_CONTROLLER_BUTTON_26 = 26,
    IG_CONTROLLER_BUTTON_27 = 27,
    IG_CONTROLLER_BUTTON_28 = 28,
    IG_CONTROLLER_BUTTON_29 = 29,
    IG_CONTROLLER_BUTTON_30 = 30,
    IG_CONTROLLER_BUTTON_31 = 31,
    IG_CONTROLLER_BUTTON_MAX = 32,
    IG_CONTROLLER_BUTTON_UNMAPPED = 0xffff
} ig_controller_button;

/* Alchemy's extracted Win32 backend exposes this controller-type vocabulary.
 * It is retained as engine semantics, not asserted as a guest object layout. */
typedef enum ig_controller_type {
    IG_CONTROLLER_TYPE_UNKNOWN = 0,
    IG_CONTROLLER_TYPE_PS2_PELICAN_16BUTTONS,
    IG_CONTROLLER_TYPE_PS2_SMARTJOY_12BUTTONS,
    IG_CONTROLLER_TYPE_PS2_XSERIES_12BUTTONS,
    IG_CONTROLLER_TYPE_PS2_ELECOM_12BUTTONS_POV,
    IG_CONTROLLER_TYPE_PS2_ELECOM_16BUTTONS,
    IG_CONTROLLER_TYPE_PS2_SANWA_16BUTTONS,
    IG_CONTROLLER_TYPE_XBOX360_MICROSOFT_10BUTTONS_POV,
    IG_CONTROLLER_TYPE_COUNT
} ig_controller_type;

typedef struct ig_controller {
    uint16_t id;
    uint32_t button_state;
    float pressure[IG_CONTROLLER_BUTTON_COUNT];
    float joystick[2][2];
    uint8_t connected;
    ig_controller_type type;
    uint8_t is_console;
    void *backend_handle;
    uint32_t device_id;
} ig_controller;

typedef struct ig_controller_manager ig_controller_manager;

typedef struct ig_controller_device {
    uint32_t device_id;
    void *backend_handle;
    ig_controller_type type;
    uint8_t is_console;
} ig_controller_device;

typedef void (*ig_controller_connection_cb)(ig_controller_manager *manager,
                                             ig_controller *controller);
typedef void (*ig_controller_disconnection_cb)(ig_controller_manager *manager,
                                                ig_controller *controller);

struct ig_controller_manager {
    ig_controller controllers[IG_CONTROLLER_MAX_COUNT];
    int count;
    ig_controller_connection_cb on_connect;
    ig_controller_disconnection_cb on_disconnect;
    void *userdata;
};

void ig_controller_manager_init(ig_controller_manager *manager);
void ig_controller_manager_shutdown(ig_controller_manager *manager);
void ig_controller_manager_set_callbacks(ig_controller_manager *manager,
                                         ig_controller_connection_cb on_connect,
                                         ig_controller_disconnection_cb on_disconnect,
                                         void *userdata);
int ig_controller_manager_get_count(const ig_controller_manager *manager);
ig_controller *ig_controller_manager_get(const ig_controller_manager *manager, int index);
ig_controller *ig_controller_manager_connect(ig_controller_manager *manager,
                                             const ig_controller_device *device);
void ig_controller_manager_disconnect(ig_controller_manager *manager, uint32_t device_id);
ig_controller *ig_controller_manager_find(const ig_controller_manager *manager,
                                          uint32_t device_id);

int ig_controller_is_connected(const ig_controller *controller);
uint32_t ig_controller_get_buttons_state(const ig_controller *controller);
int ig_controller_get_button_state(const ig_controller *controller,
                                   ig_controller_button button);
float ig_controller_get_button_pressure(const ig_controller *controller,
                                        ig_controller_button button);
void ig_controller_set_button_state(ig_controller *controller, ig_controller_button button,
                                    int pressed);
void ig_controller_set_button_pressure(ig_controller *controller, ig_controller_button button,
                                       float pressure);
void ig_controller_get_joystick(const ig_controller *controller, unsigned int stick,
                                float *x, float *y);
void ig_controller_set_joystick(ig_controller *controller, unsigned int stick, float x, float y);

#endif
