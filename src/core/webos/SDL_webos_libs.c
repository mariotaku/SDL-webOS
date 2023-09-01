#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_loadso.h"
#include "SDL_webos_libs.h"

#define SDL_HELPERS_SYM(rc, fn, params, required) SDL_DYNHELPERSFN_##fn HELPERS_##fn;
#include "SDL_webos_helpers_sym.h"

static void *WebOSGetSym(void *object, const char *name, int required, int *valid);
static void *LibHelpersHandle = NULL;

int SDL_webOSLoadLibraries()
{
    int valid = 1;
    LibHelpersHandle = SDL_LoadObject("libhelpers.so.2");
    if (LibHelpersHandle == NULL) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libhelpers");
    }
#define SDL_HELPERS_SYM(rc, fn, params, required)     HELPERS_##fn = (SDL_DYNHELPERSFN_##fn)WebOSGetSym(LibHelpersHandle, #fn, required, &valid);
#include "SDL_webos_helpers_sym.h"
    if (!valid) {
        SDL_webOSUnloadLibraries();
        return SDL_SetError("Failed to load libhelpers");
    }
    return 0;
}

void SDL_webOSUnloadLibraries()
{
#define SDL_HELPERS_SYM(rc, fn, params, required) HELPERS_##fn = NULL;
#include "SDL_webos_helpers_sym.h"
    if (LibHelpersHandle != NULL) {
        SDL_UnloadObject(LibHelpersHandle);
    }
    LibHelpersHandle = NULL;
}

static void *WebOSGetSym(void *object, const char *name, int required, int *valid)
{
    void *sym = SDL_LoadFunction(object, name);
    if (required && sym == NULL) {
        *valid = 0;
    }
    return sym;
}

#endif // __WEBOS__
