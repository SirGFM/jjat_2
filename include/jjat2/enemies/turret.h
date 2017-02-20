/**
 * @file include/jjat2/enemies/turret.h
 */
#ifndef __JJAT2_TURRET__
#define __JJAT2_TURRET__

#include <base/error.h>
#include <GFraMe/gfmParser.h>
#include <jjat2/entity.h>

/**
 * Initialize a 'turret' entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 */
err initTurret(entityCtx *pEnt, gfmParser *pParser);

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateTurret(entityCtx *pEnt);

/**
 * Set turret's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateTurret(entityCtx *pEnt);

/**
 * Render a turret
 *
 * @param  [ in]pEnt The player
 */
err drawTurret(entityCtx *pEnt);

#endif /* __JJAT2_TURRET__ */

