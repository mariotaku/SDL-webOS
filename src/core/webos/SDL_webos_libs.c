#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_loadso.h"
#include "SDL_webos_libs.h"

#define SDL_HELPERS_SYM(rc, fn, params)     SDL_DYNHELPERSFN_##fn HELPERS_##fn;
#define SDL_HELPERS_SYM_OPT(rc, fn, params) SDL_DYNHELPERSFN_##fn HELPERS_##fn;
#include "SDL_webos_helpers_sym.h"

#define SDL_PBNJSON_SYM(rc, fn, params)     SDL_DYNPBNJSONFN_##fn PBNJSON_##fn;
#include "SDL_webos_pbnjson_sym.h"

#define SDL_IMG_SYM(rc, fn, params)     SDL_DYNIMGFN_##fn IMG_##fn;
#include "SDL_webos_img_sym.h"

static void *WebOSGetSym(void *object, const char *name, int required, int *valid);
static void *LibHelpersHandle = NULL;
static void *LibPbnjsonHandle = NULL;
static void *LibImgHandle = NULL;

static int LoadHelpers();
static int LoadPbnjson();
static int LoadImg();

int SDL_webOSLoadLibraries()
{
    int ret;
    if ((ret = LoadHelpers()) != 0) {
        return ret;
    }
    if ((ret = LoadPbnjson()) != 0) {
        return ret;
    }
    if ((ret = LoadImg()) != 0) {
        return ret;
    }
    return ret;
}

void SDL_webOSUnloadLibraries()
{
#define SDL_HELPERS_SYM(rc, fn, params) HELPERS_##fn = NULL;
#include "SDL_webos_helpers_sym.h"
    if (LibHelpersHandle != NULL) {
        SDL_UnloadObject(LibHelpersHandle);
    }
    LibHelpersHandle = NULL;

#define SDL_PBNJSON_SYM(rc, fn, params) PBNJSON_##fn = NULL;
#include "SDL_webos_pbnjson_sym.h"
    if (LibPbnjsonHandle != NULL) {
        SDL_UnloadObject(LibPbnjsonHandle);
    }
    LibPbnjsonHandle = NULL;
#define SDL_IMG_SYM(rc, fn, params) IMG_##fn = NULL;
#include "SDL_webos_img_sym.h"
    if (LibImgHandle != NULL) {
        SDL_UnloadObject(LibImgHandle);
    }
    LibImgHandle = NULL;
}

static void *WebOSGetSym(void *object, const char *name, int required, int *valid)
{
    void *sym = SDL_LoadFunction(object, name);
    if (required && sym == NULL) {
        *valid = 0;
    }
    return sym;
}

int LoadHelpers()
{
    int valid = 1;
    LibHelpersHandle = SDL_LoadObject("libhelpers.so.2");
    if (LibHelpersHandle == NULL) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libhelpers");
    }
#define SDL_HELPERS_SYM(rc, fn, params)     HELPERS_##fn = (SDL_DYNHELPERSFN_##fn)WebOSGetSym(LibHelpersHandle, #fn, 1, &valid);
#define SDL_HELPERS_SYM_OPT(rc, fn, params) HELPERS_##fn = (SDL_DYNHELPERSFN_##fn)WebOSGetSym(LibHelpersHandle, #fn, 0, &valid);
#include "SDL_webos_helpers_sym.h"
    if (!valid) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libhelpers");
    }
    return 0;
}

int LoadPbnjson()
{
    int valid = 1;
    LibPbnjsonHandle = SDL_LoadObject("libpbnjson_c.so.2");
    if (LibPbnjsonHandle == NULL) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libpbnjson_c");
    }
#define SDL_PBNJSON_SYM(rc, fn, params) PBNJSON_##fn = (SDL_DYNPBNJSONFN_##fn)WebOSGetSym(LibPbnjsonHandle, #fn, 1, &valid);
#include "SDL_webos_pbnjson_sym.h"
    if (!valid) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libpbnjson_c");
    }
    return 0;
}

int LoadImg()
{
    int valid = 1;
    LibImgHandle = SDL_LoadObject("libSDL2_image-2.0.so.0");
    if (LibImgHandle == NULL) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libSDL2_image");
    }
#define SDL_IMG_SYM(rc, fn, params) IMG_##fn = (SDL_DYNIMGFN_##fn)WebOSGetSym(LibImgHandle, #fn, 1, &valid);
#include "SDL_webos_img_sym.h"
    if (!valid) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libSDL2_image");
    }
    return 0;
}

#endif // __WEBOS__
