/**
 * @file src/jjat2/enemies/spiky.c
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <jjat2/entity.h>
#include <jjat2/fx_group.h>
#include <jjat2/enemies/spiky.h>

#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>

#define spiky_width     4
#define spiky_height    6
#define spiky_offx      -2
#define spiky_offy      -2

#define SPIKY_DAMAGE      1
#define SPIKY_SPEED       TILES_TO_PX(4)
#define SPIKY_FALL_TIME   16
#define SPIKY_FALL_HEIGHT 4
#define SPIKY_FALL_GRAV   JUMP_ACCELERATION(SPIKY_FALL_TIME, SPIKY_FALL_HEIGHT)

enum {
      SPIKY_DIDATTACK = (EF_AVAILABLEF_FLAG << 0)
};

/** List of animations */
enum enSpikyAnim {
    STAND = 0
  , ATTACK
  , DEATH
  , SPIKY_ANIM_COUNT
};
typedef enum enSpikyAnim spikyAnim;

/** spiky animation data */
static int pSpikyAnimData[] = {
/*           len|fps|loop|data... */
/* STAND  */  2 , 8 ,  1 , 1562,1563
/* ATTACK */, 2 , 8 ,  1 , 1564,1563
/* DEATH  */, 4 , 8 ,  0 , 1564,1565,1566,1567
};

/**
 * Initialize a 'spiky' entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 */
err initSpiky(entityCtx *pEnt, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    y -= spiky_height;
    rv = gfmSprite_init(pEnt->pSelf, x, y, spiky_width, spiky_height
            , gfx.pSset8x8, spiky_offx, spiky_offy, pEnt
            , (T_EN_SPIKY | SPIKY_DAMAGE << T_BITS));
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pSpikyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    pEnt->maxAnimation = SPIKY_ANIM_COUNT;
    erv = setEntityAnimation(pEnt, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    pEnt->standGravity = SPIKY_FALL_GRAV;
    pEnt->fallGravity = SPIKY_FALL_GRAV;
    initEntity(pEnt);

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateSpiky(entityCtx *pEnt) {
    gfmRV rv;
    gfmCollision col;
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    gfmSprite_getCollision(&col, pEnt->pSelf);
    if (col & gfmCollision_left) {
        gfmSprite_setHorizontalVelocity(pEnt->pSelf, SPIKY_SPEED);
    }
    else if (col & gfmCollision_right) {
        gfmSprite_setHorizontalVelocity(pEnt->pSelf, -SPIKY_SPEED);
    }
    else if (col & gfmCollision_down) {
        flipEntityOnEdge(pEnt, SPIKY_SPEED);
    }

    if (!(pEnt->flags & EF_ALIVE)) {
        /* Hold position if dead */
        rv = gfmSprite_setVerticalAcceleration(pEnt->pSelf, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        gfmSprite_setHorizontalVelocity(pEnt->pSelf, 0);
    }

    /* Collide only if still alive */
    if (!(pEnt->flags & EF_ALIVE)) {
        pEnt->flags |= EF_SKIP_COLLISION;
    }

    erv = preUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);

    if (pEnt->currentAnimation == DEATH) {
        if (gfmSprite_didAnimationFinish(pEnt->pSelf) == GFMRV_TRUE) {
            pEnt->flags |= EF_DEACTIVATE;
        }
    } else if (pEnt->currentAnimation == ATTACK) {
        if (gfmSprite_didAnimationJustChangeFrame(pEnt->pSelf) == GFMRV_TRUE) {
            int frame;

            gfmSprite_getFrame(&frame, pEnt->pSelf);
            if (frame == 1563) {
                pEnt->flags &= ~SPIKY_DIDATTACK;
            }
        }
    }

    return ERR_OK;
}

/**
 * Set spiky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateSpiky(entityCtx *pEnt) {
    gfmCollision col;
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    erv = postUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(pEnt);

    gfmSprite_getCollision(&col, pEnt->pSelf);

    /* Set animation */
    if (!(pEnt->flags & EF_ALIVE)) {
        setEntityAnimation(pEnt, DEATH, 0/*force*/);
    }
    else if (col & gfmCollision_down) {
        setEntityAnimation(pEnt, ATTACK, 0/*force*/);
    }
    else {
        setEntityAnimation(pEnt, STAND, 0/*force*/);
    }

    return ERR_OK;
}

/**
 * Render a spiky
 *
 * @param  [ in]pEnt The player
 */
err drawSpiky(entityCtx *pEnt) {
    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    return drawEntity(pEnt);
}

