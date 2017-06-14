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
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>
#include <string.h>
#include <stdint.h>

#define pressurepad_offx    0
#define pressurepad_offy    0
#define pressurepad_width   8
#define pressurepad_height  32
#define pressurepad_frame   334

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

    rv = gfmSprite_init(pEnt->pSelf, x, y, pressurepad_width, pressurepad_height
            , gfx.pSset32x8, pressurepad_offx, pressurepad_offy, pEnt
            , T_PRESSURE_PAD);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Set its only frame */
    rv = gfmSprite_setFrame(pEnt->pSelf, pressurepad_frame);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Store the pressure pad's lock on the entity's flags */
    pEnt->flags = lock;

    return ERR_OK;
}

