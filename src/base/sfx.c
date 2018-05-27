/**
 * @file src/base/sfx.c
 *
 * Define the list of available textures and its respective spritesets.
 */
#include <base/error.h>
#include <base/resource.h>
#include <base/sfx.h>
#include <base/static.h>
#include <conf/sfx_list.h>

enum enSfxCount {
#define X(name, ...) \
    SFX_HND_ ## name ,
    SOUNDS_LIST
#undef X
    SFX_MAX
};

enum enSongCount {
#define X(name, ...) \
    SNG_HND_ ## name ,
    SONGS_LIST
#undef X
    SNG_MAX
};

/**
 * Set every resource in an 'unloaded' state, so lazy loading may work.
 */
err initSfx() {
#define X(name, ...) sfx.name = -1;
    SOUNDS_LIST
#undef X

    sfx.curSong = -1;
    sfx.pending = 0;

    return ERR_OK;
}

/**
 * Get how many sound effects the game has.
 */
int getSfxCount() {
    return SFX_MAX;
}

/**
 * Get how many songs the game has.
 */
int getSoundCount() {
    return SNG_MAX;
}

/**
 * Actually start playing a song. If the song is already playing, do nothing.
 *
 * @param [ in]idx Index of the song
 */
static err _playSong(int idx) {
    int hnd;

    if (idx == sfx.curSong) {
        return ERR_OK;
    }
    sfx.curSong = idx;

    hnd = getSongHandle(sfx.curSong);
    /* TODO Do actually play the song */

    return ERR_OK;
}

/**
 * Check if a song is currently loaded and, if not, start loading it.
 *
 * @param  [ in]pName The name of the song
 */
err playSong(char *pName) {
    err erv;
    int idx;

    erv = fastGetSongIndex(&idx, pName);
    if (erv == ERR_INDEXOOB) {
        erv = getDynSongIndex(&idx, pName);
    }
    ASSERT(erv == ERR_OK || erv == ERR_LOADINGRESOURCE, erv);

    if (erv == ERR_LOADINGRESOURCE) {
        sfx.pending = idx;
    }
    else {
        return _playSong(idx);
    }

    return ERR_OK;
}

/**
 * If any song was left pending, start playing it
 */
err playPendingSong() {
    int idx = sfx.pending;

    if (idx && getSongHandle(idx) != -1) {
        sfx.pending = 0;
        return _playSong(idx);
    }

    return ERR_OK;
}

