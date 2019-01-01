#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <conf/game.h>
#include <jjat2/menu_input.h>
#include <jjat2/menustate.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmText.h>
#include <string.h>

enum menuDir {
    md_none  = 0x00,
    md_left  = 0x01,
    md_right = 0x02,
    md_up    = 0x04,
    md_down  = 0x08,
};

#define PRESS_DELAY_MS 500
#define HOLD_DELAY_MS  125
#define MAX_WIDTH      (V_WIDTH / 8)
#define ACTIVE_TILE    0
#define INACTIVE_TILE    4032

#define getOptsSize(__OPTS__) (sizeof(__OPTS__) / sizeof(char*))

enum enOptions {
    OPT_NEWGAME = 0
  , OPT_EXIT
};

static const char *options[] = {
    [OPT_NEWGAME] "NEW GAME"
  , [OPT_EXIT] "EXIT"
};

/** Initialize the menustate. */
err initMenustate() {
    gfmRV rv;

    rv = gfmText_getNew(&menustate.pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_init(menustate.pText, 0/*x*/, 0/*y*/, MAX_WIDTH, 1/*maxLines*/
        , 0/*delay*/, 0/*bindToWorld*/, gfx.pSset8x8, ACTIVE_TILE);

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

    menustate.vopts = (char**)options;
    menustate.vcount = getOptsSize(options);

    menustate.dir = 0;
    menustate.delay = 0;
    menustate.vpos = 0;
    menustate.hpos = 0;
    menustate.hcount = 0;

    return erv;
}

static void updateCursorPosition() {
    int setHor = 0;

    switch (menustate.dir) {
    case md_left:
        menustate.hpos--;
        break;
    case md_right:
        menustate.hpos++;
        break;
    case md_up:
        menustate.vpos--;
        setHor = 1;
        break;
    case md_down:
        menustate.vpos++;
        setHor = 1;
        break;
    }

    if (setHor) {
        /* TODO Set the initial horizontal position based on the current item */
    }

    if (menustate.vpos < 0)
        menustate.vpos = menustate.vcount - 1;
    if (menustate.vpos >= menustate.vcount)
        menustate.vpos = 0;
    if (menustate.hpos < 0)
        menustate.hpos = menustate.hcount - 1;
    if (menustate.hpos >= menustate.hcount)
        menustate.hpos = 0;
}

static void handleCursorMovement() {
#define CHECK_CURSOR(__DIR__) \
    do { \
        if (DID_JUST_PRESS_MENU(__DIR__)) { \
            menustate.dir = md_ ## __DIR__; \
            menustate.delay = PRESS_DELAY_MS; \
            updateCursorPosition(); \
        } \
    } while (0)

    CHECK_CURSOR(up);
    CHECK_CURSOR(down);
    CHECK_CURSOR(left);
    CHECK_CURSOR(right);

#undef CHECK_CURSOR

    if (menustate.dir != md_none && menustate.delay > 0)
        menustate.delay -= game.elapsed;
    else if (menustate.dir != md_none && menustate.delay <= 0) {
        menustate.delay = HOLD_DELAY_MS;
        updateCursorPosition();
    }
}

/** Update the menustate */
err updateMenustate() {
    gfmRV rv;

    if (IS_MENU_RELEASED(up) && IS_MENU_RELEASED(down) &&
            IS_MENU_RELEASED(left) && IS_MENU_RELEASED(right))
        menustate.dir = md_none;

    handleCursorMovement();

    if (DID_JUST_PRESS_MENU(accept)) {
        // TODO Play accept SFX

        switch (menustate.vpos) {
        case OPT_NEWGAME:
            game.nextState = ST_PLAYSTATE;
            break;
        case OPT_EXIT:
            rv = gfm_setQuitFlag(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        }
    }

    return ERR_OK;
}

static err drawText(char *text, int len, int y, int active) {
    int tile, x;
    gfmRV rv;

    if (active)
        tile = ACTIVE_TILE;
    else
        tile = INACTIVE_TILE;

    x = (V_WIDTH / 8 - len) / 2;

    rv = gfmText_setPosition(menustate.pText, x * 8, y * 8);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setSpriteset(menustate.pText, gfx.pSset8x8, tile);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setText(menustate.pText, text, len, 0/*doCopy*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_forceFinish(menustate.pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_draw(menustate.pText, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Draw the menustate */
err drawMenustate() {
    err erv;
    int i, y;

    gfmDebug_printf(game.pCtx, 0, 128, "  DIR: %i", menustate.dir);
    gfmDebug_printf(game.pCtx, 0, 136, "DELAY: %i", menustate.delay);
    gfmDebug_printf(game.pCtx, 0, 144, " VPOS: %i", menustate.vpos);
    gfmDebug_printf(game.pCtx, 0, 152, " HPOS: %i", menustate.hpos);

    y = V_HEIGHT / 8 - menustate.vcount - 2;

    for (i = 0; i < menustate.vcount; i++) {
        int active = (i == menustate.vpos);
        int len = strlen(menustate.vopts[i]);

        erv = drawText(menustate.vopts[i], len, y + i, active);
        ASSERT(erv == ERR_OK, erv);
    }

    return ERR_OK;
}
