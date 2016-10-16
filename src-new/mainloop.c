/**
 * @file src/mainloop.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <base/mainloop.h>

#include <conf/state.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>

/** Run the main loop until the game is closed */
err mainloop() {
    err erv;
    gfmRV rv;

    /* TODO Set initial state */

    while (gfm_didGetQuitFlag(game.pCtx) != GFMRV_TRUE) {
        /* Switch state */
        if (game.nextState != ST_NONE) {
            switch (game.nextState) {
                /* TODO Load/Setup the state */
                default: {}
            }

            game.currentState = game.nextState;
            game.nextState = ST_NONE;
        }

        /* Wait for an event */
        rv = gfm_handleEvents(game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

#if defined(DEBUG)
        erv = updateDebugInput();
        ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
        handleDebugInput();
#endif

        while (DO_UPDATE()) {
            rv = gfm_fpsCounterUpdateBegin(game.pCtx);
            ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

            erv = updateInput();
            ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

            rv = gfm_getElapsedTime(&(game.elapsed), game.pCtx);
            ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

            /* Update the current state */
            switch (game.currentState) {
                /* TODO Call the state's update */
                default: {}
            }

            rv = gfm_fpsCounterUpdateEnd(game.pCtx);
            ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

            DEBUG_STEP();
        }

        while (gfm_isDrawing(game.pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(game.pCtx);
            ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

            /* Render the current state */
            switch (game.currentState) {
                /* TODO Call the state's draw */
                default: {}
            }

            if (IS_QUADTREE_VISIBLE()) {
                /* NOTE: This will break if the quadtree isn't set up */
                rv = gfmQuadtree_drawBounds(collision.pQt, game.pCtx, 0);
                ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
            }

            rv = gfm_drawEnd(game.pCtx);
            ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
        }
    }

    erv = ERR_OK;
__ret:

    return erv;
}

