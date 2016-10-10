/**
 * @file include/base/game.h
 *
 * Define the game structure
 */
#ifndef __BASE_GAME_H__
#define __BASE_GAME_H__

#include <GFraMe/gframe.h>

enum enDebugRunState {
    DBG_PAUSED  = 0x0000,
    DBG_RUNNING = 0x0001,
    DBG_STEP    = 0x0002,
};
typedef enum enDebugRunState debugRunState;

struct stGameCtx {
    /** GFraMe context */
    gfmCtx *pCtx;
    /** Fixed time elapsed since the last frame, in milliseconds. Note that even
     * though this should store a 'fixed' value, it may vary in a pre-determined
     * and fixed pattern (e.g., 17ms, 17ms, 16ms, ..., for 60 FPS), in order
     * match the desired FPS without rounding */
    int elapsed;
#if defined(DEBUG)
    /** Running state for the debug build. Allows the game to be executed
     * step-by-step. */
    debugRunState debugRunState;
#endif
};
typedef struct stGameCtx gameCtx;

/** DO_UPDATE wraps the conditional that decides if the update loop should run,
 * allowing for stepping frames/pause the update loop when in debug mode */
#if defined(DEBUG)
#  define DO_UPDATE() \
     (gfm_isUpdating(game.pCtx) == GFMRV_TRUE \
         && (game.debugRunState == DBG_RUNNING \
         || game.debugRunState == DBG_STEP))
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

