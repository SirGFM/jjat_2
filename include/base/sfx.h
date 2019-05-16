/**
 * @file include/base/sfx.h
 *
 * Define the list of available textures and its respective spritesets.
 */
#ifndef __BASE_SFX_H__
#define __BASE_SFX_H__

#include <base/error.h>
#include <conf/sfx_list.h>
#include <GFraMe/core/gfmAudio_bkend.h>

enum enVolume {
    SFX_VOL_0
  , SFX_VOL_25
  , SFX_VOL_50
  , SFX_VOL_75
  , SFX_VOL_100
};
typedef enum enVolume volume;

struct stSfxCtx {
    /* Handle to the currently playing song (so it may be stopped) */
    gfmAudioHandle *pSong;
#define X(name, ...) int name;
    SOUNDS_LIST
#undef X
    /** Index of the currently playing song. */
    int curSong;
    /** Whether 'curSong' still has to be started (e.g., if it were loading when
     * playSong() was called) */
    int pending;
};
typedef struct stSfxCtx sfxCtx;

/** Global sound context (declared on src/base/static.c) */
extern sfxCtx sfx;

/**
 * Set every resource in an 'unloaded' state, so lazy loading may work.
 */
err initSfx();

/**
 * Get how many sound effects the game has.
 */
int getSfxCount();

/**
 * Get how many songs the game has.
 */
int getSoundCount();

/** Get the current volume for the song. */
volume getSongVolume();

/**
 * Set the volume for the song.
 *
 * @param [ in]v The volume
 */
err setSongVolume(volume v);

/**
 * Check if a song is currently loaded and, if not, start loading it.
 *
 * @param  [ in]pName The name of the song
 */
err playSong(char *pName);

/**
 * If any song was left pending, start playing it
 */
err playPendingSong();

#endif /* __BASE_SFX_H__ */

