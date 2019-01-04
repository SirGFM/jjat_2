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
#if defined(DEBUG)
    /** Running state for the debug build. Allows the game to be executed
     * step-by-step. */
    debugRunState debugRunState;
#endif
};
typedef struct stGameCtx gameCtx;

/** DO_UPDATE wraps the conditional that decides if the update loop should run,
 * allowing for stepping frames/pause the update loop when in debug mode.
 *
 * Note that order is extremely important in this conditional. Since buttons'
 * 'just' flag is set on gfm_isUpdating call, it must only be called before an
 * actual frame. Therefore, it must come after the short-circuit conditional */
#if defined(DEBUG)
#  define DO_UPDATE() \
     ((game.debugRunState == DBG_RUNNING \
         || game.debugRunState == DBG_STEP) \
         && gfm_isUpdating(game.pCtx) == GFMRV_TRUE)
#else
#  define DO_UPDATE() \
     (gfm_isUpdating(game.pCtx) == GFMRV_TRUE)
#endif

/** On debug mode, DEBUG_STEP pauses the update loop if a step was requested */
#if defined(DEBUG)
#  define DEBUG_STEP() \
     do { \
         if (game.debugRunState == DBG_STEP) { \
            game.debugRunState = DBG_PAUSED; \
         } \
     } while (0)
#else
#  define DEBUG_STEP()
#endif

extern gameCtx game;

#endif /* __GAME_H__ */

