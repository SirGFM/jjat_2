/**
 * @file src/jjat2/enemies/walky.c
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmSprite.h>

#define walky_width     6
#define walky_height    6
#define walky_offx      -1
#define walky_offy      -2

#define WALKY_FALL_TIME     3
#define WALKY_FALL_HEIGHT   1
#define WALKY_FALL_GRAV     JUMP_ACCELERATION(WALKY_FALL_TIME, WALKY_FALL_HEIGHT)

/** List of animations */
enum enWalkyAnim {
    STAND = 0
  , RUN
  , DEATH
  , WALKY_ANIM_COUNT
};
typedef enum enWalkyAnim walkyAnim;

/** walky animation data */
static int pWalkyAnimData[] = {
/*          len|fps|loop|data... */
/* STAND */  4 , 8 ,  1 , 1536,1537,1538,1539
/* RUN   */, 4 , 8 ,  1 , 1536,1540,1541,1542
/* DEATH */, 4 , 8 ,  0 , 1543,1544,1545,1546
};

/**
 * Initialize a 'walky' entity
 *
 * @param [ in]pEnt The entity
 * @param [ in]x    Horizontal position (left!)
 * @param [ in]y    Vertical position (bottom!)
 */
err initWalky(entityCtx *pEnt, int x, int y) {
    gfmRV rv;
    err erv;

    y -= walky_height;
    rv = gfmSprite_init(pEnt->pSelf, x, y, walky_width, walky_height
            , gfx.pSset8x8, walky_offx, walky_offy, pEnt, T_WALKY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pWalkyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    pEnt->maxAnimation = WALKY_ANIM_COUNT;
    erv = setEntityAnimation(pEnt, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

#if 0
    /* TODO Set all entity attributes */
    pEnt->standGravity = WALKY_FALL_GRAV;
    pEnt->fallGravity = WALKY_FALL_GRAV;
#else
    pEnt->standGravity = 0;
    pEnt->fallGravity = 0;
#endif
    initEntity(pEnt);

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateWalky(entityCtx *pEnt) {
    gfmCollision col;
    gfmRV rv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    rv = gfmSprite_getCollision(&col, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (!(col & gfmCollision_down)) {
        /* TODO Stand still until grounded again */
    }
    else if (col & gfmCollision_left) {
        /* TODO Move right */
    }
    else if (col & gfmCollision_right) {
        /* TODO Move left */
    }

    rv = gfmSprite_update(pEnt->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

#if 0
    /* TODO Collide walky */
    if (pEnt->flags & EF_ALIVE) {
        erv = collideEntity(pEnt);
        ASSERT(erv == ERR_OK, erv);
    }
#endif

    if (pEnt->currentAnimation == DEATH) {
        if (gfmSprite_didAnimationFinish(pEnt->pSelf) == GFMRV_TRUE) {
            pEnt->flags |= EF_DEACTIVATE;
        }
    }

    return ERR_OK;
}

/**
 * Set walky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateWalky(entityCtx *pEnt) {
    double vx, vy;
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    erv = postUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(pEnt);

    gfmSprite_getVelocity(&vx, &vy, pEnt->pSelf);

    /* Set animation */
    if (vy != 0) {
        setEntityAnimation(pEnt, STAND, 0/*force*/);
    }
    else if (!(pEnt->flags & EF_ALIVE)) {
        setEntityAnimation(pEnt, DEATH, 0/*force*/);
    }
    else if (vx != 0) {
        setEntityAnimation(pEnt, RUN, 0/*force*/);
    }
    else {
        setEntityAnimation(pEnt, STAND, 0/*force*/);
    }

    return ERR_OK;
}

/**
 * Render a walky
 *
 * @param  [ in]pEnt The player
 */
err drawWalky(entityCtx *pEnt) {
    gfmRV rv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    rv = gfmSprite_draw(pEnt->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

