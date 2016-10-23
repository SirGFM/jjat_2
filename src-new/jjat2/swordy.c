/**
 * @file src/swordy.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>

#include <conf/type.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmSprite.h>

#include <jjat2/swordy.h>

/** Define Swordy's physics constants. The first parameter is the time in
 * 60FPS-frames and the second is the jump height in 8px-tiles. */
#define SWORDY_JUMP_SPEED JUMP_SPEED(18, 4)
#define SWORDY_HOP_SPEED JUMP_SPEED(8, 1.75)
#define SWORDY_JUMP_GRAV JUMP_ACCELERATION(18, 4)
#define SWORDY_FALL_GRAV JUMP_ACCELERATION(10, 4)

#define swordy_width 6
#define swordy_height 12
#define swordy_offx -5
#define swordy_offy -4


/** List of animations */
enum enSwordyAnim {
    STAND = 0,
    RUN,
    JUMP,
    FLOAT,
    FALL,
    SECJUMP,
    ATK_0,
    ATK_1,
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
/*  ATK_0  */ 5 , 12,  0 , 47,48,49,50,54,
/*  ATK_1  */ 4 , 12,  0 , 51,52,53,54,
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
    rv = gfmSprite_draw(swordy->entity.pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    return ERR_OK;
}


/**
 * Update the object's physics.
 *
 * @param  [ in]swordy The player to be updated
 */
err preUpdateSwordy(swordyCtx *swordy) {
    return ERR_OK;
}

