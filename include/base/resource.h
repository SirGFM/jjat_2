/**
 * @file include/base/resource.h
 *
 * Although called "resources", this "module" mostly deals with songs. Every
 * other resource is either unique (GFX atlas and spritesets) or is loaded once
 * at the start (SFX).
 */
#ifndef __BASE_RESOURCE_H__
#define __BASE_RESOURCE_H__

struct stResourceCtx {
    /** List of handles with loaded (or being loaded) resources */
    int *pHandles;
    /** Temporary list of resource handles, passed to the loading thread */
    int **ppTBLHandles;
    /** Ugly buffer with every song name. Should be used for storage only, and
     * pDynSongNames should actually be used to access its elements */
    char *pNameBuf;
    /** Name of songs dynamically loaded during execution. Each pointer
     * properlly points to a null-terminated string. */
    char **pDynSongNames;
    /** Number of handles currently used in pHandles */
    int numHandles;
    /** Actual length (in elements) of pHandles */
    int handlesLen;
    /** Length (in elements) of ppTBLHandles */
    int tmpListLen;
    /** Actual length of pNameBuf */
    int nameBufLen;
    /** Number of songs dynamically loaded into pDynSongNames */
    int numDynSongs;
    /** Actual length (in elements) of pDynSongNames */
    int dynSongLen;
};
typedef struct stResourceCtx resourceCtx;

/** Global resource context (declared on src/base/static.c) */
extern resourceCtx res;

#endif /* __BASE_RESOURCE_H__ */

