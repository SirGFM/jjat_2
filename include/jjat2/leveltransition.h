/**
 * @file include/jjat2/leveltransition.h
 *
 * Display and hide the transition screen and tween the player to its new
 * position.
 */
#ifndef __JJAT2_LEVELTRANSITION_H__
#define __JJAT2_LEVELTRANSITION_H__

#include <base/error.h>

#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmTilemap.h>

#include <stdint.h>

#define MAX_AREAS   16

/** Packed teleport/loadzone data that is stored within the type */
enum enLevelTransitionFlags {
    TEL_UP                   = 0x01
  , TEL_DOWN                 = 0x02
  , TEL_LEFT                 = 0x04
  , TEL_RIGHT                = 0x08
};
typedef enum enLevelTransitionFlags levelTransitionFlags;

enum {
    LT_LOADED     = 0x01
  , LT_CHECKPOINT = 0x02
};

struct stLeveltransitionData {
    /** Level that will be transitioned to */
    char *pName;
    /** Source position within the teleported level (and clamped to the edge of
     * the screen) */
    uint16_t srcX;
    uint16_t srcY;
    /** Target position within the teleported level */
    uint16_t tgtX;
    uint16_t tgtY;
    /** Direction of the transition */
    uint8_t dir;
};
typedef struct stLeveltransitionData leveltransitionData;

struct stLeveltransitionCtx {
    /** The next level to be loaded */
    leveltransitionData *pNextLevel;
    /** Foreground layer used to simulate a transition effect */
    gfmTilemap *pTransition;
    /** Source position as the edge of the screen (anchored to the top-left
     * corner, whenever possible) */
    uint16_t srcX;
    uint16_t srcY;
    /** Target position as the edge of the screen (anchored to the top-left
     * corner, whenever possible) */
    uint16_t tgtX;
    uint16_t tgtY;
    /** Initial position of gunny's tween */
    uint16_t gunnyX;
    uint16_t gunnyY;
    /** Initial position of swordy's tween */
    uint16_t swordyX;
    uint16_t swordyY;
    /** Controls the transition animation */
    int32_t timer;
    /**
     * Generic flags:
     *  - LT_LOADED - whether the level was loaded
     *  - LT_CHECKPOINT - whether the transition was triggered by a checkpoint
     */
    uint8_t flags;
    /** Direction of the current transition (shifted 24 bits) */
    uint8_t dir;
};
typedef struct stLeveltransitionCtx leveltransitionCtx;

/** Animation for the level transition */
extern leveltransitionCtx lvltransition;

/**
 * Prepare switching to a level transition
 *
 * @param  [ in]pNextLevel Next level data
 */
void switchToLevelTransition(leveltransitionData *pNextLevel);

/** Alloc all required resources */
err initLeveltransition();

/** Releases all previously alloc'ed resources */
void freeLeveltransition();

/** Prepare the transition animation */
err setupLeveltransition();

/** Update the transition animation */
err updateLeveltransition();

/** Render the transition animation */
err drawLeveltransition();

#endif /* __JJAT2_LEVELTRANSITION_H__ */

