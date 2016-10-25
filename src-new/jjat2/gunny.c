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

#define gunny_width 6
#define gunny_height 10
#define gunny_offx -5
#define gunny_offy -6


/** List of animations */
enum enGunnyAnim {
    STAND = 0,
    RUN,
    JUMP,
    FLOAT,
    FALL,
    SECJUMP,
    ATK_0,
    ATK_1,
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
/* SECJUMP */ 4 , 8 ,  1 , 75,76,77,78,
/*  ATK_0  */ 5 , 10,  0 , 79,80,81,82,86,
/*  ATK_1  */ 4 , 10,  0 , 83,84,85,86,
/*   HURT  */ 2 , 8 ,  1 , 80,81
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


    erv = updateEntityJump(&gunny->entity, input.gunnyJump.state);
    if (erv == ERR_DIDJUMP) {
        /* Set the error so the assert isn't triggered */
        erv = ERR_OK;
    }
    ASSERT(erv == ERR_OK, erv);

    rv = gfmSprite_update(gunny->entity.pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = collideEntity(&gunny->entity);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

