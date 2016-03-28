/**
 * Generic entity
 */
#include <base/collision.h>
#include <base/game_ctx.h>
#include <base/game_const.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <stdlib.h>

/**
 * Release an entity and all of its resources
 *
 * @param  [out] ppEnt The entity
 */
void entity_free(entity **ppEnt) {
    if (!ppEnt) {
        return;
    }
    gfmSprite_free(&((*ppEnt)->pSprite));
    free(*ppEnt);
    *ppEnt = 0;
}

/**
 * Update the currently playing animation
 *
 * @param  [ in]pEnt The entity
 * @param  [ in]anim The animation's index
 * @return           GFraMe return value
 */
gfmRV entity_playAnimation(entity *pEnt, int anim) {
    /** GFraMe return value */
    gfmRV rv;

    /* Only change the animation if it wasn't playing */
    if (anim == pEnt->curAnim) {
        return GFMRV_OK;
    }

    rv = gfmSprite_playAnimation(pEnt->pSprite, anim);
    ASSERT(rv == GFMRV_OK, rv);
    pEnt->curAnim = anim;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * If the entity is standing on another one, update it
 *
 * @param  [ in]pEnt The entity
 * @return           GFraMe return value
 */
gfmRV entity_updateStanding(entity *pEnt) {
    gfmRV rv;
    int dx, dy, x, y;

    /* Exit early if no on an object */
    if (!pEnt->pStanding) {
        return GFMRV_OK;
    }

    /* Calculate the translation since the previous frame */
    rv = gfmObject_getCenter(&x, &y, pEnt->pStanding);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmObject_getLastCenter(&dx, &dy, pEnt->pStanding);
    ASSERT(rv == GFMRV_OK, rv);

    dx = x - dx;
    dy = y - dy;

    /* Horizontal movement is (erm, should be) easy, just add is to the current
     * position. However, vertical movement can be a pain when falling...
     * (raising movement should be automatic).
     *
     * To be fair, if this were to be done properly, a new collision should be
     * issued for this one entity. However, since setPosition doesn't change the
     * 'last position' anymore, it shouldn't be necessary (albeit with some 1
     * frame oddities being rendered).
     *
     * Actually, it may be an actual problem. See, usually update is called
     * before adding the entity to the quadtree, so the 'last position' may
     * already be inside another object... if it comes to that (and it probably
     * will), simply re-add the entity to the quadtree at the end of this
     * function.
     * I wonder how long it will take me to find this comment when that
     * happens... */
    rv = gfmSprite_getPosition(&x, &y, pEnt->pSprite);
    ASSERT(rv == GFMRV_OK, rv);

    x += dx;
    if (dy > 0) {
        y += dy;
    }

    rv = gfmSprite_setPosition(pEnt->pSprite, x, y);
    ASSERT(rv == GFMRV_OK, rv);

    /* Clear the standing object */
    pEnt->pStanding = 0;

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Collide the entity's sprite against the world
 *
 * @param  [ in]pEnt The entity
 * @return           GFraMe return value
 */
gfmRV entity_collide(entity *pEnt) {
    gfmRV rv;

    rv = gfmQuadtree_collideSprite(pGlobal->pQt, pEnt->pSprite);
    ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE, rv);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        rv = collision_run();
        ASSERT(rv == GFMRV_OK, rv);
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

