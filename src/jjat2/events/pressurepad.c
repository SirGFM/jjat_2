/**
 * @file src/jjat2/events/pressurepad.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/type.h>
#include <jjat2/events/common.h>
#include <jjat2/events/pressurepad.h>
#include <jjat2/entity.h>
#include <GFraMe/gframe.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>
#include <string.h>
#include <stdint.h>

#define pressurepad_offx        0
#define pressurepad_offy        (-2)
#define pressurepad_width       32
#define pressurepad_height      8
#define pressurepad_objheight   (pressurepad_height + 2)
#define pressurepad_frame       334

enum enPressurePadState {
    DISABLED    = 0x0
  , PRESSING    = 0x1
  , PRESSED     = 0x2
  , STATE_MASK  = (0x03 << EV_LOCAL_SHIFT)
  , DID_COLLIDE = (0x80 << EV_LOCAL_SHIFT)
};

/**
 * Parse a pressure pad into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed a pressure pad
 */
err initPressurePad(entityCtx *pEnt, gfmParser *pParser) {
    gfmRV rv;
    int i, l, x, y;
    uint8_t lock;

    rv = gfmParser_getNumProperties(&l, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    i = 0;

    lock = 0;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (memcmp(pKey, "set_", 4) == 0) {
            lock |= _getLocalVar(pVal);
        }

        i++;
    }
    ASSERT(lock != 0, ERR_PARSINGERR);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    y -= pressurepad_height;

    rv = gfmSprite_init(pEnt->pSelf, x, y, pressurepad_width
            , pressurepad_objheight, gfx.pSset32x8, pressurepad_offx
            , pressurepad_offy, pEnt, T_PRESSURE_PAD);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_setFixed(pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Set its only frame */
    rv = gfmSprite_setFrame(pEnt->pSelf, pressurepad_frame);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Store the pressure pad's lock on the entity's flags */
    pEnt->flags = lock;

    return ERR_OK;
}

/**
 * Update the pressure pad's state (and set any local var if activated)
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdatePressurePad(entityCtx *pEnt) {
    gfmRV rv;
    uint32_t curState;

    curState = (pEnt->flags & STATE_MASK) >> EV_LOCAL_SHIFT;

    /* Update the "animation" of the pressure pad */
    if ((pEnt->flags & DID_COLLIDE) && curState != PRESSED) {
        curState++;

        rv = gfmSprite_setOffset(pEnt->pSelf, pressurepad_offx
                , pressurepad_offy + curState);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Set the local var */
        if (curState == PRESSED) {
            _localVars |= (pEnt->flags & EV_LOCAL_MASK);
        }
    }
    else if (!(pEnt->flags & DID_COLLIDE) && curState != DISABLED) {
        curState--;

        rv = gfmSprite_setOffset(pEnt->pSelf, pressurepad_offx
                , pressurepad_offy + curState);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Clear the flag */
        _localVars &= ~(pEnt->flags & EV_LOCAL_MASK);
    }

    pEnt->flags = (pEnt->flags & ~STATE_MASK);
    pEnt->flags |= curState << EV_LOCAL_SHIFT;
    pEnt->flags &= ~DID_COLLIDE;

    return ERR_OK;
}

/**
 * Draw a pressure pad
 *
 * @param  [ in]pEnt    The entity
 */
err drawPressurePad(entityCtx *pEnt) {
    gfmRV rv;
    int x, y;

    rv = gfmSprite_getPosition(&x, &y, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfm_drawTile(game.pCtx, gfx.pSset32x8, x, y, 317/*tile*/, 0/*flip*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_draw(pEnt->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Activate the pressure pad
 *
 * @param  [ in]pEnt    The entity
 */
void pressPressurePad(entityCtx *pEnt) {
    pEnt->flags |= DID_COLLIDE;
}

