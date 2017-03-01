/**
 * @file src/jjat/hitbox.c
 *
 * Manages fixed hitboxes, that last until the next level is loaded, and
 * transient ones, that last until the following frame.
 */
#include <base/collision.h>
#include <base/error.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmHitbox.h>
#include <jjat2/hitbox.h>

/** Initialize the context */
err initHitboxes() {
    gfmRV rv;

    rv = gfmHitbox_getNewList(&hitboxes.pList, MAX_HITBOXES);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Free the context */
void freeHitboxes() {
    if (hitboxes.pList) {
        gfmHitbox_free(&hitboxes.pList);
    }
}

/** Reset the context for recycling */
void resetHitboxes() {
    hitboxes.used = 0;
    hitboxes.tmpUsed = 0;
}

/** Reset only the transient hitboxes for recycling (should be called every
 * frame) */
void resetTmpHitboxes() {
    hitboxes.tmpUsed = 0;
}

/**
 * Spawn a hitbox within the list
 *
 * @param  [ in]pCtx   Context associated with the hitbox
 * @param  [ in]x      The hitbox position
 * @param  [ in]y      The hitbox position
 * @param  [ in]width  The hitbox dimensions
 * @param  [ in]height The hitbox dimensions
 * @param  [ in]type   The hitbox type
 * @param  [ in]i      The index within the list
 */
static gfmHitbox* _spawnHitboxAt(void *pCtx, int x, int y, int width, int height
        , int type, int i) {
    gfmHitbox* pHitbox;
    gfmRV rv;

    rv = gfmHitbox_initItem(hitboxes.pList, pCtx, x, y, width, height, type, i);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmHitbox_getItem(&pHitbox, hitboxes.pList, i);
    ASSERT(rv == GFMRV_OK, 0);

    return pHitbox;
}

/**
 * Spawn a hitbox that should last until changing the level. It returns the
 * spawned object.
 *
 * @param  [ in]pCtx   Context associated with the hitbox
 * @param  [ in]x      The hitbox position
 * @param  [ in]y      The hitbox position
 * @param  [ in]width  The hitbox dimensions
 * @param  [ in]height The hitbox dimensions
 * @param  [ in]type   The hitbox type
 */
gfmHitbox* spawnFixedHitbox(void *pCtx, int x, int y, int width, int height, int type) {
    gfmHitbox* pHitbox;

    ASSERT(hitboxes.used + hitboxes.tmpUsed < MAX_HITBOXES, 0);

    pHitbox = _spawnHitboxAt(pCtx, x, y, width, height, type, hitboxes.used);

    hitboxes.used++;
    return pHitbox;
}

/**
 * Spawn a hitbox that should last until the next frame. It returns the
 * spawned object.
 *
 * @param  [ in]pCtx   Context associated with the hitbox
 * @param  [ in]x      The hitbox position
 * @param  [ in]y      The hitbox position
 * @param  [ in]width  The hitbox dimensions
 * @param  [ in]height The hitbox dimensions
 * @param  [ in]type   The hitbox type
 */
gfmHitbox* spawnTmpHitbox(void *pCtx, int x, int y, int width, int height, int type) {
    gfmHitbox* pHitbox;

    ASSERT(hitboxes.used + hitboxes.tmpUsed < MAX_HITBOXES, 0);

    pHitbox = _spawnHitboxAt(pCtx, x, y, width, height, type
            , MAX_HITBOXES - hitboxes.tmpUsed - 1);

    hitboxes.tmpUsed++;
    return pHitbox;
}

/** Collide every hitbox */
err collideHitbox() {
    err erv;
    gfmRV rv;
    int i;

    i = 0;
    while (i < hitboxes.used) {
        rv = gfmHitbox_collideSubList(&i, hitboxes.pList, collision.pStaticQt
                , hitboxes.used);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            erv = doCollide(collision.pStaticQt);
            ASSERT(erv == ERR_OK, erv);
            rv = GFMRV_QUADTREE_DONE;
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    }
    i = 0;
    while (i < hitboxes.used) {
        rv = gfmHitbox_collideSubList(&i, hitboxes.pList, collision.pQt
                , hitboxes.used);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            erv = doCollide(collision.pQt);
            ASSERT(erv == ERR_OK, erv);
            rv = GFMRV_QUADTREE_DONE;
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    }

    i = MAX_HITBOXES - hitboxes.tmpUsed;
    while (i < MAX_HITBOXES) {
        rv = gfmHitbox_collideSubList(&i, hitboxes.pList, collision.pStaticQt
                , MAX_HITBOXES - 1);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            erv = doCollide(collision.pStaticQt);
            ASSERT(erv == ERR_OK, erv);
            rv = GFMRV_QUADTREE_DONE;
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    }
    i = MAX_HITBOXES - hitboxes.tmpUsed;
    while (i < MAX_HITBOXES) {
        rv = gfmHitbox_collideSubList(&i, hitboxes.pList, collision.pQt
                , MAX_HITBOXES - 1);
        if (rv == GFMRV_QUADTREE_OVERLAPED) {
            erv = doCollide(collision.pQt);
            ASSERT(erv == ERR_OK, erv);
            rv = GFMRV_QUADTREE_DONE;
        }
        ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    }

    return ERR_OK;
}

