/**
 * @file src/jjat2/playstate.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <jjat2/dictionary.h>
#include <jjat2/fx_group.h>
#include <jjat2/gunny.h>
#include <jjat2/playstate.h>
#include <jjat2/swordy.h>
#include <jjat2/teleport.h>

#include <string.h>

/** Initialize the playstate so a level may be later loaded and played */
err initPlaystate() {
    gfmRV rv;
    err erv;

    erv = initSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = initGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmParser_getNew(&playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

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
    if (playstate.pParser != 0) {
        gfmParser_free(&playstate.pParser);
    }
    freeSwordy(&playstate.swordy);
    freeGunny(&playstate.gunny);
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

/** Load the static quadtree */
static err _loadStaticQuadtree() {
    gfmRV rv;

    rv = gfmQuadtree_initRoot(collision.pStaticQt, -8/*x*/, -8/*y*/
            , playstate.width, playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_setStatic(collision.pStaticQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_populateTilemap(collision.pStaticQt, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

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

    rv = gfmParser_initStatic(playstate.pParser, game.pCtx,
            "levels/map_test_obj.gfm");
    ASSERT(erv == ERR_OK, erv);

    while (1) {
        char *type;
        err erv;

        rv = gfmParser_parseNext(playstate.pParser);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        rv = gfmParser_getIngameType(&type, playstate.pParser);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (strcmp(type, "swordy_pos") == 0) {
            erv = parseSwordy(&playstate.swordy, playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
        else if (strcmp(type, "gunny_pos") == 0) {
            erv = parseGunny(&playstate.gunny, playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
    }

    rv = gfmParser_reset(playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = _loadStaticQuadtree();
    ASSERT(erv == ERR_OK, erv);

    rv = gfmGroup_killAll(fx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    resetTeleporterTarget();

    return ERR_OK;
}

/** Update the playstate */
err updatePlaystate() {
    gfmRV rv;
    err erv;

    rv = gfmQuadtree_initRoot(collision.pQt, -8/*x*/, -8/*y*/, playstate.width
            , playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = preUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = preUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /* FX-group should be the last step of pre-update (so it runs after
     * everyting was spawned this frame) */
    erv = updateFxGroup();
    ASSERT(erv == ERR_OK, erv);

    erv = postUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = postUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** Draw the playstate */
err drawPlaystate() {
    gfmRV rv;
    err erv;

    rv = gfmTilemap_draw(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = drawGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);
    erv = drawSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmGroup_draw(fx, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

