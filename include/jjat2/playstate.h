/**
 * @file include/jjat/playstate.h
 */
#ifndef __JJAT2_PLAYSTATE_H__
#define __JJAT2_PLAYSTATE_H__

#include <base/error.h>

#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <jjat2/entity.h>
#include <jjat2/leveltransition.h>

#include <stdint.h>

/* TODO Calculate the max width/height for tilemaps */
#define TM_MAX_WIDTH    320
#define TM_MAX_HEIGHT   240
#define TM_DEFAULT_TILE -1
#define MAX_ENTITIES    32
#define MAX_AREAS       16
#define TILE_DIMENSION  8

/** Maximum number of characters for the path to any given level */
#define MAX_LEVEL_NAME  128
/** Maximum length for any level name. Since this is later modified (with prefix
 * and postfix), this length does NOT leave a free space for the trailing '\0' */
#define MAX_VALID_LEN \
    (MAX_LEVEL_NAME - (sizeof("levels/") - 1) - (sizeof("_bg_tm.gfm") - 1) - 1)

union unHitboxCtx {
    leveltransitionData ltData;
};

struct stPlaystateCtx {
    /** Next level to be loaded, if any */
    leveltransitionData *pNextLevel;
    /** Static areas (like loadzones, checkpoints and so on) */
    gfmHitbox *pAreas;
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
    /** List of enemies and interactables. Note that some interactibles (like
     * doors) only use the entity's sprite */
    entityCtx entities[MAX_ENTITIES];
    /** Map's height, in pixels */
    uint16_t height;
    /** Map's width, in pixels */
    uint16_t width;
    /** When a player last touched a downward level transition. Used to fix a
     * "bug" caused by falling back into the level transition you just went
     * through. */
    uint16_t lastTouch;
    /** How many areas were used on the current level */
    uint8_t areasCount;
    /** How many entities there are on the current map */
    uint8_t entityCount;
    /** Generic flags */
    uint8_t flags;
    /** Context for the hitboxes */
    union unHitboxCtx data[MAX_AREAS];
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
 * @param  [ in]pData Data of the loadzone that was hit
 */
void onHitLoadzone(int type, leveltransitionData *pData);

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

