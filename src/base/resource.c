/**
 * @file src/base/resource.c
 *
 * This module loads *resources in BG and keeps track of their handles.
 * Different from the rest of the engine, it may alloc some dynamic memory,
 * to keep track of resources loaded during the game. However, since most of the
 * resources should be loaded from a static list (and during the game's
 * initialization), it will most likely be useful during development (so the
 * game won't have to be recompiled to add/test new songs) and for modding.
 */
#include <base/resource.h>
#include <base/sfx.h>
#include <conf/sfx_list.h>
#include <GFraMe/gframe.h>
#include <string.h>
#include <stdlib.h>

/* Help set the default resource path in a common place */
#define SFX_BASE_PATH "assets/sfx/"
#define SONG_BASE_PATH "assets/songs/"

/** List of resources to be pre-loaded at the start of the game */
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

/**
 * Check whether a handle has already been loaded. If not, ERR_LOADINGRESOURCE
 * shall be returned.
 *
 * @param [ in]idx Index of the handle
 */
static err _checkHandleLoaded(int idx) {
    if (res.pHandles[idx] == -1) {
        return ERR_LOADINGRESOURCE;
    }
    return ERR_OK;
}

/**
 * Retrieve the name of a given dynamically loaded song.
 *
 * Since this only deals with dynamically loaded songs, the index is offset by
 * getSfxCount() + getSoundCount() from the handles list.
 *
 * @param [ in]idx Index of the song
 */
static char* _getDynSongName(int idx) {
    return res.pNameBuf + res.pDynSong[idx];
}

/**
 * Calculates the index in pHandles for a dynamically loaded song.
 *
 * @param [ in]idx Index of the song
 */
static int _convertDynSongIndex(int idx) {
    return idx + getSfxCount() + getSoundCount();
}

/**
 * Look through the pre-loaded list of songs and check if the desired song has
 * already been loaded. If not, ERR_LOADINGRESOURCE shall be returned. If it
 * does not exist in the pre-loaded list, ERR_INDEXOOB is returned instead.
 *
 * If either ERR_LOADINGRESOURCE or ERR_OK is returned, pIdx shall point to a
 * valid index, which may later be queried through getResourceHandle(idx).
 *
 * @param  [out]pIdx  Index of the song within the handle list.
 * @param  [ in]pName Name of the song getting searched. Must be '\0' terminated
 */
err fastGetSongIndex(int *pIdx, char *pName) {
    int tmp, idx = -1;

    ASSERT(pHnd != 0, ERR_ARGUMENTBAD);
    ASSERT(pName != 0, ERR_ARGUMENTBAD);

    while (tmp = getSfxCount(); tmp < getSfxCount() + getSoundCount(); tmp++) {
        char *pFilename = pResSrc[tmp] + sizeof(SONG_BASE_PATH) - 1;

        if (strcmp(pFilename, pName) == 0) {
            idx = tmp;
            break;
        }
    }

    /* TODO Change this into an ASSERT */
    /* This should be an ASSERT since no "default song" should be dynamically
     * loaded. Using an ASSERT here stops the debugger, which helps detecting if
     * I missed any songs. */
    //ASSERT((idx != -1, ERR_INDEXOOB);
    if (idx == -1) {
        return ERR_INDEXOOB;
    }

    /* Check if the asset has already been loaded */
    *pIdx = idx;
    return _checkHandleLoaded(idx);
}

/**
 * Check if a song has been dynamically loaded into the game and return the
 * index of its handle. If not found, but another resource is currently being
 * loaded, ERR_ALREADYLOADING shall be returned. If the songs has already been
 * added to the game but hasn't finished loading, ERR_LOADINGRESOURCE shall be
 * returned. If it does not exist in the pre-loaded list, ERR_INDEXOOB is
 * returned instead.
 *
 * If either ERR_LOADINGRESOURCE or ERR_OK is returned, pIdx shall point to a
 * valid index, which may later be queried through getResourceHandle(idx).
 *
 * @param  [out]pIdx  Index of the song within the handle list.
 * @param  [ in]pName Name of the song getting searched. Must be '\0' terminated
 */
err getDynSongIndex(int *pIdx, char *pName) {
    int idx, len;

    ASSERT(pHnd != 0, ERR_ARGUMENTBAD);
    ASSERT(pName != 0, ERR_ARGUMENTBAD);

    /* Check if the songs has already been loaded, or if it's currently being
     * loaded */
    for (idx = 0; idx < res.count; idx++) {
        if (strcmp(pName, _getDynSongName(idx)) == 0) {
            *pIdx = _convertDynSongIndex(idx);
            return _checkHandleLoaded(*pIdx);
        }
    }

    /* Check if anything else is currently being loaded */
    ASSERT(res.loader.progress == -1 ||
            res.loader.progress == res.loader.numLoading, ERR_ALREADYLOADING);

    len = strlen(pName);

    /* Ensure there's enough memory for the new song */
    if (res.count == res.len) {
        res.count = res.count * 2 + 1;

        res.pHandles = realloc(res.pHandles, sizeof(int) * res.count);
        ASSERT(res.pHandles, ERR_OOM);
        res.pDynSong = realloc(res.pDynSong, sizeof(int) * res.count);
        ASSERT(res.pDynSong, ERR_OOM);
    }
    if (res.usedNameBuf + len + 1 > res.nameBufLen) {
        res.nameBufLen = res.nameBufLen * 2 + len + 1;

        res.pNameBuf = realloc(res.pNameBuf, sizeof(char) * res.nameBufLen);
        ASSERT(res.pNameBuf, ERR_OOM);
    }

    /* Store the new song in the dynamic list.
     * NOTE: idx already points to the next index */
    memcpy(res.pNameBuf + res.usedNameBuf, pName + 1, len + 1);
    res.pDynSong[idx] = res.usedNameBuf;
    res.usedNameBuf += len + 1;
    res.count++;

    /* TODO Do actually load the file */
}

