/**
 * @file src/boy_player.c
 *
 */
#include <base/game_const.h>
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

#include <jjat_2/entity.h>
#include <jjat_2/boy_player.h>
#include <jjat_2/type.h>

/** Required by malloc, free, and rand (>__<) */
#include <stdlib.h>
/** Required by memset */
#include <string.h>

/** List of animations */
enum enBoyAnim {
    STAND = 0,
    RUN,
    JUMP,
    FLOAT,
    FALL,
    SECJUMP,
    ATK_0,
    ATK_1,
    HURT
};
typedef enum enBoyAnim boyAnim;

/** Boy animation data */
static int pBoyAnimData[] = {
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

enum enBoyFlags {
    DOUBLEJUMP    = 0x00000001,
    ATK1_BUFFERED = 0x00000002,
    ATK2_BUFFERED = 0x00000004,

    ATKBUFFERED   = ATK1_BUFFERED | ATK2_BUFFERED,
};


/**
 * Initialize (and alloc, if needed) the boy from the parser
 *
 * @param [ in]pParser The parser (already pointing to a 'boy' object)
 * @return             GFraMe return value
 */
gfmRV boyPl_init(gfmParser *pParser) {
    /** GFraMe return value */
    gfmRV rv;
    /** The retrieve position */
    int x, y;

    /* Alloc the boy, if not yet done */
    if (!pGlobal->pBoy) {
        pGlobal->pBoy = (entity*)malloc(sizeof(entity));
        ASSERT(pGlobal->pBoy, GFMRV_ALLOC_FAILED);
        memset(pGlobal->pBoy, 0x0, sizeof(entity));

        rv = gfmSprite_getNew(&(pGlobal->pBoy->pSprite));
        ASSERT(pGlobal->pBoy->pSprite, GFMRV_ALLOC_FAILED);
    }

    /* Parse it */
    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, rv);
    /* There shouldn't be anything else... */

    /** Initialize the boy */
    rv = gfmSprite_init(pGlobal->pBoy->pSprite, x + BOY_OFFX, y - BOY_HEIGHT,
            BOY_WIDTH, BOY_HEIGHT, pGfx->pSset16x16, -BOY_OFFX, -BOY_OFFY,
            pGlobal->pBoy, T_BOY);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_addAnimationsStatic(pGlobal->pBoy->pSprite, pBoyAnimData);
    ASSERT(rv == GFMRV_OK, rv);

    pGlobal->pBoy->curAnim = -1;
    rv = entity_playAnimation(pGlobal->pBoy, STAND);
    ASSERT(rv == GFMRV_OK, rv);

    /** Set gravity */
    rv = gfmSprite_setVerticalAcceleration(pGlobal->pBoy->pSprite, GRAV);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Handle inputs from the user, updates the gfmSprite, collides againt the
 * quadtree and creates the attack hitbox
 *
 * @return GFraMe return value
 */
gfmRV boyPl_update() {
    /** Current collision direction */
    gfmCollision dir;
    /** GFraMe return value */
    gfmRV rv;

    rv = gfmSprite_getCollision(&dir, pGlobal->pBoy->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    if ((pButton->boyJump.state & gfmInput_justPressed) ==
            gfmInput_justPressed && (dir & gfmCollision_down)) {
        /* Change to jump state */
        rv = gfmSprite_setVerticalVelocity(pGlobal->pBoy->pSprite,
                BOY_JUMP_VY);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if ((pButton->boyAtk.state & gfmInput_justPressed) ==
            gfmInput_justPressed) {
        /* TODO Check whether bullet is flying or placed, or if it should be
         * shoot */
    }
    else if (pGlobal->pBoy->curAnim == ATK_0 || pGlobal->pBoy->curAnim == ATK_1) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pBoy->pSprite, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pButton->boyRight.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pBoy->pSprite, BOY_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pButton->boyLeft.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pBoy->pSprite, -BOY_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pBoy->pSprite, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Short hop */
    if ((pButton->boyJump.state & gfmInput_justReleased) == gfmInput_justReleased) {
        double vy;

        rv = gfmSprite_getVerticalVelocity(&vy, pGlobal->pBoy->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        if (vy < BOY_MIN_SH_VY * 2) {
            vy *= 0.5;
        }
        else if (vy < BOY_MIN_SH_VY) {
            vy = BOY_MIN_SH_VY;
        }

        rv = gfmSprite_setVerticalVelocity(pGlobal->pBoy->pSprite, vy);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = gfmSprite_update(pGlobal->pBoy->pSprite, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSprite_didAnimationJustChangeFrame(pGlobal->pBoy->pSprite);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        int frame, x, y;

        rv = gfmSprite_getPosition(&x, &y, pGlobal->pBoy->pSprite);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmSprite_getFrame(&frame, pGlobal->pBoy->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        /* TODO Do stuff */
    }

    /** Check if any collisions happened and handle it */
    rv = entity_collide(pGlobal->pBoy);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Handles special collision cases (e.g.: carrying/being carried by another
 * mob), updates the animation, handle being actually hurt(?)
 *
 * NOTE: Must be called after all sprites have collided
 *
 * @return GFraMe return value
 */
gfmRV boyPl_postUpdate() {
    double vx, vy;
    gfmRV rv;

    rv = gfmSprite_getVelocity(&vx, &vy, pGlobal->pBoy->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

#if 0
    if (pGlobal->pBoy->curAnim == ATK_0 || pGlobal->pBoy->curAnim == ATK_1) {
    }
    else if (vy != 0 && abs((int)vy) < -(BOY_MIN_SH_VY)) {
#endif
    if (vy != 0 && abs((int)vy) < -(BOY_MIN_SH_VY)) {
        rv = entity_playAnimation(pGlobal->pBoy, FLOAT);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy < 0) {
        rv = entity_playAnimation(pGlobal->pBoy, JUMP);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy > 0) {
        rv = entity_playAnimation(pGlobal->pBoy, FALL);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx > 0) {
        rv = entity_playAnimation(pGlobal->pBoy, RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx < 0) {
        rv = entity_playAnimation(pGlobal->pBoy, RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = entity_playAnimation(pGlobal->pBoy, STAND);
        ASSERT(rv == GFMRV_OK, rv);
    }

    if (vx != 0) {
        rv = gfmSprite_setDirection(pGlobal->pBoy->pSprite,  vx < 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = entity_updateStanding(pGlobal->pBoy);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Draw the sprite
 *
 * @return GFraMe return value
 */
gfmRV boyPl_draw() {
    gfmRV rv;

    gfmSprite_draw(pGlobal->pBoy->pSprite, pGame->pCtx);
__ret:
    return rv;
}

