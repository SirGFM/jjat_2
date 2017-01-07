/**
 * @file include/jjat2/enemies/g_walky.h
 */
#ifndef __JJAT2_G_WALKY_H__
#define __JJAT2_G_WALKY_H__

#include <base/error.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmObject.h>

/**
 * Initialize a 'g_walky' entity
 *
 * @param [ in]pEnt The entity
 * @param [ in]x    Horizontal position (left!)
 * @param [ in]y    Vertical position (bottom!)
 */
err initGreenWalky(entityCtx *pEnt, int x, int y);

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateGreenWalky(entityCtx *pEnt);

/**
 * Set g_walky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateGreenWalky(entityCtx *pEnt);

/**
 * Render a g_walky
 *
 * @param  [ in]pEnt The player
 */
err drawGreenWalky(entityCtx *pEnt);

/**
 * Check if g_walky was actually attacked
 *
 * NOTE: gfmObject_justOverlaped must be called before this function to properly
 * set the collision flags!
 *
 * @param  [ in]pEnt      The player
 * @param  [ in]pAttacker The attacking object
 * @return                ERR_OK if hit, ERR_NOHIT otherwise
 */
err onGreenWalkyAttacked(entityCtx *pEnt, gfmObject *pAttacker);

#endif /* __JJAT2_G_WALKY_H__ */

