/**
 * @file src/jjat2/enemies/turret.c
 */

#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <jjat2/entity.h>
#include <jjat2/fx_group.h>
#include <jjat2/enemies/turret.h>

#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>

#include <string.h>

#define turret_width     6
#define turret_height    6
#define turret_offx      -1
#define turret_offy      -2

#define TURRET_FALL_GRAV    0
#define TURRET_COOLDOWN     1000

#if 0
/** List of animations */
enum enTurretAnim {
    STAND = 0
  , TURRET_ANIM_COUNT
};
typedef enum enTurretAnim turretAnim;

/** turret animation data */
static int pTurretAnimData[] = {
/*           len|fps|loop|data... */
/* STAND  */  1 , 0 ,  0 , 1600
};
#endif

/**
 * Initialize a 'turret' entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 */
err initTurret(entityCtx *pEnt, gfmParser *pParser) {
    gfmRV rv;
#if 0
    err erv;
#endif
    int dir, i, l, x, y;

    dir = 0;
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
                dir = 1;
            }
        }

        i++;
    }

    y -= turret_height;
    rv = gfmSprite_init(pEnt->pSelf, x, y, turret_width, turret_height
            , gfx.pSset8x8, turret_offx, turret_offy, pEnt, T_EN_TURRET);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#if 0
    rv = gfmSprite_addAnimationsStatic(pEnt->pSelf, pTurretAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#endif
    rv = gfmSprite_setDirection(pEnt->pSelf, dir);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
#if 0
    pEnt->maxAnimation = TURRET_ANIM_COUNT;
    erv = setEntityAnimation(pEnt, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);
#else
    pEnt->maxAnimation = 0;
    rv = gfmSprite_setFrame(pEnt->pSelf, 1600);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#endif

    /* Set all entity attributes */
    pEnt->standGravity = TURRET_FALL_GRAV;
    pEnt->fallGravity = TURRET_FALL_GRAV;
    initEntity(pEnt);

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]pEnt The entity
 */
err preUpdateTurret(entityCtx *pEnt) {
    gfmRV rv;
    err erv;
    uint16_t cooldown;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    if (!(pEnt->flags & EF_ALIVE)) {
        /* Hold position if dead */
        rv = gfmSprite_setVerticalAcceleration(pEnt->pSelf, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    /* Retrieve the cooldown from the flags */
    cooldown = (pEnt->flags >> EF_AVAILABLE_BIT) & 0xff;
    cooldown <<= 1;

    cooldown += game.elapsed;
    if (cooldown >= TURRET_COOLDOWN) {
        gfmSprite *pSpr;
        double vx;
        int dir, x, y;

        gfmSprite_getDirection(&dir, pEnt->pSelf);
        gfmSprite_getPosition(&x, &y, pEnt->pSelf);
        if (dir == DIR_LEFT) {
            x -= 4;
            vx = -120;
        }
        else if (dir == DIR_RIGHT) {
            x += turret_width;
            vx = 120;
        }
        else {
            /* Should never happen, but avoids a warning and triggers if
             * gfmSprite_getDirection ever gets modified and breaks
             * compatibility */
            ASSERT(0, ERR_UNEXPECTEDBEHAVIOUR);
        }

        pSpr = spawnFx(x, y, 4/*w*/, 4/*h*/, 0/*dir*/, 0/*ttl*/
                , FX_STAR_ATK, T_EN_G_WALKY_ATK);
        ASSERT(pSpr, ERR_GFMERR);
        gfmSprite_setOffset(pSpr, -1, -1);
        gfmSprite_setHorizontalVelocity(pSpr, vx);

        cooldown -= TURRET_COOLDOWN;
    }

    /* Store it back (as half the accumulated timed */
    pEnt->flags &= ~(0xff << EF_AVAILABLE_BIT);
    cooldown >>= 1;
    pEnt->flags |= ((cooldown & 0xff) << EF_AVAILABLE_BIT);

    /* Collide only if still alive */
    if (!(pEnt->flags & EF_ALIVE)) {
        pEnt->flags |= EF_SKIP_COLLISION;
    }

    erv = preUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Set turret's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt The player to be updated
 */
err postUpdateTurret(entityCtx *pEnt) {
    err erv;

    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    erv = postUpdateEntity(pEnt);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Render a turret
 *
 * @param  [ in]pEnt The player
 */
err drawTurret(entityCtx *pEnt) {
    if (pEnt->flags & EF_DEACTIVATE) {
        return ERR_OK;
    }

    return drawEntity(pEnt);
}

