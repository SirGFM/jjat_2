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

    /* TODO Bind every gamepad button */
#define X(name, key, button) \
    rv = gfm_bindGamepadInput(game.pCtx, input.name.handle, button, 0); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#  if 0
    X_BUTTON_LIST
#  endif
#undef X

    return ERR_OK;
}

