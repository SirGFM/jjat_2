/**
 * @file src/swordy.c
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
#include <jjat2/swordy.h>

/** Define Swordy's physics constants. The first parameter is the time in
 * 60FPS-frames and the second is the jump height in 8px-tiles. */
#define SWORDY_FALL_TIME 15
#define SWORDY_JUMP_TIME 20
#define SWORDY_JUMP_HEIGHT 4.5
#define SWORDY_HOP_TIME 18
#define SWORDY_HOP_HEIGHT 1.5
#define SWORDY_JUMP_SPEED JUMP_SPEED(SWORDY_JUMP_TIME, SWORDY_JUMP_HEIGHT)
#define SWORDY_HOP_SPEED JUMP_SPEED(SWORDY_HOP_TIME, SWORDY_HOP_HEIGHT)
#define SWORDY_JUMP_GRAV JUMP_ACCELERATION(SWORDY_JUMP_TIME, SWORDY_JUMP_HEIGHT)
#define SWORDY_FALL_GRAV JUMP_ACCELERATION(SWORDY_FALL_TIME, SWORDY_JUMP_HEIGHT)
#define SWORDY_SPEED TILES_TO_PX(12.5)

#define swordy_width 6
#define swordy_height 12
#define swordy_offx -5
#define swordy_offy -4

enum {
      flag_attacking      = 0x01
    , flag_atkMoveLeft    = 0x02
    , flag_atkSecondSlash = 0x04
    , flag_atkCombo       = 0x08
    , flag_atkResetAnim   = 0x10
};

/** List of animations */
enum enSwordyAnim {
    STAND = 0,
    RUN,
    JUMP,
    FLOAT,
    FALL,
    SECJUMP,
    ATK,
    HURT,
    SWORDY_ANIM_COUNT
};
typedef enum enSwordyAnim swordyAnim;

/** Swordy animation data */
static int pSwordyAnimData[] = {
/*           len|fps|loop|data... */
/*  STAND  */ 2 , 8 ,  1 , 32,33,
/*  RUN    */ 4 , 8 ,  1 , 34,35,36,37,
/*  JUMP   */ 2 , 8 ,  1 , 38,39,
/*  FLOAT  */ 1 , 8 ,  0 , 40,
/*  FALL   */ 2 , 8 ,  1 , 41,42,
/* SECJUMP */ 4 , 8 ,  1 , 43,44,45,46,
/*   ATK   */ 5 , 12,  0 , 47,48,49,50,51,
/*   HURT  */ 2 , 8 ,  1 , 48,49
};

/**
 * Initialize the swordy character
 *
 * @param  [ in]swordy The player to be initialized
 */
err initSwordy(swordyCtx *swordy) {
    err erv;
    gfmRV rv;

    /* Initialize the sprite */
    rv = gfmSprite_getNew(&swordy->entity.pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(swordy->entity.pSelf, 0, 0, swordy_width, swordy_height
            , gfx.pSset16x16, swordy_offx, swordy_offy, swordy, T_SWORDY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(swordy->entity.pSelf, pSwordyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    swordy->entity.maxAnimation = SWORDY_ANIM_COUNT;
    erv = setEntityAnimation(&swordy->entity, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    swordy->entity.jumpGrace = DEF_JUMP_GRACE;
    swordy->entity.jumpVelocity = SWORDY_JUMP_SPEED;
    swordy->entity.shorthopVelocity = SWORDY_HOP_SPEED;
    swordy->entity.standGravity = SWORDY_JUMP_GRAV;
    swordy->entity.fallGravity = SWORDY_FALL_GRAV;
    initEntity(&swordy->entity);

    return ERR_OK;
}

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]swordy The player to be freed
 */
void freeSwordy(swordyCtx *swordy) {
    if (swordy->entity.pSelf) {
        gfmSprite_free(&swordy->entity.pSelf);
    }
}

/**
 * Parse swordy into its position
 *
 * @param  [ in]swordy  The player
 * @param  [ in]pParser Parser that has just parsed a "swordy_pos"
 */
err parseSwordy(swordyCtx *swordy, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(swordy->entity.pSelf, x + swordy_width
            , y - swordy_height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = setEntityAnimation(&swordy->entity, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Render a swordy
 *
 * @param  [ in]swordy The player
 */
err drawSwordy(swordyCtx *swordy) {
    gfmRV rv;

    if (game.flags & AC_SWORDY) {
        rv = gfmSprite_draw(swordy->entity.pSelf, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
    else {
        int cx, cy, frame, isFlipped, x, y;

        gfmCamera_getPosition(&cx, &cy, game.pCamera);
        gfmSprite_getPosition(&x, &y, swordy->entity.pSelf);
        gfmSprite_getFrame(&frame, swordy->entity.pSelf);
        gfmSprite_getDirection(&isFlipped, swordy->entity.pSelf);

        x = x - cx + swordy_offx;
        y = y - cy + swordy_offy;
        rv = gfm_drawTile(game.pCtx, gfx.pSset16x16, x, y, frame + 64
                , isFlipped);
    }

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]swordy The player to be updated
 */
err preUpdateSwordy(swordyCtx *swordy) {
    gfmCollision col;
    gfmRV rv;
    err erv;

    if (!(game.flags & AC_SWORDY)) {
        return ERR_OK;
    }

    rv = gfmSprite_getCollision(&col, swordy->entity.pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Handle attack */
    do {
        int canAtk;

        /* Queue an attack as soon as possible */
        if (DID_JUST_PRESS(swordyAtk) && (col & gfmCollision_down)) {
            swordy->attackFlags |= flag_atkCombo;
        }

        /* Cancel attacks on the last frame */
        if (swordy->entity.currentAnimation == ATK) {
            int frame;

            gfmSprite_getFrame(&frame, swordy->entity.pSelf);
            canAtk = (frame == 51);
        }
        else {
            canAtk = 1;
        }

        /* TODO Handle aerial attack */
        if (canAtk && (swordy->attackFlags & flag_atkCombo)) {
            int anim, dir, x, y;

            gfmSprite_getPosition(&x, &y, swordy->entity.pSelf);
            gfmSprite_getDirection(&dir, swordy->entity.pSelf);
            y -= 2;
            if (dir == 0) {
                x += 3;
                swordy->attackFlags &= ~flag_atkMoveLeft;
            }
            else {
                x -= 13;
                swordy->attackFlags |= flag_atkMoveLeft;
            }

            if (swordy->attackFlags & flag_atkSecondSlash) {
                anim = FX_SWORDY_SLASH_UP;
                swordy->attackFlags &= ~flag_atkSecondSlash;
            }
            else {
                anim = FX_SWORDY_SLASH_DOWN;
                swordy->attackFlags |= flag_atkSecondSlash;
            }

            spawnFx(x, y, 16/*w*/, 16/*h*/, dir, 333/*ttl*/, anim, T_ATK_SWORD);

            swordy->attackFlags |= flag_attacking;
            swordy->attackFlags |= flag_atkResetAnim;
            swordy->attackFlags &= ~flag_atkCombo;
        }

        /* Enable moving backward when attacking */
        if (swordy->entity.currentAnimation == ATK) {
            int dir;
            gfmSprite_getDirection(&dir, swordy->entity.pSelf);
            if (dir == 0 && IS_PRESSED(swordyLeft)) {
                swordy->attackFlags |= flag_atkMoveLeft;
            }
            else if (dir == 1 && IS_PRESSED(swordyRight)) {
                swordy->attackFlags &= ~flag_atkMoveLeft;
            }
        }
    } while (0);

    /* Update horizontal movement */
    do {
        if (swordy->attackFlags & flag_attacking) {
            rv = gfmSprite_setHorizontalVelocity(swordy->entity.pSelf, 0);
        }
        else if (IS_PRESSED(swordyLeft)) {
            rv = gfmSprite_setHorizontalVelocity(swordy->entity.pSelf
                    , -SWORDY_SPEED);
        }
        else if (IS_PRESSED(swordyRight)) {
            rv = gfmSprite_setHorizontalVelocity(swordy->entity.pSelf
                    , SWORDY_SPEED);
        }
        else {
            rv = gfmSprite_setHorizontalVelocity(swordy->entity.pSelf, 0);
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    } while (0); /* Update horizontal movement */

    /* Update jump */
    do {
        /* Reset the jump count whenever swordy is grounded */
        if (col & gfmCollision_down) {
            swordy->jumpCount = 0;
        }
        else if (swordy->jumpCount == 0 && swordy->entity.jumpGrace <= 0) {
            /* If the first jump wasn't used, skip to the second one */
            swordy->jumpCount = 1;
        }

        if (swordy->jumpCount == 1) {
            /* Every frame, set a short time for the second jump (until used) */
            swordy->entity.jumpGrace = FRAMES_TO_MS(2);
        }

        erv = updateEntityJump(&swordy->entity, input.swordyJump.state);
        if (erv == ERR_DIDJUMP) {
            swordy->jumpCount++;

            /* Jump-cancel grounded attacks */
            swordy->attackFlags &= ~flag_attacking;

            /* Set the error so the assert isn't triggered */
            erv = ERR_OK;
        }
        ASSERT(erv == ERR_OK, erv);
    } while (0); /* Update jump */

    rv = gfmSprite_update(swordy->entity.pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Adjust attack animation */
    do {
        int frame;

        gfmSprite_getFrame(&frame, swordy->entity.pSelf);
        /* Reflect sprite's movement into its world position */
        if ((col & gfmCollision_down)
                && swordy->entity.currentAnimation == ATK
                && (frame == 50 || frame == 51)
                && (gfmSprite_didAnimationJustChangeFrame(swordy->entity.pSelf)
                == GFMRV_TRUE)) {
            int x;

            gfmSprite_getHorizontalPosition(&x, swordy->entity.pSelf);
            if (swordy->attackFlags & flag_atkMoveLeft) {
                gfmSprite_setHorizontalPosition(swordy->entity.pSelf, x - 2);
            }
            else {
                gfmSprite_setHorizontalPosition(swordy->entity.pSelf, x + 2);
            }
        }

        /* Clean flag as soon as animation stops */
        if (swordy->entity.currentAnimation == ATK
                && gfmSprite_didAnimationFinish(swordy->entity.pSelf)
                == GFMRV_TRUE) {
            swordy->attackFlags &= ~flag_attacking;
        }
    } while (0);

    erv = collideEntity(&swordy->entity);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Set swordy's animation and fix its entity's collision.
 *
 * @param  [ in]swordy The player to be updated
 */
err postUpdateSwordy(swordyCtx *swordy) {
    double vx, vy;
    err erv;
    gfmCollision dir;
    int hasCarrier;

    if (!(game.flags & AC_SWORDY)) {
        return ERR_OK;
    }

    hasCarrier = (swordy->entity.pCarrying != 0);

    erv = postUpdateEntity(&swordy->entity);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(&swordy->entity);

    gfmSprite_getVelocity(&vx, &vy, swordy->entity.pSelf);
    gfmSprite_getCollision(&dir, swordy->entity.pSelf);

    /* Set animation */
    if (swordy->attackFlags & flag_attacking) {
        setEntityAnimation(&swordy->entity, ATK
                , swordy->attackFlags & flag_atkResetAnim);
        swordy->attackFlags &= ~flag_atkResetAnim;
    }
    else if (hasCarrier) {
        setEntityAnimation(&swordy->entity, STAND, 0/*force*/);
    }
    else if (swordy->jumpCount == 2) {
        setEntityAnimation(&swordy->entity, SECJUMP, 0/*force*/);
    }
    else if (vy > FLOAT_SPEED) {
        setEntityAnimation(&swordy->entity, FALL, 0/*force*/);
    }
    else if (vy < -FLOAT_SPEED) {
        setEntityAnimation(&swordy->entity, JUMP, 0/*force*/);
    }
    else if ((dir & gfmCollision_down) == 0 && vy >= -FLOAT_SPEED && vy <= FLOAT_SPEED) {
        setEntityAnimation(&swordy->entity, FLOAT, 0/*force*/);
    }
    else if (vx != 0) {
        setEntityAnimation(&swordy->entity, RUN, 0/*force*/);
    }
    else {
        setEntityAnimation(&swordy->entity, STAND, 0/*force*/);
    }

    return ERR_OK;
}

