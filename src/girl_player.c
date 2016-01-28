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
    STAND_0 = 0, /* Simply stand */
    STAND_1,     /* Balance slightly */
    STAND_2,     /* Tap feet */
    STAND_3,     /* Phew */
    RUN,
    HURT,
    FALL,
    JUMP,
    ATK_0,
    ATK_1
};
typedef enum enGirlAnim girlAnim;

/** Girl animation data */
static int pGirlAnimData[] = {
/*           len|fps|loop|data... */
/* STAND_0 */ 1 , 8 ,  0 , 32,
/* STAND_1 */ 3 , 8 ,  0 , 33,32,34,
/* STAND_2 */ 3 , 8 ,  0 , 35,36,35,
/* STAND_3 */ 6 , 8 ,  0 , 37,37,38,39,39,39,
/*   RUN   */ 8 , 8 ,  1 , 40,41,42,43,44,45,46,47,
/*  HURT   */ 2 , 8 ,  1 , 48,49,
/*  FALL   */ 4 , 8 ,  1 , 50,51,52,53,
/*  JUMP   */ 4 , 8 ,  1 , 54,55,56,57,
/*  ATK_0  */ 5 , 8 ,  0 , 58,58,59,59,59,
/*  ATK_1  */ 4 , 8 ,  0 , 60,61,61,61
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
    /** How long since the last animation started */
    int lastAnimElapsed;
    /** How long until the next idle animation */
    int nextIdle;
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
    _pGirl->lastAnimElapsed = 0;

    if (anim == STAND_0) {
        _pGirl->nextIdle = (rand() % 25 + 1) * 80 + 500;
    }

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
    rv = grlPl_playAnim(STAND_0);
    ASSERT(rv == GFMRV_OK, rv);

    /** Set gravity */
    //rv = gfmSprite_setVerticalAcceleration(_pGirl->pSprite, GRAV);
    //ASSERT(rv == GFMRV_OK, rv);

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
    /** GFraMe return value */
    gfmRV rv;

    /* Check if the idle animation should be updated */
    _pGirl->lastAnimElapsed += pGame->elapsed;
    if (_pGirl->curAnim <= STAND_0) {
        if (_pGirl->lastAnimElapsed >= _pGirl->nextIdle) {
            /* Pseudo-random next animation */
            int nextIdleAnim;

            nextIdleAnim = rand() % 3 + 1;
            rv = grlPl_playAnim(nextIdleAnim);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }
    else if (_pGirl->curAnim <= STAND_3) {
        rv = gfmSprite_didAnimationFinish(_pGirl->pSprite);
        ASSERT(rv == GFMRV_TRUE || rv == GFMRV_FALSE, rv);

        if (rv == GFMRV_TRUE) {
            rv = grlPl_playAnim(STAND_0);
            ASSERT(rv == GFMRV_OK, rv);
        }
    }

    rv = gfmSprite_update(_pGirl->pSprite, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

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
    return GFMRV_OK;
}

/**
 * Draw the sprite
 *
 * @return GFraMe return value
 */
gfmRV grlPl_draw() {
    return gfmSprite_draw(_pGirl->pSprite, pGame->pCtx);
}

