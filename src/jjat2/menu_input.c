/**
 * @file src/base/input.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <conf/input_list.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>

#include <string.h>

enum enMenuInputNames {
#define X_GPAD(...)
#define X_KEY(name, ...) enMenuInput_##name,
    X_MENU_BUTTON_LIST
    enMenuInput_count,
#undef X_KEY
#undef X_GPAD
};
typedef enum enMenuInputNames menuInputNames;

/** Set the v-buttons for menuing */
static err configureMenuButtons() {
    gfmInput *pInput;
    gfmRV rv;

    rv = gfm_getInput(&pInput, game.pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmInput_reset(pInput);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Create virtual keys for every input */
#define X_GPAD(...)
#define X_KEY(name, ...) \
    rv = gfm_addVirtualKey(&menuInput.name.handle, game.pCtx); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_MENU_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    setButtonList((button*)&menuInput, enMenuInput_count);

    return ERR_OK;
}

err setDefaultMenuInput() {
    gfmRV rv;

    ASSERT(configureMenuButtons() == ERR_OK, ERR_GFMERR);

    /* Bind every key */
#define X_GPAD(...)
#define X_KEY(name, key, ...) \
    rv = gfm_bindInput(game.pCtx, menuInput.name.handle, key); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_MENU_BUTTON_LIST
    X_ALTMENU_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    /* Bind the gamepad on any possible port */
#define X_GPAD(name, button, ...) \
    rv = gfm_bindGamepadInput(game.pCtx, menuInput.name.handle, button, port); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#define X_KEY(...)
    do {
        int port;
        for (port = 0; port < 4; port++) {
            X_MENU_BUTTON_LIST
            X_ALTMENU_BUTTON_LIST
        }
    } while (0);
#undef X_KEY
#undef X_GPAD

    return ERR_OK;
}
