/**
 * @file include/base/game.h
 *
 *
 */
#ifndef __GAME_H__
#define __GAME_H__

#include <GFraMe/gframe.h>

struct stGameCtx {
    /** GFraMe context */
    gfmCtx *pCtx;
    /** Fixed time elapsed since the last frame, in milliseconds. Note that even
     * though this should store a 'fixed' value, it may vary in a pre-determined
     * and fixed pattern (e.g., 17ms, 17ms, 16ms, ..., for 60 FPS), in order
     * match the desired FPS without rounding */
    int elapsed;
};
typedef struct stGameCtx gameCtx;

extern gameCtx game;

#endif /* __GAME_H__ */

