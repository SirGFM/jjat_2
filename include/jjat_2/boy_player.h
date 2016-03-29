/**
 * @file src/boy_player.c
 *
 */
#ifndef __BOY_PLAYER_H__
#define __BOY_PLAYER_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>

/**
 * Free all resources used by the boy sprite
 *
 * @return GFraMe return value
 */
void boyPl_free();

/**
 * Initialize (and alloc, if needed) the boy from the parser
 *
 * @param [ in]pParser The parser (already pointing to a 'boy' object)
 * @return             GFraMe return value
 */
gfmRV boyPl_init(gfmParser *pParser);

/**
 * Handle inputs from the user, updates the gfmSprite, collides againt the
 * quadtree and creates the attack hitbox
 *
 * @return GFraMe return value
 */
gfmRV boyPl_update();

/**
 * Handles special collision cases (e.g.: carrying/being carried by another
 * mob), updates the animation, handle being actually hurt(?)
 *
 * NOTE: Must be called after all sprites have collided
 *
 * @return GFraMe return value
 */
gfmRV boyPl_postUpdate();

/**
 * Draw the sprite
 *
 * @return GFraMe return value
 */
gfmRV boyPl_draw();

#endif /* __BOY_PLAYER_H__ */

