#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_webos_json.h"

jvalue_ref SDL_webOSJsonParse(const char *json, jdomparser_ref *parser, SDL_bool failOnNegative)
{
    jvalue_ref result;
    jdomparser_ref new_parser = NULL;
    JSchemaInfo schemaInfo;
    PBNJSON_jschema_info_init(&schemaInfo, PBNJSON_jschema_all(), NULL, NULL);
    new_parser = PBNJSON_jdomparser_create(&schemaInfo, 0);
    if (new_parser == NULL) {
        return NULL;
    }
    PBNJSON_jdomparser_feed(new_parser, json, (int)strlen(json));
    PBNJSON_jdomparser_end(new_parser);
    result = PBNJSON_jdomparser_get_result(new_parser);
    if (result == NULL) {
        PBNJSON_jdomparser_release(&new_parser);
        return NULL;
    }
    if (failOnNegative) {
        int returnValue = 0;
        PBNJSON_jboolean_get(PBNJSON_jobject_get(result, J_CSTR_TO_BUF("returnValue")), &returnValue);
        if (!returnValue) {
            PBNJSON_jdomparser_release(&new_parser);
            return NULL;
        }
    }
    *parser = new_parser;
    return result;
}

#endif // __WEBOS__
