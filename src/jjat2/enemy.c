/**
 * @file src/jjat2/enemies.c
 *
 * Initialize and update every enemy
 */
#include <base/error.h>
#include <base/game.h>

#include <conf/type.h>

#include <jjat2/enemy.h>
#include <jjat2/entity.h>

#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>

/**
 * Parse an enemy into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 * @param  [ in]t       Type of the parsed enemy
 */
err parseEnemy(entityCtx *pEnt, gfmParser *pParser, type t) {
    return ERR_OK;
}

/**
 * Update an enemy's physics
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateEnemy(entityCtx *pEnt) {
    return ERR_OK;
}

/**
 * Set the enemy's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateEnemy(entityCtx *pEnt) {
    return ERR_OK;
}

/**
 * Draw an enemy
 *
 * @param  [ in]pEnt    The entity
 */
err drawEnemy(entityCtx *pEnt) {
    return ERR_OK;
}

