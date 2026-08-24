#ifndef ALCHEMY_IG_SDL_CONTROLLER_H
#define ALCHEMY_IG_SDL_CONTROLLER_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>

#include "ig_controller.h"

/* The application owns SDL's process-wide event pump and forwards every event
 * here. Initialization enumerates devices already present; ADDED events admit
 * devices connected later. update() publishes a complete current snapshot.
 * Call shutdown() before the generic manager shutdown so SDL handles close. */
int ig_sdl_controller_initialize(ig_controller_manager *manager);
void ig_sdl_controller_handle_event(ig_controller_manager *manager, const SDL_Event *event);
void ig_sdl_controller_update(ig_controller_manager *manager);
void ig_sdl_controller_shutdown(ig_controller_manager *manager);
ig_controller_button ig_sdl_controller_button_to_ig(SDL_GamepadButton button);
void ig_sdl_controller_set_rumble(ig_controller *controller, int motor, float speed);

#endif
