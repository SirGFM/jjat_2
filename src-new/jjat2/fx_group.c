/**
 * @file src/jjat2/fx_group.h
 *
 * Defines a group of sprites used for effects and/or temporary hitboxes.
 * Entities within this group may be physically controlled, but they doesn't
 * have to be.
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/type.h>

#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmSprite.h>

#include <jjat2/fx_group.h>

static int pFxAnimData[] = {
/*                         len|fps|loop|data... */
/* FX_SWORDY_SLASH_DOWN */  4 , 12,  0 , 56,57,58,59
/*  FX_SWORDY_SLASH_UP  */, 4 , 12,  0 , 60,61,62,63
/*    FX_GUNNY_BULLET   */, 4 , 12,  1 , 92,93,94,95
};
static int fxAnimDataLen = sizeof(pFxAnimData) / sizeof(int);

/**
 * Spawn a new effect at the desired position
 *
 * @param  [ in]x       Top left position of the effect
 * @param  [ in]y       Top left position of the effect
 * @param  [ in]w       Effect's width
 * @param  [ in]h       Effect's height
 * @param  [ in]flipped Whether the effect should be horizontally flipped
 * @param  [ in]ttl     How long the effect should live. <= 0 for infinity.
 * @param  [ in]anim    Animation played by the effect
 * @param  [ in]t       The effect type (relevant on collision)
 * @return              The spawned effect (or 0 on failure)
 */
gfmSprite* spawnFx(int x, int y, int w, int h, int flipped, int ttl,
        fxAnim anim, type t) {
    gfmSprite *pSpr;
    gfmRV rv;

    /* Ugly hack: there's no way to modify a node's TTL... So modify the groups'
     * and then revert it back... */
    if (ttl > 0) {
        gfmGroup_setDeathOnTime(fx, ttl);
        rv = gfmGroup_recycle(&pSpr, fx);
        gfmGroup_setDeathOnTime(fx, -1/*dontDie*/);
    }
    else {
        rv = gfmGroup_recycle(&pSpr, fx);
    }
    ASSERT(rv == GFMRV_OK, 0);

    rv = gfmSprite_setPosition(pSpr, x, y);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setDimensions(pSpr, w, h);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setDirection(pSpr, flipped);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_setType(pSpr, t);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_playAnimation(pSpr, anim);
    ASSERT(rv == GFMRV_OK, 0);
    rv = gfmSprite_resetAnimation(pSpr);
    ASSERT(rv == GFMRV_OK, 0);

    return pSpr;
}

/** Initialize the group */
err initFxGroup() {
    gfmRV rv;

    rv = gfmGroup_getNew(&fx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmGroup_setDefType(fx, T_FX);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDefSpriteset(fx, gfx.pSset16x16);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDefDimensions(fx, 16/*w*/, 16/*h*/, 0/*offx*/, 0/*offy*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDeathOnLeave(fx, 0/*dontDie*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDeathOnTime(fx, -1/*dontDie*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setDefAnimData(fx, pFxAnimData, fxAnimDataLen);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv =  gfmGroup_setDrawOrder(fx, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_setCollisionQuality(fx
            , gfmCollisionQuality_collideEverything);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmGroup_preCache(fx, MAX_FX_NUM, MAX_FX_NUM);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Free up all memory hold by a group */
void freeFxGroup() {
    gfmGroup_free(&fx);
}

/** Update and collide the effects */
err updateFxGroup() {
    err erv;
    gfmRV rv;

    rv = gfmGroup_update(fx, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_collideGroup(collision.pStaticQt, fx);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pStaticQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);

    rv = gfmQuadtree_collideGroup(collision.pQt, fx);
    if (rv == GFMRV_QUADTREE_OVERLAPED) {
        erv = doCollide(collision.pQt);
        ASSERT(erv == ERR_OK, erv);
        rv = GFMRV_QUADTREE_DONE;
    }
    ASSERT(rv == GFMRV_QUADTREE_DONE, ERR_GFMERR);
    return ERR_OK;
}

