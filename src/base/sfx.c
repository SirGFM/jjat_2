/**
 * @file src/base/sfx.c
 *
 * Define the list of available textures and its respective spritesets.
 */
#include <base/error.h>
#include <base/game.h>
#include <base/resource.h>
#include <base/sfx.h>
#include <base/static.h>
#include <conf/sfx_list.h>
#include <GFraMe/gframe.h>

/** How loud the song is. */
static double _songVolume = 1.0;
/** How loud the sound effects are. */
static double _sfxVolume = 1.0;

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
    sfx.pending = -1;

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

static inline volume _getVolume(double _volume) {
    return (volume)(_volume * 4);
}

static inline void _setVolume(double *_volume, volume v) {
    *_volume = v * 0.25;
}

volume getSongVolume() {
    return _getVolume(_songVolume);
}

err setSongVolume(volume v) {
    gfmRV rv = GFMRV_OK;
    int wasPaused = (_songVolume == 0.0);

    _setVolume(&_songVolume, v);

    if (_songVolume > 0 && sfx.pSong) {
        rv = gfm_setAudioVolume(game.pCtx, sfx.pSong, _songVolume);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        if (wasPaused)
            rv = gfm_resumeAudioHandle(game.pCtx, sfx.pSong);
    }
    else if (_songVolume == 0.0 && sfx.pSong)
        rv = gfm_pauseAudioHandle(game.pCtx, sfx.pSong);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

volume getSfxVolume() {
    return _getVolume(_sfxVolume);
}

err setSfxVolume(volume v) {
    _setVolume(&_sfxVolume, v);
    return ERR_OK;
}

/**
 * Actually start playing a song. If the song is already playing, do nothing.
 *
 * @param [ in]idx Index of the song
 */
static err _playSong(int idx) {
    gfmRV rv;
    int hnd;

    if (idx == sfx.curSong) {
        return ERR_OK;
    }
    sfx.curSong = idx;
    sfx.pending = -1;

    /* Stop the previous song */
    if (sfx.pSong != 0) {
        rv = gfm_stopAudio(sfx.pSong, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        sfx.pSong = 0;
    }

    hnd = getSongHandle(sfx.curSong);
    if (_songVolume == 0.0) {
        rv = gfm_playAudio(&sfx.pSong, game.pCtx, hnd, 0.01);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfm_pauseAudioHandle(game.pCtx, sfx.pSong);
    }
    else
        rv = gfm_playAudio(&sfx.pSong, game.pCtx, hnd, _songVolume);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

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

    if (idx != -1 && getSongHandle(idx) != -1) {
        return _playSong(idx);
    }

    return ERR_OK;
}

