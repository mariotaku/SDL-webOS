#include "../../SDL_internal.h"

#ifdef __WEBOS__
#include "SDL_mutex.h"
#include "SDL_webos_libs.h"
#include "SDL_webos_luna.h"

struct HContextSync
{
    union
    {
        HContext ctx;
        __attribute__((unused)) unsigned char placeholder[128];
    } base;
    SDL_mutex *mutex;
    SDL_cond *cond;
    SDL_bool finished;
    char **output;
};

static int callback(LSHandle *sh, LSMessage *reply, void *ctx);

SDL_bool SDL_webOSLunaServiceCallSync(const char *uri, const char *payload, int pub, char **output)
{
    if (!HELPERS_HLunaServiceCall) {
        SDL_SetError("webOS libraries are not initialized");
        return SDL_FALSE;
    }
    struct HContextSync context = {
        .base.ctx = {
            .multiple = 0,
            .pub = pub ? 1 : 0,
            .callback = callback,
        }
    };

    context.mutex = SDL_CreateMutex();
    context.cond = SDL_CreateCond();
    context.output = output;

    if (HELPERS_HLunaServiceCall(uri, payload, &context.base.ctx) != 0) {
        SDL_DestroyMutex(context.mutex);
        SDL_DestroyCond(context.cond);
        return SDL_FALSE;
    }
    SDL_LockMutex(context.mutex);
    while (!context.finished) {
        SDL_CondWait(context.cond, context.mutex);
    }
    SDL_UnlockMutex(context.mutex);

    SDL_DestroyMutex(context.mutex);
    SDL_DestroyCond(context.cond);
    return SDL_TRUE;
}

static int callback(LSHandle *sh, LSMessage *reply, void *ctx)
{
    struct HContextSync *context = (struct HContextSync *)ctx;
    (void)sh;
    SDL_LockMutex(context->mutex);
    context->finished = SDL_TRUE;
    if (context->output) {
        *context->output = SDL_strdup(HELPERS_HLunaServiceMessage(reply));
    }
    SDL_CondSignal(context->cond);
    SDL_UnlockMutex(context->mutex);
    return 0;
}

#endif // __WEBOS__
