/**
 * @file include/jjat2/swordy.h
 */
#ifndef __JJAT2_SWORDY_H__
#define __JJAT2_SWORDY_H__

#include <jjat2/entity.h>

#include <GFraMe/gfmParser.h>

#define swordy_width 6
#define swordy_height 12

/**
 * Initialize the swordy character
 *
 * @param  [ in]swordy The player to be initialized
 */
err initSwordy(entityCtx *swordy);

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]swordy The player to be freed
 */
void freeSwordy(entityCtx *swordy);

/**
 * Set swordy's position based on a value retrieved from the parser
 *
 * @param  [ in]swordy The player
 * @param  [ in]x      The position
 * @param  [ in]y      The position
 */
void setSwordyPositionFromParser(entityCtx *swordy, int x, int y);

/**
 * Parse swordy into its position
 *
 * @param  [ in]swordy  The player
 * @param  [ in]pParser Parser that has just parsed a "swordy_pos"
 */
err parseSwordy(entityCtx *swordy, gfmParser *pParser);

/**
 * Render a swordy
 *
 * @param  [ in]swordy The player
 */
err drawSwordy(entityCtx *swordy);

/**
 * Update the object's physics.
 *
 * @param  [ in]swordy The player to be updated
 */
err preUpdateSwordy(entityCtx *swordy);

/**
 * Set swordy's animation and fix its entity's collision.
 *
 * @param  [ in]swordy The player to be updated
 */
err postUpdateSwordy(entityCtx *swordy);

#endif /* __JJAT2_SWORDY_H__ */

