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
#define X(name, ...) \
    int name;
    SOUNDS_LIST
    SONGS_LIST
#undef X
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

#endif /* __BASE_SFX_H__ */

