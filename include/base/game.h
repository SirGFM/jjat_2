/**
 * @file include/base/game.h
 *
 * Define the game structure
 */
#ifndef __BASE_GAME_H__
#define __BASE_GAME_H__

#include <conf/state.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmCamera.h>

enum enDebugRunState {
    DBG_PAUSED  = 0x0000,
    DBG_RUNNING = 0x0001,
    DBG_STEP    = 0x0002,
};
typedef enum enDebugRunState debugRunState;

#if defined(JJATENGINE)
enum enGameFlags {
    AC_SWORDY       = 0x01
  , AC_GUNNY        = 0x02
  , AC_BOTH         = (AC_SWORDY | AC_GUNNY)
  , CMD_CUSTOMINPUT = 0x04
  , FX_PRETTYRENDER = 0x08
  , CMD_LAZYLOAD    = 0x10
  , CMD_TIMER       = 0x20
  , CMD_TP_RELEASE  = 0x40
  , CMD_DEBUG       = 0x80
};
typedef enum enGameFlags gameFlags;

enum enSessionFlags {
    SF_ENABLE_RESET = 0x01
  , SF_BLUE_ACTIVE  = 0x02
};
#endif /* JJATENGINE */

struct stGameCtx {
    /** GFraMe context */
    gfmCtx *pCtx;
    /** The game's default camera */
    gfmCamera *pCamera;
#if defined(JJATENGINE)
    /* Input to be used when starting the game (NULL-terminated) */
    char *curInputMap;
    /* Alloc'ed size for curInputMap (which must be NULL-terminated!) */
    int inputMapSize;
    /** Lots of flags to control the game */
    gameFlags flags;
    /** Flags recovered/saved for a given session/save file */
    enum enSessionFlags sessionFlags;
#endif /* JJATENGINE */
    /** Fixed time elapsed since the last frame, in milliseconds. Note that even
     * though this should store a 'fixed' value, it may vary in a pre-determined
     * and fixed pattern (e.g., 17ms, 17ms, 16ms, ..., for 60 FPS), in order
     * match the desired FPS without rounding */
    int elapsed;
    /** Current state being played (i.e., updated & drawn) */
    state currentState;
    /** State that will start being played on the next frame */
    state nextState;
    /** Running state for the debug build. Allows the game to be executed
     * step-by-step. */
    debugRunState debugRunState;
};
typedef struct stGameCtx gameCtx;

extern gameCtx game;

#endif /* __GAME_H__ */

