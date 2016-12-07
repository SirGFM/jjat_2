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
 * @param  [ in]x Target's center
 * @param  [ in]y Target's center
 */
err teleporterTargetPosition(int x, int y) {
    gfmSprite *pEffect;
    gfmRV rv;
    int type;

    cleanPreviousTarget();

    pEffect = spawnFx(x - 4, y - 4, 8/*w*/, 8/*h*/, 0/*dir*/, 0/*ttl*/
            , FX_TELEPORT_TARGET, T_FX);
    ASSERT(pEffect, ERR_GFMERR);
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
    erv = teleporterTargetPosition(0, 0);
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
 * Teleport an entity to the current target
 *
 * @param  [ in]pEnt The entity
 */
err teleportEntity(entityCtx *pEnt) {
    gfmSprite *pEffect;
    int cx, cy;
    err erv;
    gfmRV rv;

    if (teleport.pCurEffect == 0) {
        return ERR_OK;
    }

    /* Swap the target, if any */
    if (teleport.pTarget != 0) {
        rv = gfmSprite_getCenter(&cx, &cy, pEnt->pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmSprite_setCenter(teleport.pTarget->pSelf, cx, cy);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        erv = collideEntityStatic(teleport.pTarget);
        ASSERT(erv == ERR_OK, erv);
    }

    /* Swap the entity */
    rv = gfmGroup_getNodeSprite(&pEffect, teleport.pCurEffect);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_getCenter(&cx, &cy, pEffect);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_setCenter(pEnt->pSelf, cx, cy);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = collideEntityStatic(pEnt);
    ASSERT(erv == ERR_OK, erv);

    cleanPreviousTarget();

    return ERR_OK;
}

