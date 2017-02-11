/**
 * @file include/jjat2/enemies/walky.h
 */
#ifndef __JJAT2_WALKY_H__
#define __JJAT2_WALKY_H__

#include <base/error.h>
#include <jjat2/entity.h>
#include <GFraMe/gfmParser.h>

/**
 * Initialize a 'walky' entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 */
err initWalky(entityCtx *pEnt, gfmParser *pParser);

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

