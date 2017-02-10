/**
 * @file include/jjat/playstate.h
 */
#ifndef __JJAT2_PLAYSTATE_H__
#define __JJAT2_PLAYSTATE_H__

#include <base/error.h>

#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <jjat2/entity.h>

#include <stdint.h>

/* TODO Calculate the max width/height for tilemaps */
#define TM_MAX_WIDTH    320
#define TM_MAX_HEIGHT   240
#define TM_DEFAULT_TILE -1
#define MAX_ENTITIES    32

/** Maximum number of characters for the path to any given level */
#define MAX_LEVEL_NAME  128
/** Maximum length for any level name. Since this is later modified (with prefix
 * and postfix), this length does NOT leave a free space for the trailing '\0' */
#define MAX_VALID_LEN \
    (MAX_LEVEL_NAME - (sizeof("levels/") - 1) - (sizeof("_bg_tm.gfm") - 1) - 1)

struct stPlaystateCtx {
    /** The level parser */
    gfmParser *pParser;
    /** The game's map */
    gfmTilemap *pMap;
#if defined(JJAT_ENABLE_BACKGROUND)
    /** The game's background */
    gfmTilemap *pBackground;
#endif /* JJAT_ENABLE_BACKGROUND */
    /** Swordy character */
    entityCtx swordy;
    /** Gunny character */
    entityCtx gunny;
    /** Dummy object used to represent the inactive object */
    entityCtx asyncDummy;
    /** List of enemies and interactables */
    entityCtx entities[MAX_ENTITIES];
    /** Map's height, in pixels */
    uint16_t height;
    /** Map's width, in pixels */
    uint16_t width;
    /** How many entities there are on the current map */
    uint8_t entityCount;
    /** Generic flags */
    uint8_t flags;
};
typedef struct stPlaystateCtx playstateCtx;

/** The game's playstate. Declared on include/jjat2/static.c. */
extern playstateCtx playstate;

/** Initialize the playstate so a level may be later loaded and played */
err initPlaystate();

/** If the playstate has been initialized, properly free it up. */
void freePlaystate();

/**
 * Setup loading the next map.
 *
 * @param  [ in]type Type of entity that entered the loadzone
 */
void onHitLoadzone(int type, int levelType);

/** Setup the playstate so it may start to be executed */
err loadPlaystate();

/** Remove the flag that signals that no level is being loaded */
void clearPlaystateLevelFlag();

/** Update the playstate */
err updatePlaystate();

/** Draw the playstate */
err drawPlaystate();

/** Draw only the players */
err drawPlayers();

#endif /* __JJAT2_PLAYSTATE_H__ */

