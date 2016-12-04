/**
 * @file src/gunny.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>

#include <conf/type.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

#include <jjat2/fx_group.h>
#include <jjat2/gunny.h>

/** Define Gunny's physics constants. The first parameter is the time in
 * 60FPS-frames and the second is the jump height in 8px-tiles. */
#define GUNNY_FALL_TIME 14
#define GUNNY_JUMP_TIME 16
#define GUNNY_JUMP_HEIGHT 3.25
#define GUNNY_HOP_TIME 10
#define GUNNY_HOP_HEIGHT 1
#define GUNNY_JUMP_SPEED JUMP_SPEED(GUNNY_JUMP_TIME, GUNNY_JUMP_HEIGHT)
#define GUNNY_HOP_SPEED JUMP_SPEED(GUNNY_HOP_TIME, GUNNY_HOP_HEIGHT)
#define GUNNY_JUMP_GRAV JUMP_ACCELERATION(GUNNY_JUMP_TIME, GUNNY_JUMP_HEIGHT)
#define GUNNY_FALL_GRAV JUMP_ACCELERATION(GUNNY_FALL_TIME, GUNNY_JUMP_HEIGHT)
#define GUNNY_SPEED TILES_TO_PX(9.5)

#define BULLET_SPEED TILES_TO_PX(25)

#define gunny_width 6
#define gunny_height 10
#define gunny_offx -5
#define gunny_offy -6

enum {
    gunny_attack = 0x01,
};

/** List of animations */
enum enGunnyAnim {
    STAND = 0,
    RUN,
    JUMP,
    FLOAT,
    FALL,
    ATK,
    HURT,
    GUNNY_ANIM_COUNT
};
typedef enum enGunnyAnim gunnyAnim;

/** Gunny animation data */
static int pGunnyAnimData[] = {
/*           len|fps|loop|data... */
/*  STAND  */ 2 , 8 ,  1 , 64,65,
/*  RUN    */ 4 , 8 ,  1 , 66,67,68,69,
/*  JUMP   */ 2 , 8 ,  1 , 70,71,
/*  FLOAT  */ 1 , 8 ,  0 , 72,
/*  FALL   */ 2 , 8 ,  1 , 73,74,
/*   ATK   */ 4 , 12,  0 , 75,76,77,78,
/*   HURT  */ 2 , 8 ,  1 , 78,79
};

/**
 * Initialize the gunny character
 *
 * @param  [ in]gunny The player to be initialized
 */
err initGunny(gunnyCtx *gunny) {
    err erv;
    gfmRV rv;

    /* Initialize the sprite */
    rv = gfmSprite_getNew(&gunny->entity.pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(gunny->entity.pSelf, 0, 0, gunny_width, gunny_height
            , gfx.pSset16x16, gunny_offx, gunny_offy, gunny, T_GUNNY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(gunny->entity.pSelf, pGunnyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    gunny->entity.maxAnimation = GUNNY_ANIM_COUNT;
    erv = setEntityAnimation(&gunny->entity, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    gunny->entity.jumpGrace = DEF_JUMP_GRACE;
    gunny->entity.jumpVelocity = GUNNY_JUMP_SPEED;
    gunny->entity.shorthopVelocity = GUNNY_HOP_SPEED;
    gunny->entity.standGravity = GUNNY_JUMP_GRAV;
    gunny->entity.fallGravity = GUNNY_FALL_GRAV;

    return ERR_OK;
}

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]gunny The player to be freed
 */
void freeGunny(gunnyCtx *gunny) {
    if (gunny->entity.pSelf) {
        gfmSprite_free(&gunny->entity.pSelf);
    }
}

/**
 * Parse gunny into its position
 *
 * @param  [ in]gunny  The player
 * @param  [ in]pParser Parser that has just parsed a "gunny_pos"
 */
err parseGunny(gunnyCtx *gunny, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(gunny->entity.pSelf, x + gunny_width
            , y - gunny_height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = setEntityAnimation(&gunny->entity, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Render a gunny
 *
 * @param  [ in]gunny The player
 */
err drawGunny(gunnyCtx *gunny) {
    gfmRV rv;
    rv = gfmSprite_draw(gunny->entity.pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]gunny The player to be updated
 */
err preUpdateGunny(gunnyCtx *gunny) {
    gfmRV rv;
    err erv;

    /* Update horizontal movement */
    do {
        if (IS_PRESSED(gunnyLeft)) {
            rv = gfmSprite_setHorizontalVelocity(gunny->entity.pSelf
                    , -GUNNY_SPEED);
        }
        else if (IS_PRESSED(gunnyRight)) {
            rv = gfmSprite_setHorizontalVelocity(gunny->entity.pSelf
                    , GUNNY_SPEED);
        }
        else {
            rv = gfmSprite_setHorizontalVelocity(gunny->entity.pSelf, 0);
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    } while (0); /* Update horizontal movement */

    erv = updateEntityJump(&gunny->entity, input.gunnyJump.state);
    if (erv == ERR_DIDJUMP) {
        /* Set the error so the assert isn't triggered */
        erv = ERR_OK;
    }
    ASSERT(erv == ERR_OK, erv);

    /* Handle attack */
    do {
        if (DID_JUST_PRESS(gunnyAtk)
                && gunny->entity.currentAnimation != ATK) {
            gfmSprite *pBullet;
            double vx;
            int dir, x, y;

            gfmSprite_getPosition(&x, &y, gunny->entity.pSelf);
            gfmSprite_getDirection(&dir, gunny->entity.pSelf);
            y += 2;
            if (dir == 0) {
                x += 3;
                vx = BULLET_SPEED;
            }
            else {
                x -= 13;
                vx = -BULLET_SPEED;
            }

            /* TODO Add a maximum TTL */
            pBullet = spawnFx(x, y, 16/*w*/, 8/*h*/, dir, 0/*ttl*/
                    , FX_GUNNY_BULLET , T_TEL_BULLET);
            ASSERT(pBullet != 0, ERR_GFMERR);
            gfmSprite_setHorizontalVelocity(pBullet, vx);

            gunny->flags |= gunny_attack;
        }
    } while (0); /* Handle attack */

    rv = gfmSprite_update(gunny->entity.pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = collideEntity(&gunny->entity);
    ASSERT(erv == ERR_OK, erv);

    /* End attack animation */
    do {
        /* Clean flag as soon as animation stops */
        if (gunny->entity.currentAnimation == ATK
                && gfmSprite_didAnimationFinish(gunny->entity.pSelf)
                == GFMRV_TRUE) {
            gunny->flags &= ~gunny_attack;
        }
    } while (0); /* End attack animation */

    return ERR_OK;
}

/**
 * Set gunny's animation and fix its entity's collision.
 *
 * @param  [ in]gunny The player to be updated
 */
err postUpdateGunny(gunnyCtx *gunny) {
    double vx, vy;
    err erv;
    gfmCollision dir;
    int hasCarrier;

    hasCarrier = (gunny->entity.pCarrying != 0);

    erv = postUpdateEntity(&gunny->entity);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(&gunny->entity);

    gfmSprite_getVelocity(&vx, &vy, gunny->entity.pSelf);
    gfmSprite_getCollision(&dir, gunny->entity.pSelf);

    /* Set animation */
    if (gunny->flags & gunny_attack) {
        setEntityAnimation(&gunny->entity, ATK, 0/*force*/);
    }
    else if (hasCarrier) {
        setEntityAnimation(&gunny->entity, STAND, 0/*force*/);
    }
    else if (vy > FLOAT_SPEED) {
        setEntityAnimation(&gunny->entity, FALL, 0/*force*/);
    }
    else if (vy < -FLOAT_SPEED) {
        setEntityAnimation(&gunny->entity, JUMP, 0/*force*/);
    }
    else if ((dir & gfmCollision_down) == 0 && vy >= -FLOAT_SPEED && vy <= FLOAT_SPEED) {
        setEntityAnimation(&gunny->entity, FLOAT, 0/*force*/);
    }
    else if (vx != 0) {
        setEntityAnimation(&gunny->entity, RUN, 0/*force*/);
    }
    else {
        setEntityAnimation(&gunny->entity, STAND, 0/*force*/);
    }

    return ERR_OK;
}

