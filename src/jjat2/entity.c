/**
 * @file src/jjat2/entity.c
 *
 * Base structure for physical entity (player & mobs)
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>

#include <conf/type.h>

#include <jjat2/entity.h>
#include <jjat2/playstate.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmInput.h>
#include <GFraMe/gfmSprite.h>

/**
 * Initialize the entity based on the previously set attributes
 *
 * @param  [ in]entity    The entity
 */
void initEntity(entityCtx *entity) {
    gfmSprite_setVerticalAcceleration(entity->pSelf, entity->standGravity);
    entity->flags = EF_ALIVE;
}

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
    entity->currentAnimation = animation;

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

    return erv;
}

/**
 * Collide the entity's sprite only against the static world
 *
 * @param  [ in]entity The entity
 */
err collideEntityStatic(entityCtx *entity) {
    err erv;
    gfmRV rv;
    if (entity->flags & EF_SKIP_COLLISION) {
        return ERR_OK;
    }
    rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pStaticQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    return ERR_OK;
}

/**
 * Collide the entity's sprite against the world
 *
 * @param  [ in]entity The entity
 */
err collideEntity(entityCtx *entity) {
    err erv;
    gfmRV rv;
    if (entity->flags & EF_SKIP_COLLISION) {
        return ERR_OK;
    }
    rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pStaticQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    if (entity->baseType == T_PLAYER) {
        /* Colliding a second time againt the world QT solves a few corner
         * cases... */
        rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            erv = doCollide(collision.pStaticQt);
            ASSERT(erv == ERR_OK, erv);
            rv = GFMRV_QUADTREE_DONE;
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    }

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
 * Handle fixing an entity's position based on its carrier
 *
 * @param  [ in]entity   The entity
 */
static err handleCarrying(entityCtx *entity) {
    gfmSprite *pCarrierSpr;
    double vy;
    err erv;
    gfmRV rv;

    /* Ensure the bottom-most entity and handled first. This makes horizontal
     * movement be correctly propagated through the entities */
    if (!entity->pCarrying || (entity->flags & EF_HAS_CARRIER)) {
        return ERR_OK;
    }
    else if (entity->pCarrying->pCarrying) {
        erv = handleCarrying(entity->pCarrying);
        ASSERT(erv == ERR_OK, erv);
    }

    pCarrierSpr = entity->pCarrying->pSelf;

    /* Get the collision flags as this started, update the entity's position and
     * check the collision flags again to check for position adjustments */
    gfmSprite_applyDeltaX(entity->pSelf, pCarrierSpr);

    /* Get carrying's VY (into vy) */
    gfmSprite_getVerticalVelocity(&vy, pCarrierSpr);

    /* Collide to actually set the vertical position */
    gfmSprite_setFixed(pCarrierSpr);
    gfmSprite_justOverlaped(entity->pSelf, pCarrierSpr);
    gfmSprite_separateVertical(entity->pSelf, pCarrierSpr);
    gfmSprite_setMovable(pCarrierSpr);

    /* Update the entity vertical velocity (make it fall slightly faster and
     * avoid getting separated from the object) */
    if (vy >= TILES_TO_PX(2)) {
        double ay;
        gfmSprite_getVerticalAcceleration(&ay, pCarrierSpr);
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
    if (vy < MAX_FALL_SPEED) {
        gfmSprite_setVerticalVelocity(entity->pSelf, vy);
    }
    else {
        gfmSprite_setVerticalVelocity(entity->pSelf, MAX_FALL_SPEED);
    }

    /* Collide against static objects */
    rv = gfmQuadtree_collideSprite(collision.pStaticQt, entity->pSelf);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pStaticQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);

    entity->flags |= EF_HAS_CARRIER;

    return ERR_OK;
}

/**
 * Finalize updating the entity's physics
 *
 * @param  [ in]entity   The entity
 */
err preUpdateEntity(entityCtx *entity) {
    double vy;
    gfmRV rv;
    err erv;

    entity->flags &= ~(EF_HAS_CARRIER);
    entity->pCarrying = 0;

    rv = gfmSprite_getVerticalVelocity(&vy, entity->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (vy > 0) {
        /* Set fall gravity */
        gfmSprite_setVerticalAcceleration(entity->pSelf, entity->fallGravity);
    }
    else if (vy < 0) {
        /* Set jump gravity */
        gfmSprite_setVerticalAcceleration(entity->pSelf, entity->standGravity);
    }

    if (vy >= MAX_FALL_SPEED) {
        gfmSprite_setVerticalVelocity(entity->pSelf, MAX_FALL_SPEED);
    }

    rv = gfmSprite_update(entity->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = collideEntity(entity);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
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
        return handleCarrying(entity);
    }

    return ERR_OK;
}

/**
 * Draw the entity
 *
 * @param  [ in]entity   The entity
 */
err drawEntity(entityCtx *entity) {
    gfmRV rv;
    err erv;

    rv = gfmSprite_draw(entity->pSelf, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if ((game.flags & FX_PRETTYRENDER) && entity->pCarrying) {
        erv = drawEntity(entity->pCarrying);
        ASSERT(erv == ERR_OK, erv);
    }

    return ERR_OK;
}

/**
 * If the entity is offscreen, draw an 8x8 icon on its edge
 *
 * @param [ in]entity The entity
 * @param [ in]tile   The index/tile of the entity's icon
 */
err drawEntityIcon(entityCtx *entity, int tile) {
    gfmRV rv;
    int ch, cw, cx, cy, flip, x, y;

    if (gfmCamera_isSpriteInside(game.pCamera, entity->pSelf) == GFMRV_TRUE) {
        return ERR_OK;
    }

    gfmCamera_getPosition(&cx, &cy, game.pCamera);
    gfmCamera_getDimensions(&cw, &ch, game.pCamera);
    gfmSprite_getCenter(&x, &y, entity->pSelf);
    gfmSprite_getDirection(&flip, entity->pSelf);

    /* Convert the sprite's position from world space to "clamped screen
     * space" */
    if (x < cx) {
        x = 0;
    }
    else if (x > cx + cw) {
        x = cw - 8;
    }
    else {
        x -= cx;
    }

    if (y < cy) {
        y = 0;
    }
    else if (y > cy + ch) {
        y = ch - 8;
    }
    else {
        y -= cy;
    }

    rv = gfm_drawTile(game.pCtx, gfx.pSset8x8, x, y, tile, flip);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Updates an entity's direction according to its velocity.
 *
 * By default, the sprite is considered to be facing right.
 *
 * @param  [ in]entity   The entity
 */
void setEntityDirection(entityCtx *entity) {
    double vx;

    gfmSprite_getHorizontalVelocity(&vx, entity->pSelf);
    if (vx > 0) {
        gfmSprite_setDirection(entity->pSelf, DIR_RIGHT);
    }
    else if (vx < 0) {
        gfmSprite_setDirection(entity->pSelf, DIR_LEFT);
    }
}

/**
 * Simple collision check between two entities
 *
 * If this function detects collision, it ensures one entity will be carried by
 * the other, and only that.
 *
 * @param  [ in]entA One of the entities
 * @param  [ in]entB The other entity
 */
void collideTwoEntities(entityCtx *entA, entityCtx *entB) {
    gfmRV rv;

    rv = gfmSprite_justOverlaped(entA->pSelf, entB->pSelf);
    if (rv == GFMRV_TRUE) {
        gfmCollision adir, bdir;
        gfmSprite_getCurrentCollision(&adir, entA->pSelf);
        gfmSprite_getCurrentCollision(&bdir, entB->pSelf);
        if (adir & gfmCollision_down) {
            /* entA is above entB */
            entA->pCarrying = entB;
        }
        else if (bdir & gfmCollision_down) {
            /* entB is above entA */
            entB->pCarrying = entA;
        }
    }
}

/**
 * Do some damage to the entity
 *
 * @param  [ in]entity The entity
 * @param  [ in]damage How much damage was dealt
 */
void hitEntity(entityCtx *entity, int damage) {
    /* TODO Accumulate damage before killing the entity */
    entity->flags &= ~EF_ALIVE;
}

/**
 * Kill the entity
 *
 * @param  [ in]entity The entity
 */
void killEntity(entityCtx *entity) {
    entity->flags &= ~EF_ALIVE;
}

/**
 * Flip the entity if it reaches an edge
 *
 * @param  [ in]entity The entity
 * @param  [ in]vx     The entity's velocity
 */
void flipEntityOnEdge(entityCtx *entity, double vx) {
    gfmRV rv;
    int dir, h, type, w, x, y;

    gfmSprite_getDimensions(&w, &h, entity->pSelf);
    gfmSprite_getDirection(&dir, entity->pSelf);
    gfmSprite_getPosition(&x, &y, entity->pSelf);
    y += h;
    if (dir == DIR_LEFT) {
        x--;
    }
    else if (dir == DIR_RIGHT) {
        x += w + 1;
    }

    rv = gfmTilemap_getTypeAt(&type, playstate.pMap, x, y);
    if (rv == GFMRV_TILEMAP_NO_TILETYPE) {
        if (dir == DIR_LEFT) {
            /* Flip to move right */
            gfmSprite_setHorizontalVelocity(entity->pSelf, vx);
        }
        else if (dir == DIR_RIGHT){
            /* Flip to move left */
            gfmSprite_setHorizontalVelocity(entity->pSelf, -vx);
        }
    }
    else {
        double curVx;

        /* Otherwise, guarantee that it is moving */
        gfmSprite_getHorizontalVelocity(&curVx, entity->pSelf);
        if (curVx == 0) {
            gfmSprite_setHorizontalVelocity(entity->pSelf, vx);
        }
    }
}
