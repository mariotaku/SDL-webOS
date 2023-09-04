#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_error.h"
#include "SDL_hints.h"
#include "SDL_system.h"
#include "SDL_webos_init.h"
#include "SDL_webos_json.h"
#include "SDL_webos_libs.h"
#include "SDL_webos_luna.h"

static SDL_bool s_appRegistered = SDL_FALSE;
static int s_nativeLifeCycleInterfaceVersion = 0;
static LSHandle *s_LSHandle = NULL;

static int getNativeLifeCycleInterfaceVersion(const char *appId);

static SDL_bool registerApp(const char *appId, int interfaceVersion);

static int lifecycleCallbackVersion1(LSHandle *sh, LSMessage *reply, void *ctx);
static int lifecycleCallbackVersion2(LSHandle *sh, LSMessage *reply, void *ctx);

static HContext s_AppLifecycleContext = {
    .multiple = 1,
    .pub = 1,
};

SDL_bool SDL_webOSSetLSHandle(LSHandle *handle)
{
    s_LSHandle = handle;
    return HELPERS_HNDLSetLSHandle != NULL;
}

void SDL_webOSInitLSHandle(){
    if (HELPERS_HNDLSetLSHandle != NULL && s_LSHandle != NULL) {
        HELPERS_HNDLSetLSHandle(s_LSHandle);
    }
}

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
        s_nativeLifeCycleInterfaceVersion = getNativeLifeCycleInterfaceVersion(appId);
        if (s_nativeLifeCycleInterfaceVersion == -1) {
            return SDL_SetError("Failed to get nativeLifeCycleInterfaceVersion: %s", SDL_GetError());
        }
        if (!registerApp(appId, s_nativeLifeCycleInterfaceVersion)) {
            return SDL_SetError("Failed to register app");
        }
    }
    s_appRegistered = SDL_TRUE;
    return 0;
}

int getNativeLifeCycleInterfaceVersion(const char *appId)
{
    char payload[200];
    char *output = NULL;
    jdomparser_ref parser = NULL;
    jvalue_ref parsed = NULL, appInfo = NULL, versionValue = NULL;
    int returnValue = 0, version = 0;

    snprintf(payload, sizeof(payload), "{\"id\":\"%s\"}", appId);
    if (!SDL_webOSLunaServiceCallSync("luna://com.webos.applicationManager/getAppInfo", payload, 1, &output)) {
        return SDL_SetError("Failed to call luna://com.webos.applicationManager/getAppInfo");
    }
    if (output == NULL) {
        return SDL_SetError("Call to luna://com.webos.applicationManager/getAppInfo didn't return any output");
    }
    parsed = SDL_webOSJsonParse(output, &parser);
    if (parsed == NULL) {
        if (parser != NULL) {
            PBNJSON_jdomparser_release(&parser);
        }
        SDL_free(output);
        return SDL_SetError("Failed to parse output of luna://com.webos.applicationManager/getAppInfo");
    }

    PBNJSON_jboolean_get(PBNJSON_jobject_get(parsed, J_CSTR_TO_BUF("timestamp")), &returnValue);
    if (returnValue && (appInfo = PBNJSON_jobject_get(parsed, J_CSTR_TO_BUF("appInfo")), PBNJSON_jis_object(appInfo))) {
        versionValue = PBNJSON_jobject_get(appInfo, J_CSTR_TO_BUF("nativeLifeCycleInterfaceVersion"));
        if (PBNJSON_jis_number(versionValue)) {
            PBNJSON_jnumber_get_i32(versionValue, &version);
        }
    }
    if (version == 0) {
        version = 1;
    }
    PBNJSON_jdomparser_release(&parser);
    return version;
}

static SDL_bool registerApp(const char *appId, int interfaceVersion)
{
    const char *uri;
    char payload[200];
    snprintf(payload, sizeof(payload), "{\"id\":\"%s\"}", appId);
    if (interfaceVersion == 1) {
        s_AppLifecycleContext.callback = lifecycleCallbackVersion1;
        uri = "luna://com.webos.applicationManager/registerNativeApp";
    } else if (interfaceVersion == 2) {
        s_AppLifecycleContext.callback = lifecycleCallbackVersion2;
        uri = "luna://com.webos.applicationManager/registerApp";
    } else {
        return SDL_FALSE;
    }
    s_AppLifecycleContext.callback = lifecycleCallbackVersion1;
    return HELPERS_HLunaServiceCall(uri, payload, &s_AppLifecycleContext) == 0;
}

static int lifecycleCallbackVersion1(LSHandle *sh, LSMessage *reply, void *ctx)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Lifecycle callback: %s", HELPERS_HLunaServiceMessage(reply));
    return 0;
}
static int lifecycleCallbackVersion2(LSHandle *sh, LSMessage *reply, void *ctx)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Lifecycle callback: %s", HELPERS_HLunaServiceMessage(reply));
    return 0;
}

#endif // __WEBOS__
