/**
 * @file include/jjat2/events/door.h
 */
#ifndef __EVENTS_DOOR_H__
#define __EVENTS_DOOR_H__

#include <base/error.h>
#include <jjat2/entity.h>
#include <GFraMe/gfmParser.h>

/**
 * Parse a door into the entity
 *
 * @param  [ in]pEnt       The entity
 * @param  [ in]pParser    Parser that has just parsed a door
 * @param  [ in]isVertical Parser that has just parsed a door
 */
err initDoor(entityCtx *pEnt, gfmParser *pParser, int isVertical);

/**
 * Add a door to the quadtree depending on its state
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateDoor(entityCtx *pEnt);

/**
 * Change the door's animation depending on the local variables
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateDoor(entityCtx *pEnt);

#endif /* __EVENTS_DOOR_H__ */

