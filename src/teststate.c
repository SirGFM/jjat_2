/**
 * @file src/teststate.c
 *
 * Simple state to test stuff
 */
#include <base/game_const.h>
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <jjat_2/girl_player.h>
#include <jjat_2/level.h>
#include <jjat_2/teststate.h>

#include <stdlib.h>
#include <string.h>

/**
 * Alloc the test state and initialize all of its components
 *
 * @return GFraMe return value
 */
gfmRV test_init() {
    /** Return value */
    gfmRV rv;

    /* Load the teststate */
    rv = level_loadTest();

    return rv;
}

/**
 * Update the test state
 *
 * @return GFraMe return value
 */
gfmRV test_update() {
    /** Return value */
    gfmRV rv;
    /** The world dimensions in pixels */
    int w, h;

    /* Initialize the quadtree */
    rv = gfmTilemap_getDimension(&w, &h, pGlobal->pTMap);
    ASSERT(rv == GFMRV_OK, rv);
    w = (w + 4) * 8;
    h = (h + 4) * 8;
    rv = gfmQuadtree_initRoot(pGlobal->pQt, -16, -16, w, h, QT_MAX_DEPTH,
            QT_MAX_NODES);
    ASSERT(rv == GFMRV_OK, rv);

    /* Update and collide the terrain */
    rv = gfmTilemap_update(pGlobal->pTMap, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmQuadtree_populateTilemap(pGlobal->pQt, pGlobal->pTMap);
    ASSERT(rv == GFMRV_OK, rv);

    /* Update and collide the girl */
    rv = grlPl_update();
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Update everything else */

    /* Finally, run the post-update on anything that needs it */
    rv = grlPl_postUpdate();
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:

    return rv;
}

/**
 * Render the test state
 *
 * @return GFraMe return value
 */
gfmRV test_draw() {
    /** Return value */
    gfmRV rv;

    rv = gfmTilemap_draw(pGlobal->pTMap, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = grlPl_draw();
    ASSERT(rv == GFMRV_OK, rv);

    if (pGame->flags & DBG_RENDERQT) {
        rv = gfmQuadtree_drawBounds(pGlobal->pQt, pGame->pCtx, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:

    return rv;
}

/**
 * Release all the resources used by the test state
 *
 * @return GFraMe return value
 */
void test_free() {
}

