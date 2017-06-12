/**
 * @file include/jjat2/event.h
 *
 * Initialize and update every event
 */
#ifndef __JJAT2_EVENT_H__
#define __JJAT2_EVENT_H__

#include <base/error.h>
#include <conf/type.h>
#include <GFraMe/gfmParser.h>
#include <jjat2/entity.h>

/**
 * Parse an event into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an event
 * @param  [ in]t       Type of the parsed event
 */
err parseEvent(entityCtx *pEnt, gfmParser *pParser, type t);

/**
 * Update an event's physics
 *
 * Mostly used to clear any previous state and add to the global quadtree.
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateEvent(entityCtx *pEnt);

/**
 * Set the event's animation and check any action that depended on collision
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateEvent(entityCtx *pEnt);

/**
 * Draw an event
 *
 * @param  [ in]pEnt    The entity
 */
err drawEvent(entityCtx *pEnt);

#endif /* __JJAT2_EVENT_H__ */

