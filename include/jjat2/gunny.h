/**
 * @file include/jjat2/gunny.h
 */
#ifndef __JJAT2_GUNNY_H__
#define __JJAT2_GUNNY_H__

#include <jjat2/entity.h>

#include <GFraMe/gfmParser.h>

#include <stdint.h>

#define gunny_width 6
#define gunny_height 10

#define BULLET_WIDTH 8

/**
 * Initialize the gunny character
 *
 * @param  [ in]gunny The player to be initialized
 */
err initGunny(entityCtx *gunny);

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]gunny The player to be freed
 */
void freeGunny(entityCtx *gunny);

/**
 * Set gunny's position based on a value retrieved from the parser
 *
 * @param  [ in]gunny The player
 * @param  [ in]x     The position
 * @param  [ in]y     The position
 */
void setGunnyPositionFromParser(entityCtx *gunny, int x, int y);

/**
 * Parse gunny into its position
 *
 * @param  [ in]gunny   The player
 * @param  [ in]pParser Parser that has just parsed a "gunny_pos"
 */
err parseGunny(entityCtx *gunny, gfmParser *pParser);

/**
 * Render a gunny
 *
 * @param  [ in]gunny The player
 */
err drawGunny(entityCtx *gunny);

/**
 * Update the object's physics.
 *
 * @param  [ in]gunny The player to be updated
 */
err preUpdateGunny(entityCtx *gunny);

/**
 * Set gunny's animation and fix its entity's collision.
 *
 * @param  [ in]gunny The player to be updated
 */
err postUpdateGunny(entityCtx *gunny);

/**
 * Check if gunny should be teleported and do so.
 *
 * @param  [ in]gunny The player to be teleported
 */
err updateGunnyTeleport(entityCtx *gunny);

#endif /* __JJAT2_GUNNY_H__ */

