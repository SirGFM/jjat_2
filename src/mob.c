/**
 * @file src/mob.c
 *
 * Generic 'class' that handles most
 */
#include <base/collision.h>
#include <base/game_ctx.h>
#include <base/game_const.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <jjat_2/mob.h>
#include <jjat_2/type.h>

#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
#  include <stdlib.h>
#  include <signal.h>
#endif

/**
 * Update the currently playing animation
 *
 * @param  [ in]pMob The mob
 * @param  [ in]anim The animation's index
 * @return           GFraMe return value
 */
gfmRV mob_playAnim(mob *pMob, int anim) {
    /** GFraMe return value */
    gfmRV rv;

    /* Only change the animation if it wasn't playing */
    if (anim == pMob->curAnim) {
        return GFMRV_OK;
    }

    rv = gfmSprite_playAnimation(pMob->pSpr, anim);
    ASSERT(rv == GFMRV_OK, rv);
    pMob->curAnim = anim;
    pMob->lastAnimElapsed = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Calls its sub-class "pre-update", updates the mob (physically) and collide it
 * against the world
 *
 * This step should update the AI for enemies and handle inputs for the player
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_update(mob *pMob) {
    /** GFraMe return value */
    gfmRV rv;

    /* Update the sub-class */
    switch (pMob->type) {
        /* TODO Enable girl update */
#if 0
        case T_GIRL: {
            rv = girl_preUpdate((girl*)pMob->pChild);
        } break;
#endif
        /* "Trap" unhandled cases on the debug version so no type is ever
         * forgotten */
        default: {
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
            /* Unfiltered collision, do something about it */
            raise(SIGINT);
#  endif
            rv = GFMRV_INTERNAL_ERROR;
        } /* default */
    } /* switch (pMob->type) */
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmSprite_update(pMob->pSpr, pGame->pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    /** Check if any collisions happened and handle it */
    rv = gfmQuadtree_collideSprite(pGlobal->pQt, pMob->pSpr);
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
 * Handles standing over another mob, being actually hurt (?) and calls the
 * sub-class "post-update". There, each different class should update its own
 * animations and what-not.
 *
 * NOTE: Must be called after all sprites have collided
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_postUpdate(mob *pMob) {
    /* TODO Implement this */
    return GFMRV_OK;
}

/**
 * Draw the sprite
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_draw(mob *pMob) {
    return gfmSprite_draw(pMob->pSpr, pGame->pCtx);
}

