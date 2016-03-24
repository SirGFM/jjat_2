/**
 * @file src/girl_player.c
 *
 * Messy implementation (read: ugly and not generic) of the girl. This is mostly
 * a starting point so I can get the feel for what is and isn't need when coding
 * all mobs properly.
 *
 * Things I must have answered by the end of this:
 *  - How will everything update?
 *  - How will everything collide?
 *  - How do I handle collision against environment and 'moving platforms'?
 *  - How do I properly handle animations through various different sprites?
 *  - ...
 */
#include <base/collision.h>
#include <base/game_const.h>
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

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
/*  ATK_0  */ 5 , 12,  0 , 47,48,49,50,54,
/*  ATK_1  */ 4 , 12,  0 , 51,52,53,54,
/*   HURT  */ 2 , 8 ,  1 , 48,49
};

struct stGirlPlayer {
    /** The actual sprite */
    gfmSprite *pSprite;
    /** Which object (if any) this is standing over */
    gfmObject *pStanding;
    /** Last spawned attack (unneeded?) */
    gfmSprite *pAtk;
    /** Current animation (each type will have its own 'list of types'?) */
    int curAnim;
    /** Whether we've just double jumped (and haven't touched the floor yet) */
    int didDoubleJump;
    /** Whether an attack was issued and should be done as soon as possible */
    int isAtkBuffered;
    /** Enemies should also have a counter of some sort... probably */
    /** Also a current state (or I'll simply use the animation... don't know */
};
typedef struct stGirlPlayer girlPlayer;

/** This is just a slight mockup, so it should be ok to be lazy... right? */
static girlPlayer *_pGirl = 0;

/**
 * Update the currently playing animation
 *
 * @param  [ in]anim The animation's index
 * @return           GFraMe return value
 */
static gfmRV grlPl_playAnim(girlAnim anim) {
    /** GFraMe return value */
    gfmRV rv;

    /* Only change the animation if it wasn't playing */
    if (anim == _pGirl->curAnim) {
        return GFMRV_OK;
    }

    rv = gfmSprite_playAnimation(_pGirl->pSprite, anim);
    ASSERT(rv == GFMRV_OK, rv);
    _pGirl->curAnim = anim;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Free all resources used by the girl sprite
 *
 * @return GFraMe return value
 */
void grlPl_free() {
    /** Release any alloc'ed memory */
    if (_pGirl) {
        gfmSprite_free(&(_pGirl->pSprite));
        free(_pGirl);
        _pGirl = 0;
    }
}

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
    if (!_pGirl) {
        _pGirl = (girlPlayer*)malloc(sizeof(girlPlayer));
        ASSERT(_pGirl, GFMRV_ALLOC_FAILED);
        memset(_pGirl, 0x0, sizeof(girlPlayer));

        rv = gfmSprite_getNew(&(_pGirl->pSprite));
        ASSERT(_pGirl->pSprite, GFMRV_ALLOC_FAILED);
    }

    /* Parse it */
    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, rv);
    /* There shouldn't be anything else... */

    /** Initialize the girl */
    rv = gfmSprite_init(_pGirl->pSprite, x + PL_OFFX, y - PL_HEIGHT, PL_WIDTH, PL_HEIGHT,
            pGfx->pSset16x16, -PL_OFFX, -PL_OFFY, _pGirl, T_GIRL);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_addAnimationsStatic(_pGirl->pSprite, pGirlAnimData);
    ASSERT(rv == GFMRV_OK, rv);

    _pGirl->curAnim = -1;
    rv = grlPl_playAnim(STAND);
    ASSERT(rv == GFMRV_OK, rv);

    /** Set gravity */
    rv = gfmSprite_setVerticalAcceleration(_pGirl->pSprite, GRAV);
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

    rv = gfmSprite_getCollision(&dir, _pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    /* Reset double jump on touch floor */
    _pGirl->didDoubleJump &= ((dir & gfmCollision_down) == 0);

    if (_pGirl->curAnim == ATK_1) { }
    else if ((pButton->grlJump.state & gfmInput_justPressed) == gfmInput_justPressed) {
        if (dir & gfmCollision_down) {
            /* Change to jump state */
            rv = gfmSprite_setVerticalVelocity(_pGirl->pSprite, -175);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if (!_pGirl->didDoubleJump) {
            /* Change to doublejump state */
            rv = gfmSprite_setVerticalVelocity(_pGirl->pSprite, -175);
            ASSERT(rv == GFMRV_OK, rv);
            _pGirl->didDoubleJump = 1;
        }
    }
    else if ((pButton->grlAtk.state & gfmInput_justPressed) == gfmInput_justPressed) {
        /* Change to atk state */
        if (_pGirl->curAnim != ATK_0 && _pGirl->curAnim != ATK_1) {
            rv = grlPl_playAnim(ATK_0);
            ASSERT(rv == GFMRV_OK, rv);
        }
        else if ((dir & gfmCollision_down) && _pGirl->curAnim == ATK_0) {
            _pGirl->isAtkBuffered = 1;
        }
    }
    else if (_pGirl->curAnim == ATK_0) { }
    else if (pButton->grlRight.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(_pGirl->pSprite, 100);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (pButton->grlLeft.state & gfmInput_pressed) {
        rv = gfmSprite_setHorizontalVelocity(_pGirl->pSprite, -100);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = gfmSprite_setHorizontalVelocity(_pGirl->pSprite, 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Short hop */
    if ((pButton->grlJump.state & gfmInput_justReleased) == gfmInput_justReleased) {
        double vy;

        rv = gfmSprite_getVerticalVelocity(&vy, _pGirl->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        if (vy < -60) {
            vy *= 0.5;
        }
        else if (vy < -30) {
            vy = -30;
        }

        rv = gfmSprite_setVerticalVelocity(_pGirl->pSprite, vy);
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = gfmSprite_update(_pGirl->pSprite, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSprite_didAnimationJustChangeFrame(_pGirl->pSprite);
    ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);
    if (rv == GFMRV_TRUE) {
        int frame;

        rv = gfmSprite_getFrame(&frame, _pGirl->pSprite);
        ASSERT(rv == GFMRV_OK, rv);

        /* TODO Do stuff */
        if (frame == 48) {
            /* Spawn hitbox */
        }
        else if (frame == 51) {
            /* Spawn hitbox */
        }
        else if (frame == 54 && _pGirl->isAtkBuffered) {
            rv = grlPl_playAnim(ATK_1);
            ASSERT(rv == GFMRV_OK, rv);
            _pGirl->isAtkBuffered = 0;
        }

        if ((_pGirl->curAnim == ATK_0 || _pGirl->curAnim == ATK_1) && gfmSprite_didAnimationFinish(_pGirl->pSprite) == GFMRV_TRUE) {
            _pGirl->curAnim = -1;
        }
    }

    /** Check if any collisions happened and handle it */
    rv = gfmQuadtree_collideSprite(pGlobal->pQt, _pGirl->pSprite);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collision_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

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

    rv = gfmSprite_getVelocity(&vx, &vy, _pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    if (_pGirl->curAnim == ATK_0 || _pGirl->curAnim == ATK_1) {
    }
    else if (_pGirl->didDoubleJump) {
        /* Double jump */
        rv = grlPl_playAnim(SECJUMP);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy != 0 && abs((int)vy) < 30) {
        /* NOTE: ABS!! INT JUMP */
        rv = grlPl_playAnim(FLOAT);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy < 0) {
        /* either START_JUMP or JUMP */
        rv = grlPl_playAnim(JUMP);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vy > 0) {
        /* FALL */
        rv = grlPl_playAnim(FALL);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx > 0) {
        rv = grlPl_playAnim(RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (vx < 0) {
        rv = grlPl_playAnim(RUN);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else {
        rv = grlPl_playAnim(STAND);
        ASSERT(rv == GFMRV_OK, rv);
    }

    if (vx != 0) {
        rv = gfmSprite_setDirection(_pGirl->pSprite,  vx < 0);
        ASSERT(rv == GFMRV_OK, rv);
    }

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
    int tile, x, y, frame, flipped;

    rv = gfmSprite_getPosition(&x, &y, _pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getFrame(&frame, _pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmSprite_getDirection(&flipped, _pGirl->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    if (frame == 48) {
        tile = 183;
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x-7, y-4, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+2, y-4, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+2, y+4, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 49) {
        tile = 186;
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+3, y-4, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+3, y+4, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 51) {
        tile = 188;
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+6, y-5, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+3, y+3, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }
    else if (frame == 52) {
        tile = 190;
        rv = gfm_drawTile(pGame->pCtx, pGfx->pSset8x8, x+6, y-5, tile++,
                flipped);
        ASSERT(rv == GFMRV_OK, rv);
    }

    gfmSprite_draw(_pGirl->pSprite, pGame->pCtx);
__ret:
    return rv;
}

