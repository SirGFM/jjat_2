/**
 * @file include/base/sfx.h
 *
 * Define the list of available textures and its respective spritesets.
 */
#ifndef __BASE_SFX_H__
#define __BASE_SFX_H__

#include <base/error.h>
#include <conf/sfx_list.h>

struct stSfxCtx {
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

