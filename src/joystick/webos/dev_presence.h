#include "../../SDL_internal.h"

#ifndef SDL_webos_dev_presence_h_
#define SDL_webos_dev_presence_h_

typedef enum SDL_webOSDevicePresenceCheck
{
    SDL_WEBOS_DEVICE_PRESENCE_CHECK_HIDRAW,
    SDL_WEBOS_DEVICE_PRESENCE_CHECK_EVDEV,
    SDL_WEBOS_DEVICE_PRESENCE_CHECK_JS,
} SDL_webOSDevicePresenceCheck;

extern Uint32 SDL_webOSGetDevicePresenceFlags(SDL_webOSDevicePresenceCheck check);

extern SDL_bool SDL_webOSIsDeviceIndexPresent(Uint32 flags, int index);

#endif /* SDL_webos_dev_presence_h_ */
