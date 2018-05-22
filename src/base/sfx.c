/**
 * @file src/base/sfx.c
 *
 * Define the list of available textures and its respective spritesets.
 */
#include <base/error.h>
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
#define X(name, ...) \
    sfx.name = -1;
    SONGS_LIST
#undef X

    return ERR_OK;
}

