/**
 * @file include/base/menustate.h
 *
 * Implements a basic menu screen. If a spritesheet with a bitmap font is
 * supplied, the resource being currently loaded is displayed bellowed the
 * "LOADING" text.
 *
 * Every game must implements its own ...
 */
#ifndef __JJAT2_MENUSTATE_H__
#define __JJAT2_MENUSTATE_H__

#include <base/error.h>
#include <conf/state.h>
#include <GFraMe/gfmSpriteset.h>
#include <GFraMe/gfmText.h>

struct stMenustateCtx {
    gfmText *pText;
    char **vopts;
    int dir;
    int delay;
    int vpos;
    int hpos;
    int hcount;
    int vcount;
};
typedef struct stMenustateCtx menustateCtx;

/** The game's menustate. Declared on src/jjat2/static.c. */
extern menustateCtx menustate;

/** Initialize the menustate. */
err initMenustate();

/** If the menustate has been initialized, properly free it up. */
void freeMenustate();

/** Setup the loadstate so it may start to be executed */
err loadMenustate();

/** Update the menustate */
err updateMenustate();

/** Draw the menustate */
err drawMenustate();

#endif /* __JJAT2_MENUSTATE_H__ */

