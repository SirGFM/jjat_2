/**
 * @file include/jjat2/swordy.h
 */
#ifndef __JJAT2_SWORDY_H__
#define __JJAT2_SWORDY_H__

#include <jjat2/entity.h>

#include <GFraMe/gfmParser.h>

struct stSwordyCtx {
    /** Base object */
    entityCtx entity;
    /** Number of times swordy has already jumped */
    int jumpCount;
};
typedef struct stSwordyCtx swordyCtx;

/**
 * Initialize the swordy character
 *
 * @param  [ in]swordy The player to be initialized
 */
err initSwordy(swordyCtx *swordy);

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]swordy The player to be freed
 */
void freeSwordy(swordyCtx *swordy);

/**
 * Parse swordy into its position
 *
 * @param  [ in]swordy  The player
 * @param  [ in]pParser Parser that has just parsed a "swordy_pos"
 */
err parseSwordy(swordyCtx *swordy, gfmParser *pParser);

/**
 * Render a swordy
 *
 * @param  [ in]swordy The player
 */
err drawSwordy(swordyCtx *swordy);

/**
 * Update the object's physics.
 *
 * @param  [ in]swordy The player to be updated
 */
err preUpdateSwordy(swordyCtx *swordy);

/**
 * Set swordy's animation and fix its entity's collision.
 *
 * @param  [ in]swordy The player to be updated
 */
err postUpdateSwordy(swordyCtx *swordy);

#endif /* __JJAT2_SWORDY_H__ */

