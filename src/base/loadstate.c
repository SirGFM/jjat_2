/**
 * @file src/base/loadstate.c
 */
#include <base/error.h>
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

/**
 * Initialize the playstate so a level may be later loaded and played. The game
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

/** If the playstate has been initialized, properly free it up. */
void freeLoadstate() {
    loadstate.pBitmapFont = 0;
}

/** Setup the playstate so it may start to be executed */
err loadLoadstate() {
    return ERR_NOTIMPLEMENTED;
}

/** Update the playstate */
err updateLoadstate() {
    /* Skip updating if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    return ERR_OK;
}

/** Draw the playstate */
err drawLoadstate() {
    /* Skip rendering if not set up */
    if (loadstate.pBitmapFont == 0) {
        return ERR_OK;
    }

    return ERR_OK;
}

