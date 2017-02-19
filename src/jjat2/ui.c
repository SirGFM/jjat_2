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
/** Indexes for some tiles */
#define TM_TEXT_TILE    64
#define TM_EMPTY_TILE   108
#define TM_SWORDY_FULL_HEART_TILE   122
#define TM_SWORDY_EMPTY_HEART_TILE  124
#define TM_GUNNY_FULL_HEART_TILE    125
#define TM_GUNNY_EMPTY_HEART_TILE   127
/** Horizontal index range of the text tiles */
#define TM_TEXT_FIRST_TILE  8
#define TM_TEXT_LAST_TILE   31
/** Horizontal index range of swordy's health tiles */
#define TM_SWORDY_HEALTH_FIRST_TILE 2
#define TM_SWORDY_HEALTH_LAST_TILE  6
/** Horizontal index range of gunny's health tiles */
#define TM_GUNNY_HEALTH_FIRST_TILE  37
#define TM_GUNNY_HEALTH_LAST_TILE   33
#define HEALTH_PER_ROW              5
#define TOTAL_HEALTH_COUNT          (HEALTH_PER_ROW * 2)

/** Forward declaration of arrays copied into the UI tilemap */
static int _baseUiMap[TM_WIDTH * TM_HEIGHT];
static int _topUiBorder[TM_TEXT_LAST_TILE - TM_TEXT_FIRST_TILE + 3];
static int _bottomUiBorder[TM_TEXT_LAST_TILE - TM_TEXT_FIRST_TILE + 3];

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

    resetUI();

    return ERR_OK;
}

/** Release all resources used by the UI */
void freeUI() {
    if (ui.pTilemap) {
        gfmTilemap_free(&ui.pTilemap);
    }
    ui.pData = 0;
}

/** Reset the UI */
void resetUI() {
    /* Initially, set it with the bottom data */
    memcpy(ui.pData, _baseUiMap, sizeof(_baseUiMap));

    ui.control = (UI_BOTTOM | UI_HIDE);
}

/**
 * Set the title of the map on the UI.
 *
 * The title should only contain lower case characters (and space). It should
 * also be NULL terminated string.
 * Also, underscores ('_') are replaced by spaces.
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
    i = TM_TEXT_FIRST_TILE;
    while (*title != '\0' && i <= TM_TEXT_LAST_TILE){ 
        if (*title == ' ' || *title == '_') {
            ui.pData[i + offset] = TM_EMPTY_TILE;
        }
        else {
            ui.pData[i + offset] = *title - 'a' + TM_TEXT_TILE;
        }
        i++;
        title++;
    }

    /* Clear the rest of the text area */
    while (i <= TM_TEXT_LAST_TILE) {
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
    int count, i, offset, tile;

    count = 0;
    i = 0;
    offset = 0;
    tile = TM_SWORDY_FULL_HEART_TILE;

    while (count < TOTAL_HEALTH_COUNT) {
        if (i == HEALTH_PER_ROW) {
            offset += TM_WIDTH;
            i = 0;
        }

        ui.pData[offset + TM_SWORDY_HEALTH_FIRST_TILE + i] = tile;

        count++;
        i++;
        if (count >= max) {
            tile = TM_EMPTY_TILE;
        }
        else if (count >= cur) {
            tile = TM_SWORDY_EMPTY_HEART_TILE;
        }
    }
}

/**
 * Set gunny's life
 *
 * @param  [ in]cur It's current life
 * @param  [ in]max It's maximum life
 */
void setGunnyLife(int cur, int max) {
    int count, i, offset, tile;

    count = 0;
    i = 0;
    offset = 0;
    tile = TM_GUNNY_FULL_HEART_TILE;

    while (count < TOTAL_HEALTH_COUNT) {
        if (i == HEALTH_PER_ROW) {
            offset += TM_WIDTH;
            i = 0;
        }

        ui.pData[offset + TM_GUNNY_HEALTH_FIRST_TILE - i] = tile;

        count++;
        i++;
        if (count >= max) {
            tile = TM_EMPTY_TILE;
        }
        else if (count >= cur) {
            tile = TM_GUNNY_EMPTY_HEART_TILE;
        }
    }
}

/** Update the UI position */
err updateUI() {
    uint32_t time;

    time = ui.control & UI_TIME_MASK;
    if (time >= UI_HIDE) {
        return ERR_OK;
    }
    time += game.elapsed;

    ui.control &= ~UI_TIME_MASK;
    ui.control |= time & UI_TIME_MASK;
    return ERR_OK;
}

/** Render the UI */
err drawUI() {
    gfmRV rv;
    uint32_t time;
    int x, y;

    time = ui.control & UI_TIME_MASK;
    if (time >= UI_HIDE) {
        return ERR_OK;
    }

    /* About switching position: first copy the text part then the static */
    if (ui.control & UI_DIRTY_TOP) {
        /* Copy from bottom to top */
        memcpy(ui.pData + TM_TEXT_FIRST_TILE - 1
                , ui.pData + TM_TEXT_FIRST_TILE + TM_WIDTH - 1
                , sizeof(int) * (TM_TEXT_LAST_TILE - TM_TEXT_FIRST_TILE + 3));
        memcpy(ui.pData + TM_TEXT_FIRST_TILE + TM_WIDTH - 1
                , _topUiBorder, sizeof(_topUiBorder));

        ui.control &= ~UI_POS_MASK;
        ui.control |= UI_TOP;
    }
    else if (ui.control & UI_DIRTY_BOTTOM) {
        /* Copy from top to bottom */
        memcpy(ui.pData + TM_TEXT_FIRST_TILE + TM_WIDTH - 1
                , ui.pData + TM_TEXT_FIRST_TILE - 1
                , sizeof(int) * (TM_TEXT_LAST_TILE - TM_TEXT_FIRST_TILE + 3));
        memcpy(ui.pData + TM_TEXT_FIRST_TILE - 1
                , _bottomUiBorder, sizeof(_bottomUiBorder));

        ui.control &= ~UI_POS_MASK;
        ui.control |= UI_BOTTOM;
    }
    ui.control &= ~UI_DIRTY_MASK;

    /* Set the position according to the timer */
    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    if (time < UI_DISPLAY) {
        int ds, start;

        if (ui.control & UI_TOP) {
            start = y - TM_HEIGHT * 8;
            ds = TM_HEIGHT * 8;
        }
        else if (ui.control & UI_BOTTOM) {
            start = y + V_HEIGHT;
            ds = -TM_HEIGHT * 8;
        }
        else {
            ASSERT(0, ERR_UNEXPECTEDBEHAVIOUR);
        }

        y = start + ds * (int)time / UI_DISPLAY;
    }
    else if (time < UI_WAIT) {
        if (ui.control & UI_BOTTOM) {
            y = y + V_HEIGHT - TM_HEIGHT * 8;
        }
    }
    else if (time < UI_HIDE) {
        int ds, start;

        time -= UI_WAIT;
        if (ui.control & UI_TOP) {
            start = y;
            ds = -TM_HEIGHT * 8;
        }
        else if (ui.control & UI_BOTTOM) {
            start = y + V_HEIGHT - TM_HEIGHT * 8;
            ds = TM_HEIGHT * 8;
        }
        else {
            ASSERT(0, ERR_UNEXPECTEDBEHAVIOUR);
        }

        y = start + ds * (int)time / (UI_HIDE - UI_WAIT);
    }

    gfmTilemap_setPosition(ui.pTilemap, x, y);

    rv = gfmTilemap_draw(ui.pTilemap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

static int _baseUiMap[] = {
    114, 115, 122, 122, 122, 123, 123, 110, 111, 111, 111, 111, 111, 111, 111
  , 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111
  , 111, 111, 112, 126, 126, 125, 125, 125, 118, 119

  , 116, 117, 123, 124, 124, 124, 124, 109, 108, 108, 108, 108, 108, 108, 108
  , 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108
  , 108, 108, 113, 127, 127, 127, 127, 126, 120, 121
};

/** Row for the border of a top-placed UI */
static int _topUiBorder[] = {
    94, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95
  , 95, 95, 95, 95, 95, 95, 96
};

/** Row for the border of a bottom-placed UI */
static int _bottomUiBorder[] = {
    110, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111
  , 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 112
};

