/**
 * @file src/gunny.c
 */
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>

#include <conf/type.h>

#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

#include <jjat2/fx_group.h>
#include <jjat2/gunny.h>
#include <jjat2/teleport.h>

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
      gunny_attack       = (EF_AVAILABLEF_FLAG << 0)
    , gunny_justAttacked = (EF_AVAILABLEF_FLAG << 1)
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
err initGunny(entityCtx *gunny) {
    err erv;
    gfmRV rv;

    /* Initialize the sprite */
    rv = gfmSprite_getNew(&gunny->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(gunny->pSelf, 0, 0, gunny_width, gunny_height
            , gfx.pSset16x16, gunny_offx, gunny_offy, gunny, T_GUNNY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(gunny->pSelf, pGunnyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    gunny->maxAnimation = GUNNY_ANIM_COUNT;
    erv = setEntityAnimation(gunny, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    gunny->jumpGrace = DEF_JUMP_GRACE;
    gunny->jumpVelocity = GUNNY_JUMP_SPEED;
    gunny->shorthopVelocity = GUNNY_HOP_SPEED;
    gunny->standGravity = GUNNY_JUMP_GRAV;
    gunny->fallGravity = GUNNY_FALL_GRAV;
    initEntity(gunny);

    return ERR_OK;
}

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]gunny The player to be freed
 */
void freeGunny(entityCtx *gunny) {
    if (gunny->pSelf) {
        gfmSprite_free(&gunny->pSelf);
    }
}

/**
 * Parse gunny into its position
 *
 * @param  [ in]gunny  The player
 * @param  [ in]pParser Parser that has just parsed a "gunny_pos"
 */
err parseGunny(entityCtx *gunny, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(gunny->pSelf, x + gunny_width, y - gunny_height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = setEntityAnimation(gunny, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Render a gunny
 *
 * @param  [ in]gunny The player
 */
err drawGunny(entityCtx *gunny) {
    gfmRV rv;

    gfmDebug_printf(game.pCtx, 0, 72, "GUNNY ALIVE: %i", gunny->flags & EF_ALIVE);

    if (game.flags & AC_GUNNY) {
        rv = gfmSprite_draw(gunny->pSelf, game.pCtx);
    }
    else {
        int cx, cy, frame, isFlipped, x, y;

        gfmCamera_getPosition(&cx, &cy, game.pCamera);
        gfmSprite_getPosition(&x, &y, gunny->pSelf);
        gfmSprite_getFrame(&frame, gunny->pSelf);
        gfmSprite_getDirection(&isFlipped, gunny->pSelf);

        x = x - cx + gunny_offx;
        y = y - cy + gunny_offy;
        rv = gfm_drawTile(game.pCtx, gfx.pSset16x16, x, y, frame + 64
                , isFlipped);
    }
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]gunny The player to be updated
 */
err preUpdateGunny(entityCtx *gunny) {
    gfmRV rv;
    err erv;

    if (!(game.flags & AC_GUNNY)) {
        return ERR_OK;
    }

    /* Update horizontal movement */
    do {
        if (IS_PRESSED(gunnyLeft)) {
            rv = gfmSprite_setHorizontalVelocity(gunny->pSelf, -GUNNY_SPEED);
        }
        else if (IS_PRESSED(gunnyRight)) {
            rv = gfmSprite_setHorizontalVelocity(gunny->pSelf, GUNNY_SPEED);
        }
        else {
            rv = gfmSprite_setHorizontalVelocity(gunny->pSelf, 0);
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    } while (0); /* Update horizontal movement */

    erv = updateEntityJump(gunny, input.gunnyJump.state);
    if (erv == ERR_DIDJUMP) {
        /* Set the error so the assert isn't triggered */
        erv = ERR_OK;
    }
    ASSERT(erv == ERR_OK, erv);

    /* Handle attack */
    do {
        if (DID_JUST_PRESS(gunnyAtk) && !(gunny->flags & gunny_attack)
                    && teleport.pCurEffect == 0) {
            gfmSprite *pBullet;
            double vx;
            int dir, x, y;

            gfmSprite_getPosition(&x, &y, gunny->pSelf);
            gfmSprite_getDirection(&dir, gunny->pSelf);
            y += 3;
            if (dir == 0) {
                x += 3;
                vx = BULLET_SPEED;
            }
            else {
                x -= 4;
                vx = -BULLET_SPEED;
            }

            pBullet = spawnFx(x, y, BULLET_WIDTH, 2/*h*/, dir, 6000/*ttl*/
                    , FX_GUNNY_BULLET, T_TEL_BULLET);
            ASSERT(pBullet != 0, ERR_GFMERR);
            gfmSprite_setHorizontalVelocity(pBullet, vx);
            gfmSprite_setOffset(pBullet, -6/*offx*/, -2/*offy*/);

            gunny->flags |= gunny_attack;
            gunny->flags |= gunny_justAttacked;
        }
    } while (0); /* Handle attack */

    rv = gfmSprite_update(gunny->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = collideEntity(gunny);
    ASSERT(erv == ERR_OK, erv);

    /* End attack animation */
    do {
        /* Clean flag as soon as animation stops */
        if (gunny->currentAnimation == ATK
                && gfmSprite_didAnimationFinish(gunny->pSelf)
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
err postUpdateGunny(entityCtx *gunny) {
    double vx, vy;
    err erv;
    gfmCollision dir;
    int hasCarrier;

    if (!(game.flags & AC_GUNNY)) {
        return ERR_OK;
    }

    hasCarrier = (gunny->pCarrying != 0);

    erv = postUpdateEntity(gunny);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(gunny);

    gfmSprite_getVelocity(&vx, &vy, gunny->pSelf);
    gfmSprite_getCollision(&dir, gunny->pSelf);

    /* Set animation */
    if (gunny->flags & gunny_attack) {
        setEntityAnimation(gunny, ATK, 0/*force*/);
    }
    else if (hasCarrier) {
        setEntityAnimation(gunny, STAND, 0/*force*/);
    }
    else if (vy > FLOAT_SPEED) {
        setEntityAnimation(gunny, FALL, 0/*force*/);
    }
    else if (vy < -FLOAT_SPEED) {
        setEntityAnimation(gunny, JUMP, 0/*force*/);
    }
    else if ((dir & gfmCollision_down) == 0 && vy >= -FLOAT_SPEED
            && vy <= FLOAT_SPEED) {
        setEntityAnimation(gunny, FLOAT, 0/*force*/);
    }
    else if (vx != 0) {
        setEntityAnimation(gunny, RUN, 0/*force*/);
    }
    else {
        setEntityAnimation(gunny, STAND, 0/*force*/);
    }

    return ERR_OK;
}

/**
 * Check if gunny should be teleported and do so.
 *
 * @param  [ in]gunny The player to be teleported
 */
err updateGunnyTeleport(entityCtx *gunny) {
    err erv;

    if (!(gunny->flags & gunny_justAttacked) && DID_JUST_PRESS(gunnyAtk)
            && teleport.pCurEffect != 0) {
        /* Teleport */
        erv = teleportEntity(gunny);
        ASSERT(erv == ERR_OK, erv);
    }

    /* Release the flag so the player may teleport */
    if (DID_JUST_RELEASE(gunnyAtk)) {
        gunny->flags &= ~gunny_justAttacked;
    }

    return ERR_OK;
}

