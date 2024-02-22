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
#include "../SDL_internal.h"

#if SDL_VIDEO_DRIVER_WAYLAND_WEBOS
#include "SDL_scancode_webos_c.h"

SDL_Scancode SDL_GetWebOSScancode(int keycode)
{
    switch (keycode) {
    case 420:
        return SDL_SCANCODE_WEBOS_BACK;
    case 182:
        return SDL_SCANCODE_WEBOS_EXIT;
    case 406:
        return SDL_SCANCODE_WEBOS_RED;
    case 407:
        return SDL_SCANCODE_WEBOS_GREEN;
    case 408:
        return SDL_SCANCODE_WEBOS_YELLOW;
    case 409:
        return SDL_SCANCODE_WEBOS_BLUE;
    case 410:
        return SDL_SCANCODE_WEBOS_CH_UP;
    case 411:
        return SDL_SCANCODE_WEBOS_CH_DOWN;
    case 215:
        return SDL_SCANCODE_AUDIOPLAY;
    case 136:
        return SDL_SCANCODE_AUDIOSTOP;
    case 821:
        return SDL_SCANCODE_AUDIOPREV;
    case 822:
        return SDL_SCANCODE_AUDIONEXT;
    case 176:
        return SDL_SCANCODE_AUDIOREWIND;
    case 216:
        return SDL_SCANCODE_AUDIOFASTFORWARD;
    }
    return SDL_SCANCODE_UNKNOWN;
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND_WEBOS */

/* vi: set ts=4 sw=4 expandtab: */
