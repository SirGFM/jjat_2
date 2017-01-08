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
      flag_attacking        = (EF_AVAILABLEF_FLAG << 0)
    , flag_atkMoveLeft      = (EF_AVAILABLEF_FLAG << 1)
    , flag_atkSecondSlash   = (EF_AVAILABLEF_FLAG << 2)
    , flag_atkCombo         = (EF_AVAILABLEF_FLAG << 3)
    , flag_atkResetAnim     = (EF_AVAILABLEF_FLAG << 4)
    , flag_jumpCounter_mask = (EF_AVAILABLEF_FLAG << 5
                                  | EF_AVAILABLEF_FLAG << 6)
    , flag_jumpCounter_1    = (EF_AVAILABLEF_FLAG << 5)
    , flag_jumpCounter_2    = (EF_AVAILABLEF_FLAG << 6)
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
err initSwordy(entityCtx *swordy) {
    err erv;
    gfmRV rv;

    /* Initialize the sprite */
    rv = gfmSprite_getNew(&swordy->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(swordy->pSelf, 0, 0, swordy_width, swordy_height
            , gfx.pSset16x16, swordy_offx, swordy_offy, swordy, T_SWORDY);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(swordy->pSelf, pSwordyAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Play its default animation */
    swordy->maxAnimation = SWORDY_ANIM_COUNT;
    erv = setEntityAnimation(swordy, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    /* Set all entity attributes */
    swordy->jumpGrace = DEF_JUMP_GRACE;
    swordy->jumpVelocity = SWORDY_JUMP_SPEED;
    swordy->shorthopVelocity = SWORDY_HOP_SPEED;
    swordy->standGravity = SWORDY_JUMP_GRAV;
    swordy->fallGravity = SWORDY_FALL_GRAV;
    initEntity(swordy);

    return ERR_OK;
}

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]swordy The player to be freed
 */
void freeSwordy(entityCtx *swordy) {
    if (swordy->pSelf) {
        gfmSprite_free(&swordy->pSelf);
    }
}

/**
 * Parse swordy into its position
 *
 * @param  [ in]swordy  The player
 * @param  [ in]pParser Parser that has just parsed a "swordy_pos"
 */
err parseSwordy(entityCtx *swordy, gfmParser *pParser) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(swordy->pSelf, x + swordy_width
            , y - swordy_height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = setEntityAnimation(swordy, STAND, 1/*force*/);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Render a swordy
 *
 * @param  [ in]swordy The player
 */
err drawSwordy(entityCtx *swordy) {
    gfmDebug_printf(game.pCtx, 0, 64, "SWORDY ALIVE: %i", swordy->flags & EF_ALIVE);

    if (game.flags & AC_SWORDY) {
        return drawEntity(swordy);
    }
    else {
        gfmRV rv;
        int cx, cy, frame, isFlipped, x, y;

        gfmCamera_getPosition(&cx, &cy, game.pCamera);
        gfmSprite_getPosition(&x, &y, swordy->pSelf);
        gfmSprite_getFrame(&frame, swordy->pSelf);
        gfmSprite_getDirection(&isFlipped, swordy->pSelf);

        x = x - cx + swordy_offx;
        y = y - cy + swordy_offy;
        rv = gfm_drawTile(game.pCtx, gfx.pSset16x16, x, y, frame + 64
                , isFlipped);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    return ERR_OK;
}

/**
 * Update the object's physics.
 *
 * @param  [ in]swordy The player to be updated
 */
err preUpdateSwordy(entityCtx *swordy) {
    gfmCollision col;
    gfmRV rv;
    err erv;

    if (!(game.flags & AC_SWORDY)) {
        return ERR_OK;
    }

    rv = gfmSprite_getCollision(&col, swordy->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Handle attack */
    do {
        int canAtk;

        /* Queue an attack as soon as possible */
        if (DID_JUST_PRESS(swordyAtk) && (col & gfmCollision_down)) {
            swordy->flags |= flag_atkCombo;
        }

        /* Cancel attacks on the last frame */
        if (swordy->currentAnimation == ATK) {
            int frame;

            gfmSprite_getFrame(&frame, swordy->pSelf);
            canAtk = (frame == 51);
        }
        else {
            canAtk = 1;
        }

        /* TODO Handle aerial attack */
        if (canAtk && (swordy->flags & flag_atkCombo)) {
            int anim, dir, x, y;

            gfmSprite_getPosition(&x, &y, swordy->pSelf);
            gfmSprite_getDirection(&dir, swordy->pSelf);
            y -= 2;
            if (dir == DIR_RIGHT) {
                x += 3;
                swordy->flags &= ~flag_atkMoveLeft;
            }
            else if (dir == DIR_LEFT) {
                x -= 13;
                swordy->flags |= flag_atkMoveLeft;
            }

            if (swordy->flags & flag_atkSecondSlash) {
                anim = FX_SWORDY_SLASH_UP;
                swordy->flags &= ~flag_atkSecondSlash;
            }
            else {
                anim = FX_SWORDY_SLASH_DOWN;
                swordy->flags |= flag_atkSecondSlash;
            }

            spawnFx(x, y, 16/*w*/, 16/*h*/, dir, 333/*ttl*/, anim, T_ATK_SWORD);

            swordy->flags |= flag_attacking;
            swordy->flags |= flag_atkResetAnim;
            swordy->flags &= ~flag_atkCombo;
        }

        /* Enable moving backward when attacking */
        if (swordy->currentAnimation == ATK) {
            int dir;
            gfmSprite_getDirection(&dir, swordy->pSelf);
            if (dir == DIR_RIGHT && IS_PRESSED(swordyLeft)) {
                swordy->flags |= flag_atkMoveLeft;
            }
            else if (dir == DIR_LEFT && IS_PRESSED(swordyRight)) {
                swordy->flags &= ~flag_atkMoveLeft;
            }
        }
    } while (0);

    /* Update horizontal movement */
    do {
        if (swordy->flags & flag_attacking) {
            rv = gfmSprite_setHorizontalVelocity(swordy->pSelf, 0);
        }
        else if (IS_PRESSED(swordyLeft)) {
            rv = gfmSprite_setHorizontalVelocity(swordy->pSelf
                    , -SWORDY_SPEED);
        }
        else if (IS_PRESSED(swordyRight)) {
            rv = gfmSprite_setHorizontalVelocity(swordy->pSelf
                    , SWORDY_SPEED);
        }
        else {
            rv = gfmSprite_setHorizontalVelocity(swordy->pSelf, 0);
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    } while (0); /* Update horizontal movement */

    /* Update jump */
    do {
        /* Reset the jump count whenever swordy is grounded */
        if (col & gfmCollision_down) {
            swordy->flags &= ~flag_jumpCounter_mask;
        }
        else if ((swordy->flags & flag_jumpCounter_mask) == 0
                && swordy->jumpGrace <= 0) {
            /* If the first jump wasn't used, skip to the second one */
            swordy->flags |= flag_jumpCounter_1;
        }

        if ((swordy->flags & flag_jumpCounter_mask) == flag_jumpCounter_1) {
            /* Every frame, set a short time for the second jump (until used) */
            swordy->jumpGrace = FRAMES_TO_MS(2);
        }

        erv = updateEntityJump(swordy, input.swordyJump.state);
        if (erv == ERR_DIDJUMP) {
            /* Increase the jump count */
            if ((swordy->flags & flag_jumpCounter_mask) == 0) {
                swordy->flags |= flag_jumpCounter_1;
            }
            else {
                swordy->flags |= flag_jumpCounter_2;
            }

            /* Jump-cancel grounded attacks */
            swordy->flags &= ~flag_attacking;

            /* Set the error so the assert isn't triggered */
            erv = ERR_OK;
        }
        ASSERT(erv == ERR_OK, erv);
    } while (0); /* Update jump */

    /* Don't collide while updating, since attacking moves Swordy */
    swordy->flags |= EF_SKIP_COLLISION;
    erv = preUpdateEntity(swordy);
    swordy->flags &= ~EF_SKIP_COLLISION;
    ASSERT(erv == ERR_OK, erv);

    /* Adjust attack animation */
    do {
        int frame;

        gfmSprite_getFrame(&frame, swordy->pSelf);
        /* Reflect sprite's movement into its world position */
        if ((col & gfmCollision_down)
                && swordy->currentAnimation == ATK
                && (frame == 50 || frame == 51)
                && (gfmSprite_didAnimationJustChangeFrame(swordy->pSelf)
                == GFMRV_TRUE)) {
            int x;

            gfmSprite_getHorizontalPosition(&x, swordy->pSelf);
            if (swordy->flags & flag_atkMoveLeft) {
                gfmSprite_setHorizontalPosition(swordy->pSelf, x - 2);
            }
            else {
                gfmSprite_setHorizontalPosition(swordy->pSelf, x + 2);
            }
        }

        /* Clean flag as soon as animation stops */
        if (swordy->currentAnimation == ATK
                && gfmSprite_didAnimationFinish(swordy->pSelf)
                == GFMRV_TRUE) {
            swordy->flags &= ~flag_attacking;
        }
    } while (0);

    erv = collideEntity(swordy);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Set swordy's animation and fix its entity's collision.
 *
 * @param  [ in]swordy The player to be updated
 */
err postUpdateSwordy(entityCtx *swordy) {
    double vx, vy;
    err erv;
    gfmCollision dir;

    if (!(game.flags & AC_SWORDY)) {
        return ERR_OK;
    }

    erv = postUpdateEntity(swordy);
    ASSERT(erv == ERR_OK, erv);
    setEntityDirection(swordy);

    gfmSprite_getVelocity(&vx, &vy, swordy->pSelf);
    gfmSprite_getCollision(&dir, swordy->pSelf);

    /* Set animation */
    if (swordy->flags & flag_attacking) {
        setEntityAnimation(swordy, ATK, swordy->flags & flag_atkResetAnim);
        swordy->flags &= ~flag_atkResetAnim;
    }
    else if (swordy->flags & EF_HAS_CARRIER) {
        setEntityAnimation(swordy, STAND, 0/*force*/);
    }
    else if (swordy->flags & flag_jumpCounter_2) {
        setEntityAnimation(swordy, SECJUMP, 0/*force*/);
    }
    else if (vy > FLOAT_SPEED) {
        setEntityAnimation(swordy, FALL, 0/*force*/);
    }
    else if (vy < -FLOAT_SPEED) {
        setEntityAnimation(swordy, JUMP, 0/*force*/);
    }
    else if ((dir & gfmCollision_down) == 0 && vy >= -FLOAT_SPEED && vy <= FLOAT_SPEED) {
        setEntityAnimation(swordy, FLOAT, 0/*force*/);
    }
    else if (vx != 0) {
        setEntityAnimation(swordy, RUN, 0/*force*/);
    }
    else {
        setEntityAnimation(swordy, STAND, 0/*force*/);
    }

    return ERR_OK;
}

