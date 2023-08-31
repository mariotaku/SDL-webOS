#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_system.h"
#include "../../video/SDL_sysvideo.h"

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
    SDL_VideoDevice *dev = SDL_GetVideoDevice();

    if (dev == NULL) {
        SDL_SetError("Failed creating exported window: No video device");
        return NULL;
    }
    if (dev->WebOSCreateExportedWindow == NULL) {
        SDL_SetError("Failed creating exported window: Video device does not support exported windows");
        return NULL;
    }
    return dev->WebOSCreateExportedWindow(dev, type);
}

SDL_bool SDL_webOSSetExportedWindow(const char *windowId, SDL_Rect *src, SDL_Rect *dst)
{
    SDL_VideoDevice *dev = SDL_GetVideoDevice();

    if (dev == NULL) {
        SDL_SetError("Failed to set exported window: No video device");
        return SDL_FALSE;
    }
    if (dev->WebOSSetExportedWindow == NULL) {
        SDL_SetError("Failed to set exported window: Video device does not support exported windows");
        return SDL_FALSE;
    }
    return dev->WebOSSetExportedWindow(dev, windowId, src, dst);
}

SDL_bool SDL_webOSExportedSetCropRegion(const char *windowId, SDL_Rect *org, SDL_Rect *src, SDL_Rect *dst)
{
    SDL_VideoDevice *dev = SDL_GetVideoDevice();

    if (dev == NULL) {
        SDL_SetError("Failed to set crop region: No video device");
        return SDL_FALSE;
    }
    if (dev->WebOSExportedSetCropRegion == NULL) {
        SDL_SetError("Failed to set crop region: Video device does not support exported windows");
        return SDL_FALSE;
    }
    return dev->WebOSExportedSetCropRegion(dev, windowId, org, src, dst);
}

SDL_bool SDL_webOSExportedSetProperty(const char *windowId, const char *name, const char *value)
{
    SDL_VideoDevice *dev = SDL_GetVideoDevice();

    if (dev == NULL) {
        SDL_SetError("Failed to set property: No video device");
        return SDL_FALSE;
    }
    if (dev->WebOSExportedSetProperty == NULL) {
        SDL_SetError("Failed to set property: Video device does not support exported windows");
        return SDL_FALSE;
    }
    return dev->WebOSExportedSetProperty(dev, windowId, name, value);
}

void SDL_webOSDestroyExportedWindow(const char *windowId)
{
    SDL_VideoDevice *dev = SDL_GetVideoDevice();

    if (dev == NULL) {
        SDL_SetError("Failed to destroy exported window: No video device");
        return;
    }
    if (dev->WebOSDestroyExportedWindow == NULL) {
        SDL_SetError("Failed to destroy exported window: Video device does not support exported windows");
        return;
    }
    return dev->WebOSDestroyExportedWindow(dev, windowId);
}

SDL_bool SDL_webOSSetLSHandle(LSHandle *handle)
{

}

#endif // __WEBOS__
