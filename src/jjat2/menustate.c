#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <jjat2/menu_input.h>
#include <jjat2/menustate.h>
#include <GFraMe/gfmDebug.h>
#include <string.h>

enum menuDir {
    md_none  = 0x00,
    md_left  = 0x01,
    md_right = 0x02,
    md_up    = 0x04,
    md_down  = 0x08,
};

#define PRESS_DELAY_MS 500
#define HOLD_DELAY_MS  250

/** Initialize the menustate. */
err initMenustate() {
    return ERR_OK;
}

/** If the menustate has been initialized, properly free it up. */
void freeMenustate() {
}

/** Setup the loadstate so it may start to be executed */
err loadMenustate() {
    err erv;

    erv = setDefaultMenuInput();
    ASSERT(erv == ERR_OK, erv);

    memset(&menustate, 0x0, sizeof(menustateCtx));

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
    if (IS_MENU_RELEASED(up) && IS_MENU_RELEASED(down) &&
            IS_MENU_RELEASED(left) && IS_MENU_RELEASED(right))
        menustate.dir = md_none;

    handleCursorMovement();

    return ERR_OK;
}

/** Draw the menustate */
err drawMenustate() {
    gfmDebug_printf(game.pCtx, 0, 128, "  DIR: %i", menustate.dir);
    gfmDebug_printf(game.pCtx, 0, 136, "DELAY: %i", menustate.delay);
    gfmDebug_printf(game.pCtx, 0, 144, " VPOS: %i", menustate.vpos);
    gfmDebug_printf(game.pCtx, 0, 152, " HPOS: %i", menustate.hpos);

    return ERR_OK;
}
