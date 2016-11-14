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
        /* Conventional jump */
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
    }
    else {
        rv = gfmSprite_getVerticalVelocity(&vy, entity->pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    entity->jumpGrace -= game.elapsed;

    if (vy > 0) {
        /* Set fall gravity */
        rv = gfmSprite_setVerticalAcceleration(entity->pSelf,
                entity->fallGravity);
    }
    else if (vy < 0) {
        /* Set jump gravity */
        rv = gfmSprite_setVerticalAcceleration(entity->pSelf,
                entity->standGravity);
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
    err erv;
    gfmRV rv;
    rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pStaticQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    rv = gfmQuadtree_collideSprite(collision.pQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
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
    int dx, x, tmp;
    gfmCollision beforeDir, afterDir;

    /* Retrieve the movement from carrying since the previous frame (into dx) */
    gfmSprite_getCenter(&dx, &tmp, carrying);
    gfmSprite_getLastCenter(&x, &tmp, carrying);
    dx -= x;
    /* Get entity's horizontal position (into x) */
    gfmSprite_getHorizontalPosition(&x, entity->pSelf);

    /* Get the collision flags as this started, update the entity's position and
     * check the collision flags again to check for position adjustments */
    gfmSprite_getCurrentCollision(&beforeDir, entity->pSelf);
    gfmSprite_setHorizontalPosition(entity->pSelf, x + dx);
    gfmSprite_justOverlaped(entity->pSelf, carrying);
    gfmSprite_getCurrentCollision(&afterDir, entity->pSelf);

    /* Adjustments happens on both corners, to ensure the entity doesn't get
     * stuck on carrying's edge */
    if ((beforeDir & gfmCollision_left) && !(afterDir & gfmCollision_left)) {
        gfmSprite_setHorizontalPosition(entity->pSelf, x + dx + 1);
    }
    else if ((beforeDir & gfmCollision_right)
            && !(afterDir & gfmCollision_right)) {
        gfmSprite_setHorizontalPosition(entity->pSelf, x + dx - 1);
    }
    else {
        /* Set flag so the entity may collide again against every static
         * object (which will fix collision against the map) */
        entity->pCarrying = carrying;
    }

}

/**
 * Post update an entity
 *
 * During this stage, the entity fixes its collision against the world and any
 * carrying sprite.
 *
 * @param  [ in]entity   The entity
 */
err postUpdateEntity(entityCtx *entity) {
    /* If the entity is being carried, collide against every static object and
     * then adjust its velocity */
    if (entity->pCarrying) {
        double vy;
        gfmRV rv;

        /* Get carrying's VY (into vy) */
        gfmSprite_getVerticalVelocity(&vy, entity->pCarrying);

        /* TODO Potential bug: If the carrying sprite is also being carried, its
         * collision must be resolved before this entity's collision */

        /* Collide to actually set the vertical position */
        gfmSprite_setFixed(entity->pCarrying);
        gfmSprite_separateVertical(entity->pSelf, entity->pCarrying);
        gfmSprite_setMovable(entity->pCarrying);

        /* Update the entity vertical velocity (make it fall slightly faster and
         * avoid getting separated from the object) */
        if (vy >= TILES_TO_PX(2)) {
            double ay;
            gfmSprite_getVerticalAcceleration(&ay, entity->pCarrying);
            vy = 1.06125 * (vy + ay * (game.elapsed * 0.001));
        }
        else if (vy >= -TILES_TO_PX(5)) {
            vy = TILES_TO_PX(5);
        }
        else if (vy < 0) {
            vy *= 0.125;
            if (vy >= -TILES_TO_PX(2)) {
                vy = TILES_TO_PX(1);
            }
        }
        gfmSprite_setVerticalVelocity(entity->pSelf, vy);

        /* Collide against static objects */
        rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            return doCollide(collision.pStaticQt);
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);

        entity->pCarrying = 0;
    }

    return ERR_OK;
}

