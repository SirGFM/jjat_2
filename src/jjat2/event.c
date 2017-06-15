/**
 * @file src/jjat2/event.c
 *
 * Initialize and update every event
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>

#include <conf/type.h>

#include <jjat2/event.h>
#include <jjat2/entity.h>
#include <jjat2/events/common.h>
#include <jjat2/events/door.h>
#include <jjat2/events/pressurepad.h>

#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>

#include <stdint.h>
#include <string.h>

/** Local variables that events may modify/check */
uint32_t _localVars;

/**
 * Clear the local variables common to all events.
 *
 * These variables are "local" because they should get cleared on screen
 * transition.
 */
void clearLocalVariables() {
    _localVars = 0;
}

/**
 * Parse an event into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an event
 * @param  [ in]t       Type of the parsed event
 */
err parseEvent(entityCtx *pEnt, gfmParser *pParser, type t) {
    err erv;

    switch (t & T_MASK) {
        case T_DOOR: erv = initDoor(pEnt, pParser); break;
        case T_PRESSURE_PAD: erv = initPressurePad(pEnt, pParser); break;
        default: {
            ASSERT(0, ERR_INVALIDTYPE);
        }
    }
    ASSERT(erv == ERR_OK, erv);

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
    err erv;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_DOOR: {
            erv = preUpdateDoor(pEnt);
            ASSERT(erv == ERR_OK, erv);
        } break;
        default: {
            rv = gfmSprite_update(pEnt->pSelf, game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            /* Collide against stuff */
            rv = gfmQuadtree_collideSprite(collision.pQt, pEnt->pSelf);
            if (rv == GFMRV_QUADTREE_OVERLAPED) {
                erv = doCollide(collision.pQt);
                ASSERT(erv == ERR_OK, erv);
                rv = GFMRV_QUADTREE_DONE;
            }
            ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
        }
    }

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
    err erv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_PRESSURE_PAD: {
            erv = postUpdatePressurePad(pEnt);
            ASSERT(erv == ERR_OK, erv);
        } break;
        default: {}
    }

    return ERR_OK;
}

/**
 * Draw an event
 *
 * @param  [ in]pEnt    The entity
 */
err drawEvent(entityCtx *pEnt) {
    void *pChild;
    err erv;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_PRESSURE_PAD: {
            erv = drawPressurePad(pEnt);
            ASSERT(erv == ERR_OK, erv);
        } break;
        default: {
            rv = gfmSprite_draw(pEnt->pSelf, game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        }
    }

    return ERR_OK;
}

/**
 * Convert a local variable into its value
 *
 * @param  [ in]pVal The value
 * @return The variable's value or 0, on failure
 */
uint8_t _getLocalVar(char *pVal) {
#define TEST(val) \
    if (memcmp(pVal, #val , 2) == 0) { \
        return EV_LOCAL_ ## val; \
    } \

    TEST(A)
    else TEST(B)
    else TEST(C)
    else TEST(D)

    return 0;
}

