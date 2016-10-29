/**
 * @file include/jjat2/gunny.h
 */
#ifndef __JJAT2_GUNNY_H__
#define __JJAT2_GUNNY_H__

#include <jjat2/entity.h>

#include <GFraMe/gfmParser.h>

struct stGunnyCtx {
    /** Base object */
    entityCtx entity;
};
typedef struct stGunnyCtx gunnyCtx;

/**
 * Initialize the gunny character
 *
 * @param  [ in]gunny The player to be initialized
 */
err initGunny(gunnyCtx *gunny);

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]gunny The player to be freed
 */
void freeGunny(gunnyCtx *gunny);

/**
 * Parse gunny into its position
 *
 * @param  [ in]gunny   The player
 * @param  [ in]pParser Parser that has just parsed a "gunny_pos"
 */
err parseGunny(gunnyCtx *gunny, gfmParser *pParser);

/**
 * Render a gunny
 *
 * @param  [ in]gunny The player
 */
err drawGunny(gunnyCtx *gunny);

/**
 * Update the object's physics.
 *
 * @param  [ in]gunny The player to be updated
 */
err preUpdateGunny(gunnyCtx *gunny);

/**
 * Set gunny's animation and fix its entity's collision.
 *
 * @param  [ in]gunny The player to be updated
 */
err postUpdateGunny(gunnyCtx *gunny);

#endif /* __JJAT2_GUNNY_H__ */

