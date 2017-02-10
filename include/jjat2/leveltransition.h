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
    TEL_UP                   = 0x00000000
  , TEL_DOWN                 = 0x10000000
  , TEL_LEFT                 = 0x20000000
  , TEL_RIGHT                = 0x30000000

  , TEL_CHECKPOINT_TRIGGERED = 0x10000000

  , TEL_DIR_MASK             = 0x30000000
  , TEL_DIR_BITS             = 24
  , TEL_INDEX_MASK           = 0x0ff00000
  , TEL_INDEX_BITS           = 20
};
typedef enum enLevelTransitionFlags levelTransitionFlags;

enum {
    LT_LOADED     = 0x01
  , LT_CHECKPOINT = 0x02
};

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
    /**
     * Generic flags:
     *  - LT_LOADED - whether the level was loaded
     *  - LT_CHECKPOINT - whether the transition was triggered by a checkpoint
     */
    uint8_t flags;
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

/**
 * Retrieve the transition data for a given level
 *
 * @param  [out]ppName Name of the target level
 * @param  [out]pTgtX  Target position within the level
 * @param  [out]pTgtY  Target position within the level
 * @param  [ in]index  Level's index
 */
err getLevelTransitionData(char **ppName, int *pTgtX, int *pTgtY, int index);

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

/**
 * Parse a checkpoint
 *
 * @param  [ in]pParser The parser pointing at a checkpoint
 */
err parseCheckpoint(gfmParser *pParser);

/**
 * Prepare level transition into a generic level
 *
 * @param  [ in]levelName Level's name
 * @param  [ in]tgtX      Starting position of players
 * @param  [ in]tgtY      Starting position of players
 * @param  [ in]dir       Movement direction of the transition overlay
 */
err setupGenericLeveltransition(char *levelName, int tgtX, int tgtY
        , levelTransitionFlags dir);

/** Prepare the transition animation */
err setupLeveltransition();

/** Update the transition animation */
err updateLeveltransition();

/** Render the transition animation */
err drawLeveltransition();

#endif /* __JJAT2_LEVELTRANSITION_H__ */

