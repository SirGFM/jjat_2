/**
 * @file include/base/loadstate.h
 *
 * Implements a basic load screen. If a spritesheet with a bitmap font is
 * supplied, the resource being currently loaded is displayed bellowed the
 * "LOADING" text.
 *
 * Every game must implements its own ...
 */
#ifndef __BASE_LOADSTATE_H__
#define __BASE_LOADSTATE_H__

#include <base/error.h>
#include <GFraMe/gfmSpriteset.h>

struct stLoadstateCtx {
    gfmSpriteset *pBitmapFont;
    int offset;
};
typedef struct stLoadstateCtx loadstateCtx;

/** The game's loadstate. Declared on src/base/static.c. */
extern loadstateCtx loadstate;

/**
 * Initialize the playstate so a level may be later loaded and played. The game
 * is responsible for calling this with the proper parameters. Failure to do so
 * will result in a empty and (even more) boring loading screen.
 *
 * @param [ in]pBitmapFont The spriteset with the desired bitmap font
 * @param [ in]offset Offset of the bitmap font within the spriteset
 */
err initLoadstate(gfmSpriteset *pBitmapFont, int offset);

/** If the playstate has been initialized, properly free it up. */
void freeLoadstate();

/** Setup the playstate so it may start to be executed */
err loadLoadstate();

/** Update the playstate */
err updateLoadstate();

/** Draw the playstate */
err drawLoadstate();

#endif /* __BASE_LOADSTATE_H__ */

