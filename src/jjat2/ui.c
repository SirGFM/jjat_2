/**
 * @file src/jjat2/ui.c
 *
 * Controls and displays the game's UI. Usually, the UI stays hidden, but it
 * gets displayed when the player is damaged, on screen transition and
 * during the pause menu.
 */
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/game.h>
#include <GFraMe/gfmTilemap.h>
#include <jjat2/ui.h>
#include <stdint.h>

#define TM_WIDTH        (V_WIDTH / 8)
#define TM_HEIGHT       (V_HEIGHT / 8)
#define TM_DEFAULT_TILE -1

/** Initialize the UI */
err initUI() {
    gfmRV rv;

    rv = gfmTilemap_getNew(&ui.pTilemap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(ui.pTilemap, gfx.pSset8x8, TM_WIDTH, TM_HEIGHT
            , TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Cache the data to easily modify it later on */
    rv = gfmTilemap_getData(&ui.pData, ui.pTilemap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Release all resources used by the UI */
void freeUI() {
    if (ui.pTilemap) {
        gfmTilemap_free(&ui.pTilemap);
    }
    ui.pData = 0;
}

/**
 * Set the title of the map on the UI
 *
 * @param  [ in]title The title
 */
void setMapTitle(char *title);

/** Starts tweening the view in. If it's already visible, reset the countdown to
 * hide it */
void showUI();

/**
 * Set the UI position as either the top of the screen or its bottom
 *
 * @param  [ in]pos The position
 */
void setUIPosition(uiControl pos);

/** Update the UI position */
err updateUI() {
    return ERR_OK;
}

/** Render the UI */
err drawUI() {
    return ERR_OK;
}

