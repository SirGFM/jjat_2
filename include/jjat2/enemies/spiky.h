/**
 * @file include/jjat2/enemies/spiky.h
 */
#ifndef __JJAT2_SPIKY_H__
#define __JJAT2_SPIKY_H__

#include <base/error.h>

#include <jjat2/entity.h>

/**
 * Initialize a 'spiky' entity
 *
 * @param [ in]pEnt The entity
 * @param [ in]x    Horizontal position (left!)
 * @param [ in]y    Vertical position (bottom!)
 */
err initSpiky(entityCtx *pEnt, int x, int y);

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateSpiky(entityCtx *pEnt);

/**
 * Set spiky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateSpiky(entityCtx *pEnt);

/**
 * Render a spiky
 *
 * @param  [ in]pEnt The player
 */
err drawSpiky(entityCtx *pEnt);

#endif /* __JJAT2_SPIKY_H__ */
