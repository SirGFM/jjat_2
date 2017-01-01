/**
 * @file src/jjat2/teleport.c
 *
 * Handles the teleporter gun.
 *
 * As soon as a bullet hits a valid target, a new effect shall be spawned. If
 * the target is an entity, the entity should be stored on 'pTarget'. Otherwise,
 * the target should be part of the tilemap and its position is only stored on
 * the effect's position.
 */
#include <base/collision.h>
#include <base/error.h>

#include <conf/type.h>

#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmObject.h>

#include <jjat2/entity.h>
#include <jjat2/fx_group.h>
#include <jjat2/teleport.h>

/** Remove the previous target */
void resetTeleporterTarget() {
    /* No need to remove the effect from the node since this will most likely be
     * called after reseting the fx group itself */
    teleport.pCurEffect = 0;
    teleport.pTarget = 0;
}

/** Clean up the previous target so a new one may be set */
static void cleanPreviousTarget() {
    if (teleport.pCurEffect) {
        /* TODO Spawn a trasitioning effect? */
        gfmGroup_removeNode(teleport.pCurEffect);
    }
    resetTeleporterTarget();
}

/**
 * Setup the teleport target at a static position
 *
 * @param  [ in]x   Target's center
 * @param  [ in]y   Target's center
 * @param  [ in]pos 'Relative' position of the target
 */
err teleporterTargetPosition(int x, int y, teleportPosition pos) {
    gfmSprite *pEffect;
    gfmRV rv;
    int type;

    cleanPreviousTarget();

    pEffect = spawnFx(x - 4, y - 4, 8/*w*/, 8/*h*/, 0/*dir*/, 0/*ttl*/
            , FX_TELEPORT_TARGET, T_FX);
    ASSERT(pEffect, ERR_GFMERR);
    if (pos == TP_RIGHT) {
        gfmSprite_setOffset(pEffect, 4/*offx*/, 0/*offy*/);
    }
    else if (pos == TP_LEFT) {
        gfmSprite_setOffset(pEffect, -4/*offx*/, 0/*offy*/);
    }
    else {
        gfmSprite_setOffset(pEffect, 0/*offx*/, 0/*offy*/);
    }
    rv = gfmSprite_getChild((void**)&teleport.pCurEffect, &type, pEffect);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Setup the teleport target at an entity
 *
 * @param  [ in]pTarget The entity
 */
err teleporterTargetEntity(entityCtx *pTarget) {
    err erv;
    /* No need to set the position right now since it will be updated shortly
     * afterward */
    erv = teleporterTargetPosition(0, 0, TP_ENTITY);
    ASSERT(erv == ERR_OK, erv);

    teleport.pTarget = pTarget;
    return ERR_OK;
}

/** Update the teleporter's position */
err updateTeleporterTarget() {
    gfmSprite *pEffect;
    gfmRV rv;
    int cx, cy;

    if (teleport.pTarget == 0 || teleport.pCurEffect == 0) {
        return ERR_OK;
    }

    rv = gfmSprite_getCenter(&cx, &cy, teleport.pTarget->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_getNodeSprite(&pEffect, teleport.pCurEffect);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setPosition(pEffect, cx - 4, cy - 4);

    return ERR_OK;
}

/**
 * Get the centralized horizontal position and the bottom vertical
 *
 * @param  [out]pX   The horizontal position
 * @param  [out]pY   The vertical position
 * @param  [ in]pSpr The sprite
 */
static void _getSpriteBottom(int *pX, int *pY, gfmSprite *pSpr) {
    int w, h;

    gfmSprite_getPosition(pX, pY, pSpr);
    gfmSprite_getDimensions(&w, &h, pSpr);
    *pX += w / 2;
    *pY += h;
}

/**
 * Set the position based on the horizontal center and bottom
 *
 * @param  [ in]pSpr The sprite
 * @param  [ in]x    The horizontal position
 * @param  [ in]y    The vertical position
 */
static void _setSpriteBottom(gfmSprite *pSpr, int x, int y) {
    int w, h;

    gfmSprite_getDimensions(&w, &h, pSpr);
    x -= w / 2;
    y -= h;
    gfmSprite_setPosition(pSpr, x, y);
}

/**
 * Ensure that entities that are teleport have their vertical velocity correctly
 * set
 *
 * @param  [ in]pEnt The entity
 */
static void _fixFloatBug(entityCtx *pEnt) {
    double vy;

    gfmSprite_getVerticalVelocity(&vy, pEnt->pSelf);
    if (vy == 0) {
        gfmSprite_setVerticalAcceleration(pEnt->pSelf, pEnt->fallGravity);
    }
}

/**
 * Teleport an entity to the current target
 *
 * @param  [ in]pEnt The entity
 */
err teleportEntity(entityCtx *pEnt) {
    /** dst-xy: the target's position */
    int dx, dy;
    err erv;

    /**
     * NOTE: Both entities should be put on a falling state (i.e., have a
     * positive gravity). Otherwise, they may end up stuck floating in the air.
     */

    if (teleport.pCurEffect == 0) {
        return ERR_OK;
    }

    /* Swap the target, if any */
    if (teleport.pTarget != 0) {
        /** src-xy: pEnt's position */
        int sx, sy;

        _getSpriteBottom(&sx, &sy, pEnt->pSelf);
        _getSpriteBottom(&dx, &dy, teleport.pTarget->pSelf);

        _setSpriteBottom(teleport.pTarget->pSelf, sx, sy);
        collision.flags |= CF_FIXTELEPORT;
        erv = collideEntityStatic(teleport.pTarget);
        collision.flags &= ~CF_FIXTELEPORT;
        ASSERT(erv == ERR_OK, erv);
        _fixFloatBug(teleport.pTarget);
    }
    else {
        gfmSprite *pEffect;
        gfmRV rv;

        rv = gfmGroup_getNodeSprite(&pEffect, teleport.pCurEffect);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        _getSpriteBottom(&dx, &dy, pEffect);
    }

    /* Swap the entity */
    _setSpriteBottom(pEnt->pSelf, dx, dy);
    collision.flags |= CF_FIXTELEPORT;
    erv = collideEntityStatic(pEnt);
    collision.flags &= ~CF_FIXTELEPORT;
    ASSERT(erv == ERR_OK, erv);
    _fixFloatBug(pEnt);

    cleanPreviousTarget();

    return ERR_OK;
}

