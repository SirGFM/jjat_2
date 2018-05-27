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
#include <base/game.h>
#include <base/resource.h>
#include <base/sfx.h>
#include <conf/sfx_list.h>
#include <GFraMe/gframe.h>
#include <string.h>
#include <stdlib.h>

/* Help set the default resource path in a common place */
#define SFX_BASE_PATH "assets/sfx/"
#define SONG_BASE_PATH "assets/songs/"

/* Helper enumerations used to count the number of sound effects and songs */
enum enSfxCount {
#define X(name, ...) SFX_HND_ ## name ,
    SOUNDS_LIST
#undef X
    SFX_MAX
};
enum enSongCount {
#define X(name, ...) SNG_HND_ ## name ,
    SONGS_LIST
#undef X
    SNG_MAX
};

/** List of resources to be pre-loaded at the start of the game */
static char* pResSrc[] = {
#define X(name, file) SFX_BASE_PATH file,
    SOUNDS_LIST
#undef X
#define X(name, file) SONG_BASE_PATH file,
    SONGS_LIST
#undef X
};

/* Static list of song handles */
static int _songHandleList[SNG_MAX];
/** List of handles though which the loaded resources will be accessed (actually
 * filled on initResource) */
static int* _pResHnd[SFX_MAX + SNG_MAX];
/** List of types for the loaded resources */
static gfmAssetType _pResType[SFX_MAX + SNG_MAX] = { ASSET_AUDIO } ;

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
 * Setup everything and start the BG thread loading the resources.
 *
 * @param [out]pHandles   List where the loaded handles shall be stored
 * @param [ in]ppFiles    List of files to be loaded
 * @param [ in]numHandles How many handles there are in the list
 */
static err startLoadingResources(int *pHandles, char **ppFiles, int numHandles) {
    gfmAssetType *pResType;
    gfmRV rv;
    int i, len;

    /* Check if anything else is currently being loaded */
    ASSERT(res.loader.progress == -1 ||
            res.loader.progress == res.loader.numLoading, ERR_ALREADYLOADING);

    /* Expand buffers as necessary */
    if (res.loader.len < numHandles) {
        res.loader.len = numHandles;

        res.loader.ppHandles = realloc(res.loader.ppHandles
                , sizeof(int**) * numHandles);
        ASSERT(res.loader.ppHandles, ERR_OOM);
        res.loader.pResType = realloc(res.loader.pResType
                , sizeof(gfmAssetType) * numHandles);
        ASSERT(res.loader.pResType, ERR_OOM);
    }
    pResType = (gfmAssetType*)res.loader.pResType;

    /* Setup the handles and calculate the memory required by the paths */
    len = 0;
    for (i = 0; i < numHandles; i++) {
        pHandles[i] = -1;
        res.loader.ppHandles[i] = pHandles + i;
        pResType[i] = ASSET_AUDIO;
        len += sizeof(char) * (strlen(ppFiles[i]) + sizeof(SONG_BASE_PATH))
                + sizeof(char**);
    }

    /* Setup the file paths */
    if (len > res.loader.filesLen) {
        res.loader.filesLen = len;

        res.loader.pFiles = realloc(res.loader.pFiles, len);
        ASSERT(res.loader.pFiles, ERR_OOM);
    }

    len = 0;
    for (i = 0; i < numHandles; i++) {
        char *pName;
        int nameLen;

        pName  = (char*)(res.loader.pFiles);
        pName += sizeof(char**) * numHandles + len;

        res.loader.pFiles[i] = pName;

        memcpy(pName, SONG_BASE_PATH, sizeof(SONG_BASE_PATH) - 1);
        pName += sizeof(SONG_BASE_PATH) - 1;

        nameLen = strlen(ppFiles[i]);
        memcpy(pName, ppFiles[i], nameLen);
        pName[nameLen] = '\0';

        len += sizeof(SONG_BASE_PATH) + nameLen;
    }

    res.loader.numLoading = numHandles;

    rv = gfm_loadAssetsAsync(&res.loader.progress, game.pCtx
            , pResType, res.loader.pFiles, res.loader.ppHandles
            , res.loader.numLoading);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);

    return ERR_OK;
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

    ASSERT(pIdx != 0, ERR_ARGUMENTBAD);
    ASSERT(pName != 0, ERR_ARGUMENTBAD);

    for (tmp = SFX_MAX; tmp < SFX_MAX + SNG_MAX; tmp++) {
        char *pFilename = pResSrc[tmp] + sizeof(SONG_BASE_PATH) - 1;

        if (strcmp(pFilename, pName) == 0) {
            idx = tmp - SFX_MAX;
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
    char *ppFiles[1];
    int idx, len;

    ASSERT(pIdx != 0, ERR_ARGUMENTBAD);
    ASSERT(pName != 0, ERR_ARGUMENTBAD);

    /* Check if the songs has already been loaded, or if it's currently being
     * loaded */
    for (idx = SNG_MAX; idx < res.count; idx++) {
        if (strcmp(pName, _getDynSongName(idx)) == 0) {
            *pIdx = idx;
            return _checkHandleLoaded(*pIdx);
        }
    }

    /* Check if anything else is currently being loaded */
    ASSERT(res.loader.progress == -1 ||
            res.loader.progress == res.loader.numLoading, ERR_ALREADYLOADING);

    len = strlen(pName);

    /* Ensure there's enough memory for the new song */
    if (res.count == res.len) {
        /* Initially, only a hard-coded list is used. If still on that point,
         * malloc everything and copy it */
        if (res.len == 0) {
            res.len = SNG_MAX + 1;

            res.pHandles = malloc(sizeof(int) * SNG_MAX);
            if (res.pHandles) {
                memcpy(res.pHandles, _songHandleList, sizeof(int) * SNG_MAX);
            }
        }
        else {
            res.len = res.len * 2 + 1;

            res.pHandles = realloc(res.pHandles, sizeof(int) * (res.len + SNG_MAX));
        }
        ASSERT(res.pHandles, ERR_OOM);

        res.pDynSong = realloc(res.pDynSong, sizeof(int) * res.len);
        ASSERT(res.pDynSong, ERR_OOM);
    }
    if (res.usedNameBuf + len + 1 > res.nameBufLen) {
        res.nameBufLen = res.nameBufLen * 2 + len + 1;

        res.pNameBuf = realloc(res.pNameBuf, sizeof(char) * res.nameBufLen);
        ASSERT(res.pNameBuf, ERR_OOM);
    }

    /* Store the new song in the dynamic list.
     * NOTE: idx already points to the next index */
    memcpy(res.pNameBuf + res.usedNameBuf, pName, len + 1);
    res.pDynSong[idx] = res.usedNameBuf;
    res.usedNameBuf += len + 1;
    res.count++;

    ppFiles[0] = res.pNameBuf + res.usedNameBuf;
    return startLoadingResources(res.pHandles + idx, ppFiles, 1/*numHandles*/);
}

/**
 * Setup the resources with the hard-coded/pre-initialized songs and start
 * loading it.
 */
err initResource() {
    gfmRV rv;
    int i = 0;

    /* Setup _pResHnd from sfx.* and from _songHandleList */
#define X(name, ...) _pResHnd[i++] = &sfx.name,
    SOUNDS_LIST
#undef X
    for (; i < SFX_MAX + SNG_MAX; i++) {
        _pResHnd[i] = _songHandleList + i - SFX_MAX;
    }
    /* Although res.pHandles initially point to _songHandleList, keeping
     * res.len == 0 causes it to be expanded on the firts unloaded resource */
    res.pHandles = _songHandleList;

    /* Start loading everything */
    res.loader.numLoading = SFX_MAX + SNG_MAX;

    rv = gfm_loadAssetsAsync(&res.loader.progress, game.pCtx, _pResType, pResSrc
            , _pResHnd, res.loader.numLoading);
    ASSERT((rv == GFMRV_OK), ERR_GFMERR);

    return ERR_OK;
}

/**
 * Release the context's resources.
 */
void cleanResource() {
    if (res.pHandles != _songHandleList) {
        free(res.pHandles);
    }
    free(res.pNameBuf);
    free(res.pDynSong);
    free(res.loader.ppHandles);
    free(res.loader.pResType);
    free(res.loader.pFiles);
}

