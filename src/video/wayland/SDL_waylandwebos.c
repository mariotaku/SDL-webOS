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

#include "../../core/webos/SDL_webos_libs.h"
#include "../../events/SDL_events_c.h"
#include "SDL_hints.h"
#include "SDL_waylandwebos.h"
#include "webos-client-protocol.h"

#ifndef WL_WEBOS_SHELL_SURFACE_STATE_ENUM
#define WL_WEBOS_SHELL_SURFACE_STATE_ENUM
/**
 * @ingroup iface_wl_webos_shell_surface
 * the state of the surface
 *
 * The state provides info to the client on how the compositor has placed
 * the surface.
 *
 * The default state will indicate to the client that it is windowed. The
 * "position_changed" event will tell the position in screen coordinates
 * when the surface is in this state.
 */
enum wl_webos_shell_surface_state {
    WL_WEBOS_SHELL_SURFACE_STATE_DEFAULT = 0,
    WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED = 1,
    WL_WEBOS_SHELL_SURFACE_STATE_MAXIMIZED = 2,
    WL_WEBOS_SHELL_SURFACE_STATE_FULLSCREEN = 3,
};
#endif /* WL_WEBOS_SHELL_SURFACE_STATE_ENUM */

static void webos_shell_handle_state(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state);

static void webos_shell_handle_position(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, int32_t x, int32_t y);

static void webos_shell_handle_close(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface);

static void webos_shell_handle_exposed(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, struct wl_array *rectangles);

static void webos_shell_handle_state_about_to_change(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state);

const static struct wl_webos_shell_surface_listener webos_shell_surface_listener = {
    .state_changed = webos_shell_handle_state,
    .position_changed = webos_shell_handle_position,
    .close = webos_shell_handle_close,
    .exposed = webos_shell_handle_exposed,
    .state_about_to_change = webos_shell_handle_state_about_to_change,
};

int WaylandWebOS_SetupSurface(SDL_WindowData *data)
{
    const char *appId;
    const char *hintValue;
    appId = SDL_getenv("APPID");
    if (appId == NULL) {
        return SDL_SetError("APPID environment variable is not set");
    }
    wl_webos_shell_surface_add_listener(data->shell_surface.webos, &webos_shell_surface_listener, data);
    wl_webos_shell_surface_set_property(data->shell_surface.webos, "appId", appId);
    if (SDL_GetHintBoolean(SDL_HINT_WEBOS_ACCESS_POLICY_KEYS_BACK, SDL_FALSE)) {
        wl_webos_shell_surface_set_property(data->shell_surface.webos, "_WEBOS_ACCESS_POLICY_KEYS_BACK", "true");
    }
    if (SDL_GetHintBoolean(SDL_HINT_WEBOS_ACCESS_POLICY_KEYS_EXIT, SDL_FALSE)) {
        wl_webos_shell_surface_set_property(data->shell_surface.webos, "_WEBOS_ACCESS_POLICY_KEYS_EXIT", "true");
    }
    if ((hintValue = SDL_GetHint(SDL_HINT_WEBOS_CURSOR_SLEEP_TIME)) != NULL) {
        data->webos_cursor_sleep_time = strtol(hintValue, NULL, 10);
    } else {
        data->webos_cursor_sleep_time = 300000;
    }
    wl_webos_shell_surface_set_property(data->shell_surface.webos, "_WEBOS_ACCESS_POLICY_FORCESTRETCH", "true");
    return 0;
}

static void webos_shell_handle_state(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state)
{
    SDL_WindowData *d = data;
    if (state == WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED) {
        HELPERS_HProcessAppState(3);
        SDL_SendWindowEvent(d->sdlwindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
        SDL_SendAppEvent(SDL_APP_WILLENTERBACKGROUND);
        SDL_SendAppEvent(SDL_APP_DIDENTERBACKGROUND);
    } else if (state == WL_WEBOS_SHELL_SURFACE_STATE_FULLSCREEN) {
        if (d->webos_shell_state < WL_WEBOS_SHELL_SURFACE_STATE_MAXIMIZED) {
            SDL_SendWindowEvent(d->sdlwindow, SDL_WINDOWEVENT_RESTORED, 0, 0);
            SDL_SendAppEvent(SDL_APP_WILLENTERFOREGROUND);
            SDL_SendAppEvent(SDL_APP_DIDENTERFOREGROUND);
            HELPERS_HProcessAppState(0);
        }
    }
    d->webos_shell_state = state;
}

static void webos_shell_handle_position(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, int32_t x, int32_t y)
{
    SDL_WindowData *d = data;
    SDL_SendWindowEvent(d->sdlwindow, SDL_WINDOWEVENT_MOVED, x, y);
}

static void webos_shell_handle_close(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface)
{
    SDL_WindowData *d = data;
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    SDL_SendWindowEvent(d->sdlwindow, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

static void webos_shell_handle_exposed(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, struct wl_array *rectangles)
{
}

static void webos_shell_handle_state_about_to_change(void *data, struct wl_webos_shell_surface *wl_webos_shell_surface, uint32_t state)
{
    if (state != WL_WEBOS_SHELL_SURFACE_STATE_MINIMIZED) {
        return;
    }
    SDL_SendAppEvent(SDL_APP_WILLENTERBACKGROUND);
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND_WEBOS */
