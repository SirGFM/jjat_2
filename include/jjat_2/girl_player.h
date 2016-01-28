/**
 * @file src/girl_player.c
 *
 * Messy implementation (read: ugly and not generic) of the girl. This is mostly
 * a starting point so I can get the feel for what is and isn't need when coding
 * all mobs properly.
 *
 * Things I must have answered by the end of this (probably after also
 * implementing the boy):
 *  - How will everything update?
 *  - How will everything collide?
 *  - How do I handle collision against environment and 'moving platforms'?
 *  - How do I properly handle animations through various different sprites?
 *  - How do mobs carry the teleporter target?
 *  - ...
 */
#ifndef __GIRL_PLAYER_H__
#define __GIRL_PLAYER_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>

/**
 * Free all resources used by the girl sprite
 *
 * @return GFraMe return value
 */
void grlPl_free();

/**
 * Initialize (and alloc, if needed) the girl from the parser
 *
 * @param [ in]pParser The parser (already pointing to a 'girl' object)
 * @return             GFraMe return value
 */
gfmRV grlPl_init(gfmParser *pParser);

/**
 * Handle inputs from the user, updates the gfmSprite, collides againt the
 * quadtree and creates the attack hitbox
 *
 * @return GFraMe return value
 */
gfmRV grlPl_update();

/**
 * Handles special collision cases (e.g.: carrying/being carried by another
 * mob), updates the animation, handle being actually hurt(?)
 *
 * NOTE: Must be called after all sprites have collided
 *
 * @return GFraMe return value
 */
gfmRV grlPl_postUpdate();

/**
 * Draw the sprite
 *
 * @return GFraMe return value
 */
gfmRV grlPl_draw();

#endif /* __GIRL_PLAYER_H__ */

