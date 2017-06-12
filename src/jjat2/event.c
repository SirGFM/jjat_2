/**
 * @file src/jjat2/event.c
 *
 * Initialize and update every event
 */
#include <base/error.h>
#include <base/game.h>

#include <conf/type.h>

#include <jjat2/event.h>
#include <jjat2/entity.h>

#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>

/**
 * Parse an event into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an event
 * @param  [ in]t       Type of the parsed event
 */
err parseEvent(entityCtx *pEnt, gfmParser *pParser, type t) {
    err erv;

    //switch (t & T_MASK) {
    //    case T_DOOR: erv = initDoor(pEnt, pParser); break;
    //    case T_PRESSURE_PAD: erv = initPressurePad(pEnt, pParser); break;
    //    default: {
    //        ASSERT(0, ERR_INVALIDTYPE);
    //    }
    //}
    //ASSERT(erv == ERR_OK, erv);

    pEnt->baseType = (t & T_BASE_MASK);

    return ERR_OK;
}

/**
 * Update an event's physics
 *
 * Mostly used to clear any previous state and add to the global quadtree.
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateEvent(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* TODO Implement */

    return ERR_OK;
}

/**
 * Set the event's animation and check any action that depended on collision
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateEvent(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* TODO Implement */

    return ERR_OK;
}

/**
 * Draw an event
 *
 * @param  [ in]pEnt    The entity
 */
err drawEvent(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* TODO Implement */

    return ERR_OK;
}

