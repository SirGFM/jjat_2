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
#include <jjat2/hitbox.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>
#include <string.h>
#include <stdint.h>

#define door_offx       0
#define door_offy       0
#define door_width      8
#define door_height     32
#define door_f3_height0 12
#define door_f3_height1 8
#define door_f4_height0 8
#define door_f4_height1 6
#define door_f5_height  2

enum enDoorFrames {
    f0 = 313
  , f1 = 314
  , f2 = 315
  , f3 = 316
  , f4 = 317
  , f5 = 318
  , f6 = 319
};

enum enHorDoorFrames {
    hf0 = 207
  , hf1 = 222
  , hf2 = 223
  , hf3 = 238
  , hf4 = 239
  , hf5 = 254
  , hf6 = 255
};

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
/* OPENED  */  1, 0 ,  0 , f6
/* CLOSED  */, 1, 0 ,  0 , f0
/* OPENING */,10, 10,  0 , f1,f0,f1,f2,f2,f0,f3,f4,f5,f6
/* CLOSING */,10, 10,  0 , f5,f4,f2,f3,f3,f0,f1,f0,f1,f0
};

/** horizontal door animation data */
static int pHorDoorAnimData[] = {
/*           len|fps|loop|data... */
/* OPENED  */  1, 0 ,  0 , hf6
/* CLOSED  */, 1, 0 ,  0 , hf0
/* OPENING */,10, 10,  0 , hf1,hf0,hf1,hf2,hf2,hf0,hf3,hf4,hf5,hf6
/* CLOSING */,10, 10,  0 , hf5,hf4,hf2,hf3,hf3,hf0,hf1,hf0,hf1,hf0
};

/**
 * Parse a door into the entity
 *
 * @param  [ in]pEnt       The entity
 * @param  [ in]pParser    Parser that has just parsed a door
 * @param  [ in]isVertical Parser that has just parsed a door
 */
err initDoor(entityCtx *pEnt, gfmParser *pParser, int isVertical) {
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
                /* TODO Implement initial state 'opened' */
                ASSERT(0, ERR_NOTIMPLEMENTED);
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

    if (isVertical) {
        y -= door_height;

        rv = gfmSprite_init(pEnt->pSelf, x, y, door_width, door_height
                , gfx.pSset8x32, door_offx, door_offy, pEnt, T_DOOR);
    }
    else {
        y -= door_width;

        rv = gfmSprite_init(pEnt->pSelf, x, y, door_height, door_width
                , gfx.pSset32x8, door_offy, door_offx, pEnt, T_HDOOR);
    }
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_setFixed(pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (isVertical) {
        rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pDoorAnimData);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
    else {
        rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pHorDoorAnimData);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

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
    int x, y, frame;

    rv = gfmSprite_update(pEnt->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_getPosition(&x, &y, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getFrame(&frame, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    switch (frame) {
        case f0:
        case f1:
        case f2: {
            spawnTmpHitbox(pEnt, x, y, door_width, door_height, T_DOOR);
        } break;
        case f3: {
            spawnTmpHitbox(pEnt, x, y, door_width, door_f3_height0, T_DOOR);
            spawnTmpHitbox(pEnt, x, y + door_height - door_f3_height1
                    , door_width, door_f3_height1, T_DOOR);
        } break;
        case f4: {
            spawnTmpHitbox(pEnt, x, y, door_width, door_f4_height0, T_DOOR);
            spawnTmpHitbox(pEnt, x, y + door_height - door_f4_height1
                    , door_width, door_f4_height1, T_DOOR);
        } break;
        case f5: {
            spawnTmpHitbox(pEnt, x, y, door_width, door_f5_height, T_DOOR);
        } break;
        /* Horizontal doors are the same as regular ones, but with width and
         * height flipped (even the collision type can be the same) */
        case hf0:
        case hf1:
        case hf2: {
            spawnTmpHitbox(pEnt, x, y, door_height, door_width, T_DOOR);
        } break;
        case hf3: {
            spawnTmpHitbox(pEnt, x, y, door_f3_height1, door_width, T_DOOR);
            spawnTmpHitbox(pEnt, x + door_height - door_f3_height0, y
                    , door_f3_height0, door_width, T_DOOR);
        } break;
        case hf4: {
            spawnTmpHitbox(pEnt, x, y, door_f4_height1, door_width, T_DOOR);
            spawnTmpHitbox(pEnt, x + door_height - door_f4_height0, y
                    , door_f4_height0, door_width, T_DOOR);
        } break;
        case hf5: {
            spawnTmpHitbox(pEnt, x + door_height - door_f5_height, y
                    , door_f5_height, door_width, T_DOOR);
        } break;
        case f6:
        case hf6:
        default: { /* No collision */ }
    }

    return ERR_OK;
}

/**
 * Change the door's animation depending on the local variables
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateDoor(entityCtx *pEnt) {
    gfmRV rv;

    rv = gfmSprite_didAnimationFinish(pEnt->pSelf);
    if (rv == GFMRV_FALSE) {
        /* Exit early */
        return ERR_OK;
    }

    if ((pEnt->flags & _localVars) == pEnt->flags
            && pEnt->currentAnimation != OPENED) {
        return setEntityAnimation(pEnt, OPENING, 0/*force*/);
    }
    else if ((pEnt->flags & _localVars) != pEnt->flags
            && pEnt->currentAnimation != CLOSED) {
        return setEntityAnimation(pEnt, CLOSING, 0/*force*/);
    }

    return ERR_OK;
}

