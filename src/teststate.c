/**
 * @file src/teststate.c
 *
 * Simple state to test stuff
 */
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

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

    /* TODO Initialize the quadtree */

    rv = gfmTilemap_update(pGlobal->pTMap, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Add the tilemap to the quadtree */

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

