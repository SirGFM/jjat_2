/**
 * @file include/jjat2/ui.h
 *
 * Controls and displays the game's UI. Usually, the UI stays hidden, but it
 * gets displayed when the player is damaged, on screen transition and
 * during the pause menu.
 */
#ifndef __JJAT2_UI_H__
#define __JJAT2_UI_H__

#include <base/error.h>

#include <GFraMe/gfmTilemap.h>

#include <stdint.h>

/** Enumeration for the UI control bitfield */
enum enUIControl {
    /* UI position */
    UI_BOTTOM       = 0x00000000
  , UI_TOP          = 0x80000000
  , UI_POS_MASK     = 0x80000000
    /* Whether the border and map name should be copied from top <-> bottom */
  , UI_DIRTY_TOP    = 0x40000000
  , UI_DIRTY_BOTTOM = 0x20000000
  , UI_DIRTY_MASK   = 0x60000000
    /* Max time for each animation step (in milliseconds) */
  , UI_TIME_MASK    = 0x1fffffff
  , UI_DISPLAY      = 333
  , UI_WAIT         = 2500
  , UI_HIDE         = 3000
};
typedef enum enUIControl uiControl;

struct stUICtx {
    uint32_t control;
    /** The tilemap's data */
    int *pData;
    /** The tilemap renderer */
    gfmTilemap *pTilemap;
};
typedef struct stUICtx uiCtx;

/** The UI context */
extern uiCtx ui;

/** Initialize the UI */
err initUI();

/** Release all resources used by the UI */
void freeUI();

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

/**
 * Set swordy's life
 *
 * @param  [ in]cur It's current life
 * @param  [ in]max It's maximum life
 */
void setSwordyLife(int cur, int max);

/**
 * Set gunny's life
 *
 * @param  [ in]cur It's current life
 * @param  [ in]max It's maximum life
 */
void setGunnyLife(int cur, int max);

/** Update the UI position */
err updateUI();

/** Render the UI */
err drawUI();

#endif /* __JJAT2_UI_H__ */

