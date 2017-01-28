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
#include <string.h>

#define TM_WIDTH        (V_WIDTH / 8)
#define TM_HEIGHT       2
#define TM_DEFAULT_TILE -1
#define TM_TEXT_TILE    64
#define TM_EMPTY_TILE   108
/** Horizontal index range of the text tiles */
#define TM_TEXT_MIN_X   8
#define TM_TEXT_MAX_X   31

/** Forward declaration of arrays copied into the UI tilemap */
static int _baseUiMap[TM_WIDTH * TM_HEIGHT];
static int _topUiBorder[TM_TEXT_MAX_X - TM_TEXT_MIN_X + 3];
static int _bottomUiBorder[TM_TEXT_MAX_X - TM_TEXT_MIN_X + 3];

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

    /* Initially, set it with the bottom data */
    memcpy(ui.pData, _baseUiMap, sizeof(_baseUiMap));

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
 * Set the title of the map on the UI.
 *
 * The title should only contain lower case characters (and space). It should
 * also be NULL terminated string.
 *
 * @param  [ in]title The title
 */
void setMapTitle(char *title) {
    int i, offset;

    /* If necessary, offset to the bottom row */
    if ((ui.control & UI_POS_MASK) == UI_BOTTOM) {
        offset = TM_WIDTH;
    }
    else {
        offset = 0;
    }

    /* Set the text */
    i = TM_TEXT_MIN_X;
    while (*title != '\0' && i <= TM_TEXT_MAX_X){ 
        if (*title == ' ') {
            ui.pData[i + offset] = TM_EMPTY_TILE;
        }
        else {
            ui.pData[i + offset] = *title - 'a' + TM_TEXT_TILE;
        }
        i++;
        title++;
    }

    /* Clear the rest of the text area */
    while (i <= TM_TEXT_MAX_X) {
        ui.pData[i + offset] = TM_EMPTY_TILE;
        i++;
    }
}

/** Starts tweening the view in. If it's already visible, reset the countdown to
 * hide it */
void showUI() {
    /* Cleart the time bits so it may be accumulated once again */
    ui.control &= ~UI_TIME_MASK;
}

/**
 * Set the UI position as either the top of the screen or its bottom
 *
 * @param  [ in]pos The position
 */
void setUIPosition(uiControl pos) {
    ui.control &= ~UI_DIRTY_MASK;
    if ((ui.control & UI_POS_MASK) != pos) {
        if (pos == UI_BOTTOM) {
            ui.control |= UI_DIRTY_BOTTOM;
        }
        else {
            ui.control |= UI_DIRTY_TOP;
        }
    }
}

/**
 * Set swordy's life
 *
 * @param  [ in]cur It's current life
 * @param  [ in]max It's maximum life
 */
void setSwordyLife(int cur, int max) {
    /* TODO */
}

/**
 * Set gunny's life
 *
 * @param  [ in]cur It's current life
 * @param  [ in]max It's maximum life
 */
void setGunnyLife(int cur, int max) {
    /* TODO */
}

/** Update the UI position */
err updateUI() {
    uint32_t time;

    time = ui.control & UI_TIME_MASK;
    if (time >= UI_HIDE) {
        return ERR_OK;
    }
    time += game.elapsed;

    ui.control |= time & UI_TIME_MASK;
    return ERR_OK;
}

/** Render the UI */
err drawUI() {
    if ((ui.control & UI_TIME_MASK) < UI_HIDE) {
        return ERR_OK;
    }

    /* About switching position: first copy the text part then the static */
    if (ui.control & UI_DIRTY_TOP) {
        /* TODO Copy from bottom to top */
    }
    else if (ui.control & UI_DIRTY_BOTTOM) {
        /* TODO Copy from top to bottom */
    }

    /* TODO Render it */

    return ERR_OK;
}

static int _baseUiMap[] = {
    116, 117, 124, 124, 124, 125, 125, 112, 113, 113, 113, 113, 113, 113, 113
  , 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113
  , 113, 113, 114, 128, 128, 127, 127, 127, 120, 121

  , 118, 119, 125, 126, 126, 126, 126, 111, 110, 110, 110, 110, 110, 110, 110
  , 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110
  , 110, 110, 115, 129, 129, 129, 129, 128, 122, 123
};

/** Row for the border of a top-placed UI */
static int _topUiBorder[] = {
    96, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97
  , 97, 97, 97, 97, 97, 97, 98
};

/** Row for the border of a bottom-placed UI */
static int _bottomUiBorder[] = {
    112, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113
  , 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 114
};

