/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* *INDENT-OFF* */ /* clang-format off */

#ifndef SDL_webos_helpers_types_
#define SDL_webos_helpers_types_
typedef struct LSHandle LSHandle;
typedef struct LSMessage LSMessage;
typedef unsigned long LSMessageToken;

typedef int (*LSFilterFunc) (LSHandle *sh, LSMessage *reply, void *ctx);

typedef struct HContext {
    /**
     * @brief Callback function called on incoming message.
     */
    LSFilterFunc callback;
    void* userdata;
    void* unknown;
    /**
     * @brief Whether the call is multiple (like subscription), or one-shot.
     */
    int multiple;
    /**
     * @brief Whether the call is a public call or private call
     */
    int pub;
    LSMessageToken ret_token;
} HContext;

typedef struct HJson HJson;
typedef struct _HString HString;

#endif // SDL_webos_helpers_types_

#ifndef SDL_HELPERS_SYM
#define SDL_HELPERS_SYM(rc,fn,params)
#endif

SDL_HELPERS_SYM(int, HLunaServiceCall, (const char *uri, const char *payload, HContext *context))
SDL_HELPERS_SYM(const char*, HLunaServiceMessage, (LSMessage *msg))
SDL_HELPERS_SYM(HJson*, HJson_create, (const char *jsonStr))
SDL_HELPERS_SYM(void, HJson_free, (HJson *hjson))
SDL_HELPERS_SYM(int, HJson_valueExists, (HJson *hjson, const char *valueName))
SDL_HELPERS_SYM(int, HJson_getInt, (HJson *hjson, const char *valueName))
SDL_HELPERS_SYM(int, HJson_getBool, (HJson *hjson, const char *valueName))

#undef SDL_HELPERS_SYM

/* *INDENT-ON* */ /* clang-format on */

/* vi: set ts=4 sw=4 expandtab: */
