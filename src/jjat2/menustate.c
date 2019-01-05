#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/menu_ctx.h>
#include <conf/game.h>
#include <jjat2/menu_input.h>
#include <jjat2/menus.h>
#include <jjat2/menustate.h>
#include <jjat2/util.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmText.h>
#include <string.h>

/** Initialize the menustate. */
err initMenustate() {
    gfmRV rv;

    rv = gfmText_getNew(&menustate.pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_init(menustate.pText, 0/*x*/, 0/*y*/, MAX_WIDTH, 1/*maxLines*/
        , 0/*delay*/, 0/*bindToWorld*/, gfx.pSset8x8, DEF_ACTIVE_TILE);
    menustate.activeOffset = DEF_ACTIVE_TILE;
    menustate.inactiveOffset = DEF_INACTIVE_TILE;

    return ERR_OK;
}

/** If the menustate has been initialized, properly free it up. */
void freeMenustate() {
    if (menustate.pText != 0)
        gfmText_free(&menustate.pText);
}

/** Setup the loadstate so it may start to be executed */
err loadMenustate() {
    err erv;

    erv = setDefaultMenuInput();
    ASSERT(erv == ERR_OK, erv);

    return loadMainmenu(&menustate);
}

err updateMenustate() {
    return updateMenuCtx(&menustate);
}

/** Draw the menustate */
err drawMenustate() {
    return drawMenuCtx(&menustate);
}
