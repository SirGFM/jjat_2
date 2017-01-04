/**
 * @file include/jjat2/enemy.h
 *
 * Initialize and update every enemy
 */
#ifndef __JJAT2_ENEMY_H__
#define __JJAT2_ENEMY_H__

#include <base/error.h>

#include <conf/type.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmParser.h>

/**
 * Parse an enemy into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 * @param  [ in]t       Type of the parsed enemy
 */
err parseEnemy(entityCtx *pEnt, gfmParser *pParser, type t);

/**
 * Update an enemy's physics
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateEnemy(entityCtx *pEnt);

/**
 * Set the enemy's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateEnemy(entityCtx *pEnt);

/**
 * Draw an enemy
 *
 * @param  [ in]pEnt    The entity
 */
err drawEnemy(entityCtx *pEnt);

#endif /* __JJAT2_ENEMIES_H__ */

