/**
 * @file include/base/resource.h
 *
 * This module loads *resources in BG and keeps track of their handles.
 * Different from the rest of the engine, it may alloc some dynamic memory,
 * to keep track of resources loaded during the game. However, since most of the
 * resources should be loaded from a static list (and during the game's
 * initialization), it will most likely be useful during development (so the
 * game won't have to be recompiled to add/test new songs) and for modding.
 */
#ifndef __BASE_RESOURCE_H__
#define __BASE_RESOURCE_H__

/** Stores structures used when loading resources asynchronously */
struct stLoaderCtx {
    /** Temporary list of resource handles, passed to the loading thread */
    int **ppHandles;
    /** Temporary list of resources getting loaded. Declared as 'void*' to
     * avoid including 'GFraMe/gfram.h' */
    void *pResType;
    /** Keeps both pointers to the loading files (at the start of the array)
     * and the file names ('\0' ended) themself). */
    char **pFiles;
    /** Length, in elements, of ppHandles and pResType. Since pFiles store
     * pointers to its files as well as as the strings themselves, it's actually
     * much longer than len (it's at least "sizeof(char**)*len +
     * sum(len(filename))" */
    int len;
    /* Length, in bytes, of pFiles */
    int filesLen;
    /** Progress status of the loader */
    int progress;
    /** How many resources are currently being loaded */
    int numLoading;
};
typedef struct stLoaderCtx loaderCtx;

struct stResourceCtx {
    /** List of handles with loaded (or being loaded) resources. */
    int *pHandles;
    /** Store null-terminated songs sequentially. The starting index of any song
     * may be retrieved from pDynSong */
    char *pNameBuf;
    /** Indexes to the starting position of songs in pNameBuf */
    int *pDynSong;
    /** Everything required by the BG loader */
    loaderCtx loader;
    /* Number of handles currently stored in pHandles and pDynSong */
    int count;
    /* Length, in elements, of pHandles and pDynSong */
    int len;
    /* Length, in bytes, of pNameBuf */
    int nameBufLen;
    /* Number of bytes currently used in pNameBuf */
    int usedNameBuf;
};
typedef struct stResourceCtx resourceCtx;

/** Global resource context (declared on src/base/static.c) */
extern resourceCtx res;

#endif /* __BASE_RESOURCE_H__ */

