
#ifndef SDL_waylandwebos_cursor_h_
#define SDL_waylandwebos_cursor_h_

#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_WAYLAND_WEBOS
#include "SDL_surface.h"

char WaylandWebOS_GetCursorSize();

SDL_Surface *WaylandWebOS_LoadCursorSurface(const char *type, const char *state);

#endif

#endif // SDL_waylandwebos_cursor_h_
