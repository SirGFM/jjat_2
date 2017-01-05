/**
 * @file include/jjat2/enemies/walky.h
 */
#ifndef __JJAT2_WALKY_H__
#define __JJAT2_WALKY_H__

#include <base/error.h>

#include <jjat2/entity.h>

/**
 * Initialize a 'walky' entity
 *
 * @param [ in]pEnt The entity
 * @param [ in]x    Horizontal position (left!)
 * @param [ in]y    Vertical position (bottom!)
 */
err initWalky(entityCtx *pEnt, int x, int y);

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateWalky(entityCtx *pEnt);

/**
 * Set walky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateWalky(entityCtx *pEnt);

/**
 * Render a walky
 *
 * @param  [ in]pEnt The player
 */
err drawWalky(entityCtx *pEnt);

#endif /* __JJAT2_WALKY_H__ */

