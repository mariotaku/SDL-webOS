#include "SDL_waylandwebos_cursor.h"

/* Contributed by Mariotaku <mariotaku.lee@gmail.com> */

#ifdef SDL_VIDEO_DRIVER_WAYLAND_WEBOS
#include "../../core/webos/SDL_webos_libs.h"
#include "../../core/webos/SDL_webos_luna.h"
#include "../../core/webos/SDL_webos_json.h"

char WaylandWebOS_GetCursorSize()
{
    const char * uri = "luna://com.webos.settingsservice/getSystemSettings";
    const char * payload = "{\"keys\": [\"pointerSize\"],\"category\":\"option\",\"subscribe\":false}";
    char * response = NULL;
    jdomparser_ref parser = NULL;
    jvalue_ref parsed = NULL;
    char size = 'M';
    if (!SDL_webOSLunaServiceCallSync(uri, payload, 1, &response)) {
        return 'M';
    }
    if ((parsed = SDL_webOSJsonParse(response, &parser, 1))) {
        jvalue_ref settings;
        if (PBNJSON_jobject_get_exists(parsed, J_CSTR_TO_BUF("settings"), &settings)) {
            jvalue_ref pointerSize;
            if (PBNJSON_jobject_get_exists(settings, J_CSTR_TO_BUF("pointerSize"), &pointerSize)) {
                raw_buffer sizeStr = PBNJSON_jstring_get_fast(pointerSize);
                if (SDL_strncmp(sizeStr.m_str, "small", sizeStr.m_len) == 0) {
                    size = 'S';
                } else if (SDL_strncmp(sizeStr.m_str, "large", sizeStr.m_len) == 0) {
                    size = 'L';
                }
            }
        }
        PBNJSON_jdomparser_release(&parser);
    }
    SDL_free(response);
    return size;
}

SDL_Surface *WaylandWebOS_LoadCursorSurface(const char *type, const char *state)
{
    SDL_Surface *surface = NULL;
    char path[1024];
    char size = WaylandWebOS_GetCursorSize();
    snprintf(path, sizeof(path), "/usr/share/im/cursorType%ssz%cst%s.png", type, size, state);
    surface = IMG_IMG_Load(path);
    if (!surface) {
        snprintf(path, sizeof(path), "/usr/share/im/fhd/cursorType%ssz%cst%s.png", type, size, state);
        surface = IMG_IMG_Load(path);
    }
    return surface;
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND_WEBOS */
