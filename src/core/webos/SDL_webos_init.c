#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_hints.h"
#include "SDL_webos_init.h"
#include "SDL_webos_libs.h"
#include "SDL_webos_luna.h"

static SDL_bool s_appRegistered = SDL_FALSE;
static int s_nativeLifeCycleInterfaceVersion = 0;

static SDL_bool getAppInfo(const char *appId);

static SDL_bool registerApp(const char *appId, int interfaceVersion);

SDL_bool SDL_webOSAppRegistered()
{
    return s_appRegistered;
}

void SDL_webOSAppQuited()
{
    s_appRegistered = SDL_FALSE;
}

int SDL_webOSRegisterApp()
{
    if (s_appRegistered) {
        return 0;
    }
    if (SDL_GetHintBoolean(SDL_HINT_WEBOS_REGISTER_APP, SDL_TRUE)) {
        const char *appId = SDL_getenv("APPID");
        getAppInfo(appId);
        if (s_nativeLifeCycleInterfaceVersion == 1) {
            HELPERS_HLunaServiceCall("luna://com.webos.applicationManager/registerNativeApp", "{\"subscribe\":true}", NULL);
        }
    }
    s_appRegistered = SDL_TRUE;
    return 0;
}

SDL_bool getAppInfo(const char *appId)
{
    char payload[200];
    char *output = NULL;
    HJson *json;

    snprintf(payload, sizeof(payload), "{\"id\":\"%s\"}", appId);
    if (!SDL_webOSLunaServiceCallSync("luna://com.webos.applicationManager/getAppInfo", payload, 1, &output)) {
        return SDL_FALSE;
    }
    if (output == NULL) {
        return SDL_FALSE;
    }
    json = HELPERS_HJson_create(output);
    s_nativeLifeCycleInterfaceVersion = 1;
    HELPERS_HJson_free(json);
    return SDL_TRUE;
}

static SDL_bool registerApp(const char *appId, int interfaceVersion)
{
}
#endif // __WEBOS__
