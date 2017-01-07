/**
 * @file src/jjat2/enemies/g_walky.c
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmSprite.h>

#define g_walky_width     6
#define g_walky_height    6
#define g_walky_offx      -1
#define g_walky_offy      -2

#define G_WALKY_FALL_TIME   16
#define G_WALKY_FALL_HEIGHT 4
#define G_WALKY_FALL_GRAV   JUMP_ACCELERATION(G_WALKY_FALL_TIME, G_WALKY_FALL_HEIGHT)

/** List of animations */
enum enGWalkyAnim {
    STAND = 0
  , DEFEND
  , ATTACK
  , DEATH
  , G_WALKY_ANIM_COUNT
};
typedef enum enGWalkyAnim gWalkyAnim;

/** g_walky animation data */
static int pGWalkyAnimData[] = {
/*           len|fps|loop|data... */
/* STAND  */  4 , 8 ,  1 , 1547,1548,1549,1550
/* DEFEND */, 4 , 8 ,  0 , 1551,1552,1553,1554
/* ATTACK */, 8 , 10,  0 , 1555,1556,1555,1556,1555,1556,1557,1557
/* DEATH  */, 4 , 8 ,  0 , 1558,1559,1560,1561
};

/**
 * Initialize a 'g_walky' entity
 *
 * @param [ in]pEnt The entity
 * @param [ in]x    Horizontal position (left!)
 * @param [ in]y    Vertical position (bottom!)
 */
err initGreenWalky(entityCtx *pEnt, int x, int y) {
    gfmRV rv;
    err erv;

    y -= g_walky_height;
    rv = gfmSprite_init(pEnt->pSelf, x, y, g_walky_width, g_walky_height
            , gfx.pSset8x8, g_walky_offx, g_walky_offy, pEnt, T_G_WALKY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pGWalkyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    pEnt->maxAnimation = G_WALKY_ANIM_COUNT;
    erv = setEntityAnimation(pEnt, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    pEnt->standGravity = G_WALKY_FALL_GRAV;
    pEnt->fallGravity = G_WALKY_FALL_GRAV;
    initEntity(pEnt);

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateGreenWalky(entityCtx *pEnt) {
    gfmRV rv;
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    if (!(pEnt->flags & EF_ALIVE)) {
        /* Hold position if dead */
        rv = gfmSprite_setVerticalAcceleration(pEnt->pSelf, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    /* Collide only if still alive */
    if (!(pEnt->flags & EF_ALIVE)) {
        pEnt->flags |= EF_SKIP_COLLISION;
    }

    /* TODO Attack? */

    erv = preUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);

    if (pEnt->currentAnimation == DEATH) {
        if (gfmSprite_didAnimationFinish(pEnt->pSelf) == GFMRV_TRUE) {
            pEnt->flags |= EF_DEACTIVATE;
        }
    } else if (pEnt->currentAnimation == DEFEND) {
        if (gfmSprite_didAnimationFinish(pEnt->pSelf) == GFMRV_TRUE) {
            setEntityAnimation(pEnt, ATTACK, 0/*force*/);
        }
    } else if (pEnt->currentAnimation == ATTACK) {
        if (gfmSprite_didAnimationJustChangeFrame(pEnt->pSelf) == GFMRV_TRUE) {
            int frame;

            gfmSprite_getFrame(&frame, pEnt->pSelf);
            if (frame == 1561) {
                /* TODO If changed into the last attack frame, spawn the bullet */
            }
        }
    }

    return ERR_OK;
}

/**
 * Set g_walky's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateGreenWalky(entityCtx *pEnt) {
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    erv = postUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(pEnt);

    /* Set animation */
    if (!(pEnt->flags & EF_ALIVE)) {
        setEntityAnimation(pEnt, DEATH, 0/*force*/);
    }
    else if (pEnt->currentAnimation != STAND) {
        /* Automatically reset every animation to stand */
        if (gfmSprite_didAnimationFinish(pEnt->pSelf) == GFMRV_TRUE) {
            setEntityAnimation(pEnt, STAND, 0/*force*/);
        }
    }

    return ERR_OK;
}

/**
 * Render a g_walky
 *
 * @param  [ in]pEnt The player
 */
err drawGreenWalky(entityCtx *pEnt) {
    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    return drawEntity(pEnt);
}

/**
 * Check if g_walky was actually attacked
 *
 * NOTE: gfmObject_justOverlaped must be called before this function to properly
 * set the collision flags!
 *
 * @param  [ in]pEnt      The player
 * @param  [ in]pAttacker The attacking object
 * @return                ERR_OK if hit, ERR_NOHIT otherwise
 */
err onGreenWalkyAttacked(entityCtx *pEnt, gfmObject *pAttacker) {
    gfmCollision col;

    gfmSprite_getCollision(&col, pEnt->pSelf);
    if (pEnt->currentAnimation == DEFEND) {
        int frame, dir;

        gfmSprite_getFrame(&frame, pEnt->pSelf);
        if (frame >= 1553) {
            return ERR_OK;
        }

        gfmSprite_getDirection(&dir, pEnt->pSelf);
        if (((col & gfmCollision_left) && !dir)
                || ((col & gfmCollision_right) && dir)) {
            return ERR_OK;
        }
    }

    if (col & gfmCollision_left) {
        gfmSprite_setDirection(pEnt->pSelf, 1/*flip*/);
    }
    else if (col & gfmCollision_right) {
        gfmSprite_setDirection(pEnt->pSelf, 0/*not-flipped*/);
    }
    setEntityAnimation(pEnt, DEFEND, 1/*force*/);

    return ERR_NOHIT;
}

