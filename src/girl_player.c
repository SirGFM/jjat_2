/**
 * @file src/girl_player.c
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
#include <jjat_2/girl_player.h>
#include <jjat_2/type.h>

/** Required by malloc, free, and rand (>__<) */
#include <stdlib.h>
/** Required by memset */
#include <string.h>

/** List of animations */
enum enGirlAnim {
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
typedef enum enGirlAnim girlAnim;

/** Girl animation data */
static int pGirlAnimData[] = {
/*           len|fps|loop|data... */
/*  STAND  */ 2 , 8 ,  1 , 32,33,
/*  RUN    */ 4 , 8 ,  1 , 34,35,36,37,
/*  JUMP   */ 2 , 8 ,  1 , 38,39,
/*  FLOAT  */ 1 , 8 ,  0 , 40,
/*  FALL   */ 2 , 8 ,  1 , 41,42,
/* SECJUMP */ 4 , 8 ,  1 , 43,44,45,46,
/*  ATK_0  */ 5 , 10,  0 , 47,48,49,50,54,
/*  ATK_1  */ 4 , 10,  0 , 51,52,53,54,
/*   HURT  */ 2 , 8 ,  1 , 48,49
};

enum enGirlFlags {
    DOUBLEJUMP    = 0x00000001,
    ATK1_BUFFERED = 0x00000002,
    ATK2_BUFFERED = 0x00000004,

    ATKBUFFERED   = ATK1_BUFFERED | ATK2_BUFFERED,
};


/**
 * Initialize (and alloc, if needed) the girl from the parser
 *
 * @param [ in]pParser The parser (already pointing to a 'girl' object)
 * @return             GFraMe return value
 */
gfmRV grlPl_init(gfmParser *pParser) {
    /** GFraMe return value */
    gfmRV rv;
    /** The retrieve position */
    int x, y;

    /* Alloc the girl, if not yet done */
    if (!pGlobal->pGirl) {
        pGlobal->pGirl = (entity*)malloc(sizeof(entity));
        ASSERT(pGlobal->pGirl, GFMRV_ALLOC_FAILED);
        memset(pGlobal->pGirl, 0x0, sizeof(entity));

        rv = gfmSprite_getNew(&(pGlobal->pGirl->pSprite));
        ASSERT(pGlobal->pGirl->pSprite, GFMRV_ALLOC_FAILED);
    }

    /* Parse it */
    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, rv);
    /* There shouldn't be anything else... */

    /** Initialize the girl */
    rv = gfmSprite_init(pGlobal->pGirl->pSprite, x + GIRL_OFFX, y - GIRL_HEIGHT,
            GIRL_WIDTH, GIRL_HEIGHT, pGfx->pSset16x16, -GIRL_OFFX, -GIRL_OFFY,
            pGlobal->pGirl, T_GIRL);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_addAnimationsStatic(pGlobal->pGirl->pSprite, pGirlAnimData);
    ASSERT(rv == GFMRV_OK, rv);

    pGlobal->pGirl->curAnim = -1;
    rv = entity_playAnimation(pGlobal->pGirl, STAND);
    ASSERT(rv == GFMRV_OK, rv);

    /** Set gravity */
    rv = gfmSprite_setVerticalAcceleration(pGlobal->pGirl->pSprite, GRAV);
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
gfmRV grlPl_update() {
    /** Current collision direction */
    gfmCollision dir;
    /** GFraMe return value */
    gfmRV rv;

    rv = gfmSprite_getCollision(&dir, pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    /* Reset double jump on touch floor */
    if (dir & gfmCollision_down) {
        pGlobal->pGirl->entityData &= ~DOUBLEJUMP;
    }

    if ((pButton->grlJump.state & gfmInput_justPressed) ==
            gfmInput_justPressed) {
        if (dir & gfmCollision_down) {
            /* Change to jump state */
            rv = gfmSprite_setVerticalVelocity(pGlobal->pGirl->pSprite,
                    GIRL_JUMP_VY);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if (!(pGlobal->pGirl->entityData & DOUBLEJUMP)) {
            /* Change to doublejump state */
            rv = gfmSprite_setVerticalVelocity(pGlobal->pGirl->pSprite,
                    GIRL_JUMP_VY);
            ASSERT(rv == GFMRV_OK, rv);
            pGlobal->pGirl->entityData |= DOUBLEJUMP;
        }
    }
    else if ((pButton->grlAtk.state & gfmInput_justPressed) ==
            gfmInput_justPressed) {
        /* Change to atk state */
        if (pGlobal->pGirl->curAnim != ATK_0 && pGlobal->pGirl->curAnim != ATK_1) {
            rv = entity_playAnimation(pGlobal->pGirl, ATK_0);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if ((dir & gfmCollision_down)) {
            if (pGlobal->pGirl->curAnim == ATK_0) {
                pGlobal->pGirl->entityData |= ATK1_BUFFERED;
            }
            else {
                pGlobal->pGirl->entityData |= ATK2_BUFFERED;
            }
        }
    }
    else if (pGlobal->pGirl->curAnim == ATK_0 || pGlobal->pGirl->curAnim == ATK_1) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pGirl->pSprite, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pButton->grlRight.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pGirl->pSprite, GIRL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pButton->grlLeft.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pGirl->pSprite, -GIRL_VX);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = gfmSprite_setHorizontalVelocity(pGlobal->pGirl->pSprite, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Short hop */
    if ((pButton->grlJump.state & gfmInput_justReleased) == gfmInput_justReleased) {
        double vy;

        rv = gfmSprite_getVerticalVelocity(&vy, pGlobal->pGirl->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        if (vy < GIRL_MIN_SH_VY * 2) {
            vy *= 0.5;
        }
        else if (vy < GIRL_MIN_SH_VY) {
            vy = GIRL_MIN_SH_VY;
        }

        rv = gfmSprite_setVerticalVelocity(pGlobal->pGirl->pSprite, vy);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = gfmSprite_update(pGlobal->pGirl->pSprite, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSprite_didAnimationJustChangeFrame(pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        int frame, x, y;

        rv = gfmSprite_getPosition(&x, &y, pGlobal->pGirl->pSprite);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfmSprite_getFrame(&frame, pGlobal->pGirl->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        /* TODO Do stuff */
        if (frame == 48) {
            rv = gfmSprite_setPosition(pGlobal->pGirl->pSprite, x + 3, y);
            ASSERT(rv == GFMRV_OK, rv);
            /* Spawn hitbox */
        }
        if (frame == 50) {
            rv = gfmSprite_setPosition(pGlobal->pGirl->pSprite, x + 1, y);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if (frame == 54 && (pGlobal->pGirl->entityData & ATKBUFFERED)) {
            if (pGlobal->pGirl->entityData & ATK1_BUFFERED) {
                rv = entity_playAnimation(pGlobal->pGirl, ATK_1);
                rv = gfmSprite_setPosition(pGlobal->pGirl->pSprite, x + 2, y);
                ASSERT(rv == GFMRV_OK, rv);
                /* Spawn hitbox */
            }
            else {
                rv = entity_playAnimation(pGlobal->pGirl, ATK_0);
            }
            ASSERT(rv == GFMRV_OK, rv);
            pGlobal->pGirl->entityData &= ~ATKBUFFERED;
        }

        if ((pGlobal->pGirl->curAnim == ATK_0 ||
                pGlobal->pGirl->curAnim == ATK_1) &&
                gfmSprite_didAnimationFinish(pGlobal->pGirl->pSprite) ==
                GFMRV_TRUE) {
            pGlobal->pGirl->curAnim = -1;
        }
    }

    /** Check if any collisions happened and handle it */
    rv = entity_collide(pGlobal->pGirl);
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
gfmRV grlPl_postUpdate() {
    double vx, vy;
    gfmRV rv;

    rv = gfmSprite_getVelocity(&vx, &vy, pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    if (pGlobal->pGirl->curAnim == ATK_0 || pGlobal->pGirl->curAnim == ATK_1) {
    }
    else if (pGlobal->pGirl->entityData & DOUBLEJUMP) {
        /* Double jump */
        rv = entity_playAnimation(pGlobal->pGirl, SECJUMP);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy != 0 && abs((int)vy) < 30) {
        rv = entity_playAnimation(pGlobal->pGirl, FLOAT);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy < 0) {
        rv = entity_playAnimation(pGlobal->pGirl, JUMP);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy > 0) {
        rv = entity_playAnimation(pGlobal->pGirl, FALL);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx > 0) {
        rv = entity_playAnimation(pGlobal->pGirl, RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx < 0) {
        rv = entity_playAnimation(pGlobal->pGirl, RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = entity_playAnimation(pGlobal->pGirl, STAND);
        ASSERT(rv == GFMRV_OK, rv);
    }

    if (vx != 0) {
        rv = gfmSprite_setDirection(pGlobal->pGirl->pSprite,  vx < 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = entity_updateStanding(pGlobal->pGirl);
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
gfmRV grlPl_draw() {
    gfmRV rv;
    int x, y, frame, flipped;

    rv = gfmSprite_getPosition(&x, &y, pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getFrame(&frame, pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getDirection(&flipped, pGlobal->pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    if (frame == 47) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX,
            y - GIRL_OFFY - 3, 57, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 48) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX,
            y - GIRL_OFFY - 2, 58, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 49) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX + 1,
            y - GIRL_OFFY, 59, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 50) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX + 1,
            y - GIRL_OFFY, 60, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 51) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX + 4,
            y - GIRL_OFFY - 3, 61, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 52) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX + 4,
            y - GIRL_OFFY - 4, 62, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 53) {
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset16x16, x - GIRL_OFFX + 4,
            y - GIRL_OFFY - 3, 63, flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }

    gfmSprite_draw(pGlobal->pGirl->pSprite, pGame->pCtx);
__ret:
    return rv;
}

