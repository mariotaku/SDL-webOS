/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* Contributed by Mariotaku <mariotaku.lee@gmail.com> */

#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_WAYLAND_WEBOS

#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keysym_to_scancode_c.h"
#include "SDL_waylandevents_c.h"
#include "../SDL_sysvideo.h"
#include "SDL_hints.h"
#include "SDL_waylandwebos_osk.h"
#include "webos-client-protocol.h"

struct webos_osk_data
{
    struct text_model *text_model;
    uint32_t index;
    uint32_t length;
    uint32_t state;
};

static void osk_commit_string(void *data, struct text_model *text_model, uint32_t serial, const char *text);
static void osk_preedit_string(void *data, struct text_model *text_model, uint32_t serial, const char *text, const char *commit);
static void osk_delete_surrounding_text(void *data, struct text_model *text_model, uint32_t serial, int32_t index, uint32_t length);
static void osk_cursor_position(void *data, struct text_model *text_model, uint32_t serial, int32_t index, int32_t anchor);
static void osk_preedit_styling(void *data, struct text_model *text_model, uint32_t serial, uint32_t index, uint32_t length, uint32_t style);
static void osk_preedit_cursor(void *data, struct text_model *text_model, uint32_t serial, int32_t index);
static void osk_modifiers_map(void *data, struct text_model *text_model, struct wl_array *map);
static void osk_keysym(void *data, struct text_model *text_model, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers);
static void osk_enter(void *data, struct text_model *text_model, struct wl_surface *surface);
static void osk_leave(void *data, struct text_model *text_model);
static void osk_input_panel_state(void *data, struct text_model *text_model, uint32_t state);
static void input_panel_rect(void *data, struct text_model *text_model, int32_t x, int32_t y, uint32_t width, uint32_t height);

static const struct text_model_listener osk_text_model_listener = {
    .commit_string = osk_commit_string,
    .preedit_string = osk_preedit_string,
    .delete_surrounding_text = osk_delete_surrounding_text,
    .cursor_position = osk_cursor_position,
    .preedit_styling = osk_preedit_styling,
    .preedit_cursor = osk_preedit_cursor,
    .modifiers_map = osk_modifiers_map,
    .keysym = osk_keysym,
    .enter = osk_enter,
    .leave = osk_leave,
    .input_panel_state = osk_input_panel_state,
    .input_panel_rect = input_panel_rect,
};

static void ensureTextModel(SDL_VideoData *waylandData);

SDL_bool WaylandWebOS_HasScreenKeyboardSupport(_THIS)
{
    SDL_VideoData * waylandData = _this->driverdata;
    if (_this->windows == NULL) {
        // The first window is not created yet, so we assume the system has screen keyboard support
        return SDL_TRUE;
    }
    return waylandData->text_model_factory != NULL;
}

void WaylandWebOS_ShowScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_WindowData * windowData = window->driverdata;
    SDL_VideoData *waylandData = windowData->waylandData;
    struct SDL_WaylandInput *input = waylandData->input;
    struct webos_osk_data *osk_data;

    if (waylandData->text_model_factory == NULL) {
        // Screen keyboard is not supported
        return;
    }

    ensureTextModel(waylandData);
    osk_data = waylandData->webos_screen_keyboard_data;
    if (osk_data) {
        static uint32_t text_model_serial = 0;
        text_model_activate(osk_data->text_model, ++text_model_serial, input->seat, windowData->surface);
        text_model_set_content_type(osk_data->text_model, 7, 0);
        window->flags |= SDL_WINDOW_INPUT_FOCUS;
    }
}

void WaylandWebOS_HideScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_WindowData * windowData = window->driverdata;
    SDL_VideoData *waylandData = windowData->waylandData;
    struct SDL_WaylandInput *input = waylandData->input;
    struct webos_osk_data *osk_data = waylandData->webos_screen_keyboard_data;
    if (osk_data == NULL || osk_data->text_model == NULL || input->seat == NULL) {
        return;
    }
    text_model_deactivate(osk_data->text_model, input->seat);
    osk_data->text_model = NULL;
    osk_data->state = 0;
}

SDL_bool WaylandWebOS_IsScreenKeyboardShown(_THIS, SDL_Window *window)
{
    SDL_WindowData * windowData = window->driverdata;
    SDL_VideoData *waylandData = windowData->waylandData;
    struct webos_osk_data *oskData = waylandData->webos_screen_keyboard_data;
    if (oskData != NULL && SDL_IsTextInputActive()) {
        return oskData->state != 0;
    }
    return SDL_FALSE;
}

static void ensureTextModel(SDL_VideoData *waylandData)
{
    struct webos_osk_data *osk_data = waylandData->webos_screen_keyboard_data;
    if (osk_data == NULL) {
        osk_data = SDL_calloc(1, sizeof(struct webos_osk_data));
        waylandData->webos_screen_keyboard_data = osk_data;
    } else if (osk_data->text_model != NULL) {
        return;
    }
    osk_data->text_model = text_model_factory_create_text_model(waylandData->text_model_factory);
    text_model_add_listener(osk_data->text_model, &osk_text_model_listener, osk_data);
    WAYLAND_wl_display_dispatch(waylandData->display);
    WAYLAND_wl_display_flush(waylandData->display);
}

void osk_commit_string(void *data, struct text_model *text_model, uint32_t serial, const char *text)
{
    SDL_SendKeyboardText(text);
}

void osk_preedit_string(void *data, struct text_model *text_model, uint32_t serial, const char *text, const char *commit) {
    struct webos_osk_data *osk_data = data;
    if (osk_data->length != 0) {
        SDL_SendEditingText(text, 0, (int) SDL_strlen(text));
    }
}

void osk_delete_surrounding_text(void *data, struct text_model *text_model, uint32_t serial, int32_t index, uint32_t length)
{
    if ((int32_t) length == -1) {
        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_CLEAR);
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_CLEAR);
    } else {
        SDL_SendEditingText("", index, (int) length);
    }
}
void osk_cursor_position(void *data, struct text_model *text_model, uint32_t serial, int32_t index, int32_t anchor) {}

void osk_preedit_styling(void *data, struct text_model *text_model, uint32_t serial, uint32_t index, uint32_t length, uint32_t style)
{
    struct webos_osk_data *osk_data = data;
    osk_data->index = index;
    osk_data->length = length;
}

void osk_preedit_cursor(void *data, struct text_model *text_model, uint32_t serial, int32_t index)
{
}

void osk_modifiers_map(void *data, struct text_model *text_model, struct wl_array *map)
{

}

void osk_keysym(void *data, struct text_model *text_model, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
    SDL_Scancode scancode = SDL_GetScancodeFromKeySym(sym, 0);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        return;
    }
    if (SDL_GetHintBoolean(SDL_HINT_RETURN_KEY_HIDES_IME, SDL_FALSE) && scancode == SDL_SCANCODE_RETURN) {
        SDL_StopTextInput();
        return;
    }
    SDL_SendKeyboardKey(state, scancode);
}

void osk_enter(void *data, struct text_model *text_model, struct wl_surface *surface)
{
    // No-op
}

void osk_leave(void *data, struct text_model *text_model)
{
    struct webos_osk_data *osk_data = data;
    osk_data->text_model = NULL;
    SDL_StopTextInput();
}

void osk_input_panel_state(void *data, struct text_model *text_model, uint32_t state)
{
    struct webos_osk_data *osk_data = data;
    osk_data->state = state;
    if (state == 0) {
        SDL_StopTextInput();
    }
}

void input_panel_rect(void *data, struct text_model *text_model, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
}
#endif /* SDL_VIDEO_DRIVER_WAYLAND_WEBOS */
