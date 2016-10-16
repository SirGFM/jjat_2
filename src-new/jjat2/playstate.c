/**
 * @file src/jjat2/playstate.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <GFraMe/gfmTilemap.h>

#include <jjat2/dictionary.h>
#include <jjat2/playstate.h>

/** Initialize the playstate so a level may be later loaded and played */
err initPlaystate() {
    gfmRV rv;

    rv = gfmTilemap_getNew(&playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(playstate.pMap, gfx.pSset8x8, TM_MAX_WIDTH
            , TM_MAX_HEIGHT, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** If the playstate has been initialized, properly free it up. */
void freePlaystate() {
    if (playstate.pMap != 0) {
        gfmTilemap_free(&playstate.pMap);
    }
}

/** Updates the quadtree's bounds according to the currently loaded map */
static err _updateQuadtreeSize() {
    gfmRV rv;

    rv = gfmTilemap_getDimension(&playstate.width, &playstate.height
            , playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /** Make the quadtree 2 tiles larger than the actual map */
    playstate.width += 16;
    playstate.height += 16;

    return ERR_OK;
}

/** Setup the playstate so it may start to be executed */
err loadPlaystate() {
    gfmRV rv;
    err erv;

    /* TODO Decide where the first map will be loaded from */
    rv = gfmTilemap_loadf(playstate.pMap, game.pCtx, "levels/map_test_tm.gfm"
            , 22 , pDictNames, pDictTypes, dictLen);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = _updateQuadtreeSize();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** Update the playstate */
err updatePlaystate() {
    gfmRV rv;

    rv = gfmQuadtree_initRoot(collision.pQt, -8/*x*/, -8/*y*/, playstate.width
            , playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmQuadtree_populateTilemap(collision.pQt, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Draw the playstate */
err drawPlaystate() {
    gfmRV rv;

    rv = gfmTilemap_draw(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

