/**
 * @file src/jjat2/events/door.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <conf/type.h>
#include <jjat2/events/common.h>
#include <jjat2/events/door.h>
#include <jjat2/entity.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <string.h>
#include <stdint.h>

#define door_offx   0
#define door_offy   0
#define door_width  8
#define door_height 32

/** List of animations */
enum enDoorAnim {
    OPENED = 0
  , CLOSED
  , OPENING
  , CLOSING
  , DOOR_ANIM_COUNT
};
typedef enum enDoorAnim doorAnim;

/** door animation data */
static int pDoorAnimData[] = {
/*           len|fps|loop|data... */
/* OPENED  */  1, 0 ,  0 , 319
/* CLOSED  */, 1, 0 ,  0 , 313
/* OPENING */,10, 10,  0 , 314,313,314,315,315,313,316,317,318,319
/* CLOSING */,10, 10,  0 , 318,317,315,316,316,313,314,313,314,313
};

/**
 * Parse a door into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed a door
 */
err initDoor(entityCtx *pEnt, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int i, l, x, y;
    uint8_t anim;
    uint8_t lock;

    rv = gfmParser_getNumProperties(&l, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    i = 0;

    lock = 0;
    anim = DOOR_ANIM_COUNT;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (memcmp(pKey, "init_state", 11) == 0) {
            if (memcmp(pVal, "closed", 7) == 0) {
                anim = CLOSED;
            }
            else if (memcmp(pVal, "opened", 7) == 0) {
                anim = OPENED;
            }
        }
        else if (memcmp(pKey, "lock_", 5) == 0) {
            lock |= _getLocalVar(pVal);
        }

        i++;
    }
    ASSERT(anim != DOOR_ANIM_COUNT, ERR_PARSINGERR);
    ASSERT(lock != 0, ERR_PARSINGERR);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    y -= door_height;

    rv = gfmSprite_init(pEnt->pSelf, x, y, door_width, door_height
            , gfx.pSset8x32, door_offx, door_offy, pEnt, T_DOOR);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_setFixed(pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pDoorAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    pEnt->maxAnimation = DOOR_ANIM_COUNT;
    erv = setEntityAnimation(pEnt, anim, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Store the door's lock on the entity's flags */
    pEnt->flags = lock;

    return ERR_OK;
}

/**
 * Add a door to the quadtree depending on its state
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateDoor(entityCtx *pEnt) {
    gfmRV rv;
    err erv;
    uint8_t curAnim;

    rv = gfmSprite_update(pEnt->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_didAnimationFinish(pEnt->pSelf);
    if (rv == GFMRV_TRUE) {
        curAnim = pEnt->currentAnimation;
    }
    else {
        /* Defaults to closed, if the animation is still running */
        curAnim = CLOSED;
    }

    switch (curAnim) {
        case OPENED:
        case OPENING: {
            /* Do nothing, since the door is open */
        } break;
        case CLOSED:
        case CLOSING:
        default: {
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

