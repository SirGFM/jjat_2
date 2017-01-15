/**
 * @file include/jjat2/leveltransition.h
 *
 * Display and hide the transition screen and tween the player to its new
 * position.
 */
#ifndef __JJAT2_LEVELTRANSITION_H__
#define __JJAT2_LEVELTRANSITION_H__

#include <base/error.h>

#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <stdint.h>

#define MAX_AREAS   16

struct stLeveltransitionCtx {
    /** Maps connected to the current one */
    char *pNames[MAX_AREAS];
    /** Teleports (and their walls) on the current map */
    gfmObject *pAreas[MAX_AREAS];
    /** Foreground layer used to simulate a transition effect */
    gfmTilemap *pTransition;
    /** Target position for the player, after teleporting to a new map */
    uint32_t teleportPosition[MAX_AREAS];
    /** Controls the transition animation */
    int32_t timer;
    /** How many areas were used on the current level */
    uint8_t areasCount;
    /** Index of the level to be loaded */
    uint8_t index;
    /** Whether the next level has been loaded */
    uint8_t loaded;
    /** Direction of the current transition (shifted 24 bits) */
    uint8_t dir;
};
typedef struct stLeveltransitionCtx leveltransitionCtx;

/** Animation for the level transition */
extern leveltransitionCtx lvltransition;

/**
 * Retrieves the index of a level given its type
 *
 * @param  [ in]levelType The type of the loadzone object
 */
int getLoadzoneIndex(int levelType);

/**
 * Prepare switching to a level transition
 *
 * @param  [ in]index Index of the loadzone
 */
void switchToLevelTransition(int index);

/** Retrieve the name of the level to be loaded */
char* getNextLevelName();

/** Alloc all required resources */
err initLeveltransition();

/** Releases all previously alloc'ed resources */
void freeLeveltransition();

/** Reset all objects parsed on the previous level */
void resetLeveltransition();

/** Setup static collision against all level transition objects */
err calculateStaticLeveltransition();

/**
 * Parse a loadzone
 *
 * @param  [ in]pParser The parser pointing at a loadzone
 */
err parseLoadzone(gfmParser *pParser);

/**
 * Parse a invisible wall
 *
 * @param  [ in]pParser The parser pointing at a invisible wall
 */
err parseInvisibleWall(gfmParser *pParser);

/** Prepare the transition animation */
err setupLeveltransition();

/** Update the transition animation */
err updateLeveltransition();

/** Render the transition animation */
err drawLeveltransition();

#endif /* __JJAT2_LEVELTRANSITION_H__ */

