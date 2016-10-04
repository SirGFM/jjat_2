/**
 * @file src/base/input.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <conf/input_list.h>

#include <GFraMe/gfmError.h>

err initInput() {
    gfmRV rv;

    /* Create virtual keys for every input */
#define X(name, ...) \
    rv = gfm_addVirtualKey(&input.name.handle, game.pCtx); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
#undef X

    /* Bind every key */
#define X(name, key, ...) \
    rv = gfm_bindInput(game.pCtx, input.name.handle, key); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
#undef X

    /* Bind every gamepad button.
     *
     * Macro shenanigans is used to capture the number of arguments and, from
     * that, use only add code only for the keys with a default gamepad button.
     *
     * Note that this number of indirections is required so a "macro parameter"
     * is evaluated before resolving it.
     *
     * Source: https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s */
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, N, ...) N
#define PP_RSEQ_N() 3, 2, 1, 0

#define X(...)       X_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define X_(N, ...)   X_N(N, __VA_ARGS__)
#define X_N(_N, ...) X_ ## _N(__VA_ARGS__)
#define X_0()
#define X_1(name)
#define X_2(name, key)
#define X_3(name, key, button) \
    rv = gfm_bindGamepadInput(game.pCtx, input.name.handle, button, 0); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
#undef X_3
#undef X_2
#undef X_1
#undef X_0
#undef X_N
#undef X_
#undef X
#undef PP_RSEQ_N
#undef PP_ARG_N
#undef PP_NARG_
#undef PP_NARG

    return ERR_OK;
}

