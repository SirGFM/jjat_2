/**
 * @file src/jjat2/entity.c
 *
 * Base structure for physical entity (player & mobs)
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>

#include <jjat2/entity.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
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

/**
 * Update the entity's jump, based on the last frame and the jump button. This
 * function also updates the gravity (if fall gravity is different from jump).
 *
 * Changing to a jump state/animation is delegated to the caller. If jump gets
 * issued, the sprite's vertical velocity is set accordingly and ERR_DIDJUMP is
 * returned.
 *
 * @param  [ in]entity The entity
 * @param  [ in]jumpBt State of the jump button
 */
err updateEntityJump(entityCtx *entity, gfmInputState jumpBt) {
    double vy;
    gfmCollision dir;
    gfmRV rv;
    /* This function either exit on a failed assert (therefore, with a custom
     * error code) or with whatever erv is set to. By default, it exits with
     * ERR_OK, however, on jump, erv is set to ERR_DIDJUMP. */
    err erv = ERR_OK;

    /* Reset the jump grace time whenerver the entity is grounded */
    rv = gfmSprite_getCollision(&dir, entity->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    if (dir & gfmCollision_down) {
        entity->jumpGrace = DEF_JUMP_GRACE;
    }

    if (entity->jumpGrace > 0 && IS_STATE_JUSTPRESSED(jumpBt)) {
        /* Convetional jump */
        vy = entity->jumpVelocity;
        rv = gfmSprite_setVerticalVelocity(entity->pSelf, vy);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        entity->jumpGrace = 0;
        erv = ERR_DIDJUMP;
    }
    else if (IS_STATE_JUSTRELEASED(jumpBt)) {
        /* Activate shorthop.
         * A theoretical bug is that releasing the button more than once may
         * lead to even shorter hops. However, this shouldn't be a real issue
         * (nor a feasible tech) */

        rv = gfmSprite_getVerticalVelocity(&vy, entity->pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (vy < entity->shorthopVelocity * 2) {
            vy *= 0.5;
        }
        else if (vy < entity->shorthopVelocity) {
            vy = entity->shorthopVelocity;
        }

        rv = gfmSprite_setVerticalVelocity(entity->pSelf, vy);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        erv = ERR_DIDJUMP;
    }
    else {
        rv = gfmSprite_getVerticalVelocity(&vy, entity->pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    entity->jumpGrace -= game.elapsed;

    if (vy < 0) {
        /* Set jump gravity */
        rv = gfmSprite_setVerticalAcceleration(entity->pSelf,
                entity->standGravity);
    }
    else {
        /* Set fall gravity */
        rv = gfmSprite_setVerticalAcceleration(entity->pSelf,
                entity->fallGravity);
    }
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return erv;
}

/**
 * Collide the entity's sprite against the world
 *
 * @param  [ in]entity The entity
 */
err collideEntity(entityCtx *entity) {
    gfmRV rv;
    rv = gfmQuadtree_collideSprite(collision.pQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        return doCollide();
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    return ERR_OK;
}

/**
 * Make an entity be carried by another,
 *
 * NOTE: To avoid collision issues with the environment, any entity that is
 * being carried should run a second collision against all static objects. This
 * solves potential zipping through platforms.
 *
 * @param  [ in]entity   The entity
 * @param  [ in]carrying The sprite carrying the entity
 */
void carryEntity(entityCtx *entity, gfmSprite *carrying) {
    double vy;
    int h, selfX, lastX, x, y;

    /* Retrieve all required attributes from the carrying object */
    gfmSprite_getLastCenter(&lastX, &y, carrying);
    gfmSprite_getCenter(&x, &y, carrying);
    gfmSprite_getVerticalPosition(&y, carrying);
    gfmSprite_getVerticalVelocity(&vy, carrying);

    gfmSprite_getHorizontalPosition(&selfX, entity->pSelf);
    gfmSprite_getHeight(&h, entity->pSelf);

    /* TODO Copy the carrying's velocity, so the entity may walk away from the
     * carrying (manually setting the position zeros the decimal part and makes
     * the entity locked to the carrying, unless it jumps or the carrying stop moving) */
    /* Set the entity's position */
    gfmSprite_setPosition(entity->pSelf, selfX + (x - lastX), y - h);

    /* Update the entity position and velocity (make it fall slightly faster to
     * avoid getting separated from the object) */
    if (vy < 0) {
        vy *= 0.25;
    }
    else if (vy > 0) {
        vy *= 1.75;
    }
    gfmSprite_setVerticalVelocity(entity->pSelf, 0);

    /* TODO Set flag so the entity may collide again against every static
     * object (which will fix collision against the map) */
}

