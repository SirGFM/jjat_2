/**
 * @file src/mainloop.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <base/mainloop.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>

/** Run the main loop until the game is closed */
err mainloop() {
    err erv;
    gfmRV rv;

    while (gfm_didGetQuitFlag(game.pCtx) != GFMRV_TRUE) {
        /* TODO Switch state */

        /* Wait for an event */
        rv = gfm_handleEvents(game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

#if defined(DEBUG)
        erv = updateDebugInput();
        ASSERT(erv == ERR_OK, erv);
        handleDebugInput();
#endif

        while (DO_UPDATE()) {
            rv = gfm_fpsCounterUpdateBegin(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            erv = updateInput();
            ASSERT(erv == ERR_OK, erv);

            rv = gfm_getElapsedTime(&(game.elapsed), game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            /* TODO Update the current state */

            rv = gfm_fpsCounterUpdateEnd(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            DEBUG_STEP();
        }

        while (gfm_isDrawing(game.pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            /* TODO Render the current state */

            if (IS_QUADTREE_VISIBLE()) {
                /* NOTE: This will break if the quadtree isn't set up */
                rv = gfmQuadtree_drawBounds(collision.pQt, game.pCtx, 0);
                ASSERT(rv == GFMRV_OK, rv);
            }

            rv = gfm_drawEnd(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        }
    }

    return ERR_OK;
}

