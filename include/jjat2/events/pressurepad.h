/**
 * @file include/jjat2/events/pressurepad.h
 */
#ifndef __EVENTS_PRESSURE_PAD_H__
#define __EVENTS_PRESSURE_PAD_H__

#include <base/error.h>
#include <jjat2/entity.h>
#include <GFraMe/gfmParser.h>

/**
 * Parse a pressure pad into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed a pressure pad
 */
err initPressurePad(entityCtx *pEnt, gfmParser *pParser);

/**
 * Update the pressure pad's state (and set any local var if activated)
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdatePressurePad(entityCtx *pEnt);

/**
 * Draw a pressure pad
 *
 * @param  [ in]pEnt    The entity
 */
err drawPressurePad(entityCtx *pEnt);

/**
 * Activate the pressure pad
 *
 * @param  [ in]pEnt    The entity
 */
void pressPressurePad(entityCtx *pEnt);

#endif /* __EVENTS_PRESSURE_PAD_H__ */

