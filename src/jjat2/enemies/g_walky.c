/**
 * @file src/jjat2/enemies/g_walky.c
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <jjat2/entity.h>
#include <jjat2/fx_group.h>
#include <jjat2/hitbox.h>

#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>

#include <string.h>

/** Eye-of-sight attributes when facing right */
#define g_walky_view_offx   (-8)
#define g_walky_view_offy   (-8)
#define g_walky_view_width  80
#define g_walky_view_height 24

#define g_walky_width     6
#define g_walky_height    6
#define g_walky_offx      -1
#define g_walky_offy      -2

#define G_WALKY_FALL_TIME   16
#define G_WALKY_FALL_HEIGHT 4
#define G_WALKY_FALL_GRAV   JUMP_ACCELERATION(G_WALKY_FALL_TIME, G_WALKY_FALL_HEIGHT)

enum {
      GWALKY_DIDATTACK     = (EF_AVAILABLEF_FLAG << 0)
    , GWALKY_TRIGGERATTACK = (EF_AVAILABLEF_FLAG << 1)
};

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
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 */
err initGreenWalky(entityCtx *pEnt, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int flip, i, l, x, y;

    flip = 0;
    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getNumProperties(&l, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    i = 0;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        if (strcmp(pKey, "flipped") == 0) {
            if (strcmp(pVal, "true") == 0) {
                flip = 1;
            }
        }

        i++;
    }

    y -= g_walky_height;
    rv = gfmSprite_init(pEnt->pSelf, x, y, g_walky_width, g_walky_height
            , gfx.pSset8x8, g_walky_offx, g_walky_offy, pEnt, T_EN_G_WALKY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pGWalkyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setDirection(pEnt->pSelf, flip);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Spawn an "eye-of-sight" */
    if (flip) {
        /* Facing left */
        x = x + g_walky_width - g_walky_view_offx - g_walky_view_width;
        y = y + g_walky_height - g_walky_view_offy - g_walky_view_height;
    }
    else {
        /* Facing right */
        x += g_walky_view_offx;
        y += g_walky_view_offy;
    }
    pEnt->pSight = spawnFixedHitbox(pEnt, x, y, g_walky_view_width
            , g_walky_view_height, T_EN_G_WALKY_VIEW);
    ASSERT(pEnt->pSight, ERR_GFMERR);

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
    int flip, x, y;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    if (!(pEnt->flags & EF_ALIVE)) {
        /* Hold position if dead */
        rv = gfmSprite_setVerticalAcceleration(pEnt->pSelf, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    gfmSprite_getDirection(&flip, pEnt->pSelf);
    gfmSprite_getPosition(&x, &y, pEnt->pSelf);
    if (flip) {
        /* Facing left */
        x = x + g_walky_width - g_walky_view_offx - g_walky_view_width;
        y = y + g_walky_height - g_walky_view_offy - g_walky_view_height;
    }
    else {
        /* Facing right */
        x += g_walky_view_offx;
        y += g_walky_view_offy;
    }
    gfmObject_setPosition((gfmObject*)pEnt->pSight, x, y);

    /* Collide only if still alive */
    if (!(pEnt->flags & EF_ALIVE)) {
        pEnt->flags |= EF_SKIP_COLLISION;
    }

    if (pEnt->currentAnimation != ATTACK) {
        if (pEnt->flags & GWALKY_TRIGGERATTACK) {
            setEntityAnimation(pEnt, ATTACK, 0/*force*/);
            pEnt->flags &= ~GWALKY_TRIGGERATTACK;
        }
        pEnt->flags &= ~GWALKY_DIDATTACK;
    }

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
        if (gfmSprite_didAnimationJustChangeFrame(pEnt->pSelf) == GFMRV_TRUE
                && !(pEnt->flags & GWALKY_DIDATTACK)) {
            int frame;

            gfmSprite_getFrame(&frame, pEnt->pSelf);
            if (frame == 1557) {
                gfmSprite *pSpr;
                double vx;
                int dir, x, y;

                gfmSprite_getPosition(&x, &y, pEnt->pSelf);
                gfmSprite_getDirection(&dir, pEnt->pSelf);
                if (dir == DIR_LEFT) {
                    /* If flipped (i.e., facing left) */
                    x -= 4;
                    vx = -120;
                }
                else if (dir == DIR_RIGHT) {
                    x += g_walky_width;
                    vx = 120;
                }

                /* TODO Set damage within the type */
                /* If changed into the last attack frame, spawn the bullet */
                pSpr = spawnFx(x, y, 4/*w*/, 4/*h*/, 0/*dir*/, 0/*ttl*/
                        , FX_STAR_ATK, T_EN_G_WALKY_ATK);
                ASSERT(pSpr, ERR_GFMERR);
                gfmSprite_setOffset(pSpr, -1, -1);
                gfmSprite_setHorizontalVelocity(pSpr, vx);

                pEnt->flags |= GWALKY_DIDATTACK;
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

        /* Got hit on the back while defending */
        gfmSprite_getDirection(&dir, pEnt->pSelf);
        if (((col & gfmCollision_left) && dir == DIR_RIGHT)
                || ((col & gfmCollision_right) && dir == DIR_LEFT)) {
            return ERR_OK;
        }
    }
    else if (pEnt->currentAnimation == ATTACK) {
        return ERR_OK;
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

/**
 * Triggers g_walky's attack
 *
 * @param  [ in]pEnt The enemy
 */
void triggerGreenWalkyAttack(entityCtx *pEnt) {
    pEnt->flags |= GWALKY_TRIGGERATTACK;
}

