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

/* TODO Calculate the max width/height for tilemaps */
#define TM_MAX_WIDTH    320
#define TM_MAX_HEIGHT   240
#define TM_DEFAULT_TILE -1
#define MAX_ENTITIES    32
#define MAX_MAPS        8
#define MAX_AREAS       (MAX_MAPS * 2)

struct stPlaystateCtx {
    /** Maps connected to the current one */
    char *pMapNames[MAX_MAPS];
    /** Target position for the player, after teleporting to a new map */
    int teleportPosition[MAX_MAPS];
    /** Teleports (and their walls) on the current map */
    gfmObject *pAreas[MAX_AREAS];
    /** The level parser */
    gfmParser *pParser;
    /** The game's map */
    gfmTilemap *pMap;
    /** How many map names (for teleporting) there are on this map */
    int teleportCount;
    /** How many areas there are on this map */
    int areaCount;
    /** Map's height, in pixels */
    int height;
    /** Map's width, in pixels */
    int width;
    /** How many entities there are on the current map */
    int entityCount;
    /** Swordy character */
    entityCtx swordy;
    /** Gunny character */
    entityCtx gunny;
    /** Dummy object used to represent the inactive object */
    entityCtx asyncDummy;
    /** List of enemies and interactables */
    entityCtx entities[MAX_ENTITIES];
};
typedef struct stPlaystateCtx playstateCtx;

/** The game's playstate. Declared on include/jjat2/static.c. */
extern playstateCtx playstate;

/** Initialize the playstate so a level may be later loaded and played */
err initPlaystate();

/** If the playstate has been initialized, properly free it up. */
void freePlaystate();

/** Setup the playstate so it may start to be executed */
err loadPlaystate();

/** Update the playstate */
err updatePlaystate();

/** Draw the playstate */
err drawPlaystate();

#endif /* __JJAT2_PLAYSTATE_H__ */

