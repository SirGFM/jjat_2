/**
 * @file src/base/loadstate.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/loadstate.h>
#include <base/sfx.h>
#include <conf/game.h>
#include <conf/sfx_list.h>
#include <GFraMe/gframe.h>

#include <string.h>

#define LOAD_TXT "  LOADING...  "
#define LOAD_DELAY 300
#define FILE_DELAY 100
#define SFX_BASE_PATH "assets/sfx/"
#define SONG_BASE_PATH "assets/songs/"

/** List of resources to be loaded */
static char* pResSrc[] = {
#define X(name, file) \
    SFX_BASE_PATH file,
    SOUNDS_LIST
#undef X
#define X(name, file) \
    SONG_BASE_PATH file,
    SONGS_LIST
#undef X
};

/** List of handles though which the loaded resources will be accessed */
static int* pResHnd[] = {
#define X(name, ...) \
    &sfx.name,
    SOUNDS_LIST
    SONGS_LIST
#undef X
};

/** List of types for the loaded resources */
static gfmAssetType pResType[] = {
#define X(...) \
    ASSET_AUDIO,
    SOUNDS_LIST
    SONGS_LIST
#undef X
};

/** Number of resources to be loaded */
static const int numAssets = sizeof(pResType) / sizeof(gfmAssetType);

/** Retrieve the name of the current resource being loaded */
static char *getCurrentResourceName() {
    int idx = loadstate.lastProgress;

    /* Strip the common part from the file name */
    if (idx < getSfxCount()) {
        return pResSrc[idx] + sizeof(SFX_BASE_PATH) - 1;
    }
    else {
        return pResSrc[idx] + sizeof(SONG_BASE_PATH) - 1;
    }
}

/** Calculate the top-left corner of a centered text */
static int centerText(int len) {
    return (V_WIDTH - len * loadstate.fontWidth) / 2;
}

/** Calculate the top-left corner of text */
static int getHeightFromBottom(int y) {
    return V_HEIGHT - y * loadstate.fontHeight;
}

/**
 * Initialize the loadstate so it start loading the resources in BG. The game
 * is responsible for calling this with the proper parameters. Failure to do so
 * will result in a empty and (even more) boring loading screen.
 *
 * @param [ in]pBitmapFont The spriteset with the desired bitmap font
 * @param [ in]offset Offset of the bitmap font within the spriteset
 */
err initLoadstate(gfmSpriteset *pBitmapFont, int offset) {
    gfmRV rv;

    loadstate.pBitmapFont = pBitmapFont;
    loadstate.offset = offset;
    loadstate.lastProgress = -1;

    rv = gfmText_getNew(&loadstate.pLoading);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    rv = gfmText_getNew(&loadstate.pCurFile);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);

    return ERR_OK;
}

/** If the loadstate has been initialized, properly free it up. */
void freeLoadstate() {
    gfmText_free(&loadstate.pLoading);
    gfmText_free(&loadstate.pCurFile);

    loadstate.pBitmapFont = 0;
}

/** Setup the loadstate so it may start to be executed */
err loadLoadstate() {
    gfmRV rv;

    ASSERT((loadstate.progress == 0), ERR_ALREADYLOADING);

    if (loadstate.pBitmapFont != 0) {
        rv = gfmSpriteset_getDimension(&loadstate.fontWidth
                , &loadstate.fontHeight, loadstate.pBitmapFont);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);

        rv = gfmText_init(loadstate.pLoading, centerText(sizeof(LOAD_TXT)-1)
                , getHeightFromBottom(4), sizeof(LOAD_TXT)-1, 1/*maxLines*/
                , LOAD_DELAY, 0/*bindToWorld*/, loadstate.pBitmapFont
                , loadstate.offset);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
        rv = gfmText_setText(loadstate.pLoading, LOAD_TXT, sizeof(LOAD_TXT)-1
                , 1/*copy*/);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    }

    rv = gfm_loadAssetsAsync(&loadstate.progress, game.pCtx, pResType, pResSrc
            , pResHnd, numAssets);
    ASSERT((rv != GFMRV_ASYNC_LOADER_THREAD_IS_RUNNING), ERR_ALREADYLOADING);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);

    return ERR_OK;
}

/** Update the loadstate */
err updateLoadstate() {
    gfmRV rv;

    if (((game.flags & CMD_LAZYLOAD) && loadstate.progress >= getSfxCount()) ||
            loadstate.progress >= numAssets) {
        /* TODO Set the proper next state */
        //game.nextState = ST_MENUSTATE;
        game.nextState = ST_PLAYSTATE;
        return ERR_OK;
    }

    /* Skip updating if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    /* Update the name of the resource getting loaded */
    if (loadstate.lastProgress != loadstate.progress) {
        char *pText;
        int len;

        loadstate.lastProgress = loadstate.progress;
        if (loadstate.lastProgress == numAssets) {
            /* progress was updated by the other thread, bail out */
            return ERR_OK;
        }
        pText = getCurrentResourceName();
        len = strlen(pText);

        rv = gfmText_init(loadstate.pCurFile, centerText(len)
                , getHeightFromBottom(3), len, 1/*maxLines*/, FILE_DELAY
                , 0/*bindToWorld*/, loadstate.pBitmapFont, loadstate.offset);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
        rv = gfmText_setText(loadstate.pCurFile, pText, len, 1/*copy*/);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    }

    /* Animate the load screen */
    rv = gfmText_update(loadstate.pLoading, game.pCtx);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    if (loadstate.lastProgress >= 0) {
        rv = gfmText_update(loadstate.pCurFile, game.pCtx);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    }

    if (gfmText_didFinish(loadstate.pLoading) == GFMRV_TRUE) {
        rv = gfmText_setText(loadstate.pLoading, LOAD_TXT, sizeof(LOAD_TXT)-1
                , 1/*copy*/);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    }

    return ERR_OK;
}

/** Draw the loadstate */
err drawLoadstate() {
    gfmRV rv;

    /* Skip rendering if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    /* TODO Render a black box behind the text */

    /* Render the load screen */
    rv = gfmText_draw(loadstate.pLoading, game.pCtx);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    if (loadstate.lastProgress >= 0) {
        rv = gfmText_draw(loadstate.pCurFile, game.pCtx);
        ASSERT((rv == GFMRV_OK), ERR_GFMERR);
    }

    return ERR_OK;
}

