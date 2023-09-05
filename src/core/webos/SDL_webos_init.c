#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "../../video/SDL_sysvideo.h"
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

static SDL_bool registerScreenSaverRequest(const char *appId);
static int screenSaverRequestCallback(LSHandle *sh, LSMessage *reply, void *ctx);

static HContext s_AppLifecycleContext = {
    .multiple = 1,
    .pub = 1,
};

static HContext s_ScreenSaverRequestContext = {
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
        if (!registerScreenSaverRequest(appId)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to register screen saver request");
        }
    }
    s_appRegistered = SDL_TRUE;
    return 0;
}

extern void SDL_webOSUnregisterApp() {
    if (!s_appRegistered || !HELPERS_HUnregisterServiceCallback) {
        return;
    }
    if (s_AppLifecycleContext.callback != NULL) {
        HELPERS_HUnregisterServiceCallback(&s_AppLifecycleContext);
        s_AppLifecycleContext.callback = NULL;
    }
    if (s_ScreenSaverRequestContext.callback != NULL) {
        HELPERS_HUnregisterServiceCallback(&s_ScreenSaverRequestContext);
        if (s_ScreenSaverRequestContext.userdata) {
            SDL_free(s_ScreenSaverRequestContext.userdata);
        }
        s_ScreenSaverRequestContext.callback = NULL;
    }
    s_appRegistered = SDL_FALSE;
}

int getNativeLifeCycleInterfaceVersion(const char *appId)
{
    char payload[200];
    char *output = NULL;
    jdomparser_ref parser = NULL;
    jvalue_ref parsed = NULL, appInfo = NULL, versionValue = NULL;
    int version = 0;

    snprintf(payload, sizeof(payload), "{\"id\":\"%s\"}", appId);
    if (!SDL_webOSLunaServiceCallSync("luna://com.webos.applicationManager/getAppInfo", payload, 1, &output)) {
        return SDL_SetError("Failed to call luna://com.webos.applicationManager/getAppInfo");
    }
    if (output == NULL) {
        return SDL_SetError("Call to luna://com.webos.applicationManager/getAppInfo didn't return any output");
    }
    parsed = SDL_webOSJsonParse(output, &parser, 1);
    if (parsed == NULL) {
        SDL_free(output);
        return SDL_SetError("Failed to parse output of luna://com.webos.applicationManager/getAppInfo");
    }

    if (appInfo = PBNJSON_jobject_get(parsed, J_CSTR_TO_BUF("appInfo")), PBNJSON_jis_object(appInfo)) {
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

static SDL_bool registerScreenSaverRequest(const char *appId)
{
    jvalue_ref payload;
    char *client_name;
    SDL_bool result;

    client_name = SDL_malloc(64);
    SDL_snprintf(client_name, 64, "%s.wakelock", appId);
    payload = PBNJSON_jobject_create_var(
        PBNJSON_jkeyval(J_CSTR_TO_JVAL("subscribe"), PBNJSON_jboolean_create(1)),
        PBNJSON_jkeyval(J_CSTR_TO_JVAL("clientName"), PBNJSON_j_cstr_to_jval(client_name)),
        NULL);
    s_ScreenSaverRequestContext.userdata = client_name;
    s_ScreenSaverRequestContext.callback = screenSaverRequestCallback;

    result = HELPERS_HLunaServiceCall("luna://com.webos.service.tvpower/power/registerScreenSaverRequest",
                                      PBNJSON_jvalue_stringify(payload), &s_ScreenSaverRequestContext) == 0;

    PBNJSON_j_release(&payload);
    return result;
}

static int screenSaverRequestCallback(LSHandle *sh, LSMessage *reply, void *ctx)
{
    SDL_VideoDevice *device;
    const char *message;
    jdomparser_ref parser = NULL;
    jvalue_ref parsed = NULL;
    jvalue_ref timestamp = NULL;
    jvalue_ref response;

    (void) sh;
    (void) ctx;

    device = SDL_GetVideoDevice();
    if (device == NULL || !device->suspend_screensaver) {
        return 0;
    }

    message = HELPERS_HLunaServiceMessage(reply);
    if ((parsed = SDL_webOSJsonParse(message, &parser, 1)) == NULL) {
        return -1;
    }

    timestamp = PBNJSON_jobject_get(parsed, J_CSTR_TO_BUF("timestamp"));

    if (PBNJSON_jis_null(timestamp)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Skip invalid screensaver request (no timestamp)");
        PBNJSON_jdomparser_release(&parser);
        return 0;
    }
    response = PBNJSON_jobject_create_var(
        PBNJSON_jkeyval(J_CSTR_TO_JVAL("clientName"), PBNJSON_j_cstr_to_jval((const char *)ctx)),
        PBNJSON_jkeyval(J_CSTR_TO_JVAL("ack"), PBNJSON_jboolean_create(0)),
        PBNJSON_jkeyval(J_CSTR_TO_JVAL("timestamp"), timestamp),
        NULL);

    SDL_webOSLunaServiceCallSync("luna://com.webos.service.tvpower/power/responseScreenSaverRequest",
                             PBNJSON_jvalue_stringify(response), 1, NULL);
    PBNJSON_j_release(&response);
    PBNJSON_jdomparser_release(&parser);
    return 0;
}

#endif // __WEBOS__
