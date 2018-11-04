/**
 * @file include/conf/sfx_list.h
 *
 * Define the list of available textures and its respective spritesets.
 */
#ifndef __CONF_SFX_LIST_H__
#define __CONF_SFX_LIST_H__

/**
 * List of sound effects. Tuple with (dst, source-file).
 */
#define SOUNDS_LIST

/**
 * List of songs effects. Tuple with (dst, source-file).
 */
#define SONGS_LIST \
  X(stage00, "intro.mml") \
  X(stage01, "first-steps.mml") \
  X(stage02, "starting-to-move.mml")

#endif /* __CONF_SFX_LIST_H__ */

