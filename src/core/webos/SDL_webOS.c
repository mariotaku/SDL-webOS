#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_system.h"

SDL_bool SDL_webOSCursorVisibility(SDL_bool visible)
{
    return SDL_FALSE;
}

SDL_bool SDL_webOSGetPanelResolution(int *width, int *height)
{
    *width = 1920;
    *height = 1080;
    return SDL_TRUE;
}

SDL_bool SDL_webOSGetRefreshRate(int *rate)
{
    *rate = 60;
    return SDL_TRUE;
}

const char *SDL_webOSCreateExportedWindow(SDL_webOSExportedWindowType type)
{
    return NULL;
}

SDL_bool SDL_webOSSetExportedWindow(const char *windowId, SDL_Rect *src, SDL_Rect *dst)
{
    return SDL_FALSE;
}

SDL_bool SDL_webOSExportedSetCropRegion(const char *windowId, SDL_Rect *org, SDL_Rect *src, SDL_Rect *dst)
{
    return SDL_FALSE;
}

SDL_bool SDL_webOSExportedSetProperty(const char *windowId, const char *name, const char *value)
{
    return SDL_FALSE;
}

void SDL_webOSDestroyExportedWindow(const char *windowId)
{

}

#endif // __WEBOS__
