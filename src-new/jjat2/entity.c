/**
 * @file src/jjat2/entity.c
 *
 * Base structure for physical entity (player & mobs)
 */
#include <base/error.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmSprite.h>

/**
 * Set the current animation of the entity
 *
 * @param  [ in]entity    The entity
 * @param  [ in]animation Index of the animation to be played
 * @param  [ in]force     Whether the animation should be reset even if it was
 *                        being previously played
 */
err setEntityAnimation(entityCtx *entity, int animation, int force) {
    gfmRV rv;

    ASSERT(animation < entity->maxAnimation, ERR_INDEXOOB);

    /* It's not an error to play the same animation, but it should be ignored */
    if (animation != entity->currentAnimation || force) {
        rv = gfmSprite_playAnimation(entity->pSelf, animation);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmSprite_resetAnimation(entity->pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    return ERR_OK;
}

