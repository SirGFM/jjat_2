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
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed a door
 */
err initDoor(entityCtx *pEnt, gfmParser *pParser);

#endif /* __EVENTS_DOOR_H__ */

