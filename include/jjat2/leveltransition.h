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

/** Packed teleport/loadzone data that is stored within the type */
enum enLevelTransitionFlags {
    TEL_UP         = 0x00000000
  , TEL_DOWN       = 0x10000000
  , TEL_LEFT       = 0x20000000
  , TEL_RIGHT      = 0x30000000

  , TEL_DIR_MASK   = 0x30000000
  , TEL_DIR_BITS   = 24
  , TEL_INDEX_MASK = 0x0ff00000
  , TEL_INDEX_BITS = 20
};
typedef enum enLevelTransitionFlags levelTransitionFlags;

struct stLeveltransitionGeometryCtx {
    /** Maps connected to the current one */
    char *pNames[MAX_AREAS];
    /** Teleports (and their walls) on the current map */
    gfmObject *pAreas[MAX_AREAS];
    /** Target position for the player, after teleporting to a new map */
    uint32_t teleportPosition[MAX_AREAS];
    /** How many areas were used on the current level */
    uint8_t areasCount;
    /** Index of the level to be loaded */
    uint8_t index;
};
typedef struct stLeveltransitionGeometryCtx leveltransitionGeometryCtx;

struct stLeveltransitionCtx {
    /** Pointer to the level name */
    char *pCachedName;
    /** Foreground layer used to simulate a transition effect */
    gfmTilemap *pTransition;
    /** Target position for the current transition. X is packed into the lower
     * 16 and Y is packed into the higher 16 bits */
    uint32_t cachedTargetPosition;
    /** Initial position of gunny's tween */
    uint32_t gunnyPos;
    /** Initial position of swordy's tween */
    uint32_t swordyPos;
    /** Controls the transition animation */
    int32_t timer;
    /** Padding to keep everything nicely packed into 64 bits */
    int32_t padding_0;
    /** Padding to keep everything nicely packed into 32 bits */
    uint16_t padding_1;
    /** Whether the next level has been loaded */
    uint8_t loaded;
    /** Direction of the current transition (shifted 24 bits) */
    uint8_t dir;
    /** Geometry data loaded from the parser */
    leveltransitionGeometryCtx geometry;
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

