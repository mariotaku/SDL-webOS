#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_webos_json.h"

jvalue_ref SDL_webOSJsonParse(const char *json, jdomparser_ref *parser)
{
    JSchemaInfo schemaInfo;
    PBNJSON_jschema_info_init(&schemaInfo, PBNJSON_jschema_all(), NULL, NULL);
    *parser = PBNJSON_jdomparser_create(&schemaInfo, 0);
    PBNJSON_jdomparser_feed(*parser, json, (int)strlen(json));
    PBNJSON_jdomparser_end(*parser);
    return PBNJSON_jdomparser_get_result(*parser);
}

#endif // __WEBOS__
