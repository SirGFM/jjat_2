/**
 * @file src/base/loadstate.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/loadstate.h>
#include <base/sfx.h>
#include <conf/sfx_list.h>
#include <GFraMe/gframe.h>

static char* pResSrc[] = {
#define X(name, file) \
    "assets/sfx/" file,
    SOUNDS_LIST
#undef X
#define X(name, file) \
    "assets/songs/" file,
    SONGS_LIST
#undef X
};

static int* pResHnd[] = {
#define X(name, ...) \
    &sfx.name,
    SOUNDS_LIST
    SONGS_LIST
#undef X
};

static gfmAssetType pResType[] = {
#define X(...) \
    ASSET_AUDIO,
    SOUNDS_LIST
    SONGS_LIST
#undef X
};

static const int numAssets = sizeof(pResType) / sizeof(gfmAssetType);

/**
 * Initialize the loadstate so it start loading the resources in BG. The game
 * is responsible for calling this with the proper parameters. Failure to do so
 * will result in a empty and (even more) boring loading screen.
 *
 * @param [ in]pBitmapFont The spriteset with the desired bitmap font
 * @param [ in]offset Offset of the bitmap font within the spriteset
 */
err initLoadstate(gfmSpriteset *pBitmapFont, int offset) {
    loadstate.pBitmapFont = pBitmapFont;
    loadstate.offset = offset;

    return ERR_OK;
}

/** If the loadstate has been initialized, properly free it up. */
void freeLoadstate() {
    loadstate.pBitmapFont = 0;
}

/** Setup the loadstate so it may start to be executed */
err loadLoadstate() {
    err erv;

    ASSERT((loadstate.progress != 0), ERR_ALREADYLOADING);

    rv = gfm_loadAssetsAsync(&loadstate.progress, game.pCtx, pResType, pResSrc
            , pResHnd, numAssets);
    ASSERT((rv != GFMRV_ASYNC_LOADER_THREAD_IS_RUNNING), ERR_ALREADYLOADING);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);

    return ERR_OK;
}

/** Update the loadstate */
err updateLoadstate() {
    if ((game.flags & CMD_LAZYLOAD) && loadstate.progress > getSfxCount()) {
        /* TODO Set the proper next state */
        //game.nextState = ST_MENUSTATE;
        game.nextState = ST_PLAYSTATE;
        return ERR_OK;
    }

    /* Skip updating if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    /* TODO Animate the load screen */

    return ERR_OK;
}

/** Draw the loadstate */
err drawLoadstate() {
    /* Skip rendering if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    /* TODO Render the load screen */

    return ERR_OK;
}

