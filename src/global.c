/**
 * @file src/global.c
 *
 * Declare all global variables
 */
#include <base/game_const.h>
#include <base/game_ctx.h>
#include <base/global.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmGroup.h>

#include <jjat_2/entity.h>
#include <jjat_2/type.h>

/** Store data related to game */
gameCtx *pGame = 0;

/** Store all handles to texture and spritesets' pointers */
gfxCtx *pGfx = 0;

/** Store all handles to songs and sound effects */
audioCtx *pAudio = 0;

/** Store all actions do-able ingame */
buttonCtx *pButton = 0;

/** Store all data modifiably on the option menu, as well as anything that may
 * be saved on the config file */
configCtx *pConfig = 0;

/** Store game-related variables that should be globally accessible */
globalCtx *pGlobal = 0;

/**
 * Initialize all pointers
 *
 * @param  [ in]pMem A buffer of at least SIZEOF_GAME_MEM bytes
 */
void global_init(void *pMem) {
    /* Retrieve all pointers from the memory */
    pGame = (gameCtx*)OFFSET_MEM(pMem, GAME_OFFSET);
    pGfx = (gfxCtx*)OFFSET_MEM(pMem, GFX_OFFSET);
    pAudio = (audioCtx*)OFFSET_MEM(pMem, AUDIO_OFFSET);
    pButton = (buttonCtx*)OFFSET_MEM(pMem, BUTTON_OFFSET);
    pConfig = (configCtx*)OFFSET_MEM(pMem, CONFIG_OFFSET);
    pGlobal = (globalCtx*)OFFSET_MEM(pMem, GLOBAL_OFFSET);

    /* Set any pointers within those structs that were already alloc'ed */
    pConfig->pLast = (configCtx*)OFFSET_MEM(pMem, LASTCONFIG_OFFSET);
}

/**
 * Initialize all variables in pGlobal
 *
 * @return GFraMe return value
 */
gfmRV global_initUserVar() {
    /** Return value */
    gfmRV rv;

    /* Initialize everything */
    rv = gfmGroup_getNew(&(pGlobal->pHitbox));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefType(pGlobal->pHitbox, T_HITBOX);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefSpriteset(pGlobal->pHitbox, pGfx->pSset16x16);
    ASSERT(rv == GFMRV_OK, rv);
    /* TODO Set animation data and default hitbox */
#if 0
    rv = gfmGroup_setDefAnimData(pGlobal->pHitbox, int *pData, int len);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setDefDimensions(pGlobal->pHitbox, int width, int height, int offX,
        int offY);
    ASSERT(rv == GFMRV_OK, rv);
#endif /* 0 */
    /* No need to get fancy with this group ;) */
    rv = gfmGroup_setDrawOrder(pGlobal->pHitbox, gfmDrawOrder_linear);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmGroup_setCollisionQuality(pGlobal->pHitbox,
            gfmCollisionQuality_visibleOnly);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmQuadtree_getNew(&(pGlobal->pQt));
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmTilemap_getNew(&(pGlobal->pTMap));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfmTilemap_init(pGlobal->pTMap, pGfx->pSset8x8, MAP_MAX_WIDTH,
            MAP_MAX_HEIGHT, -1/*defTile*/);
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    return GFMRV_OK;
}

/**
 * Release all variables in pGlobal
 */
void global_freeUserVar() {
    gfmQuadtree_free(&(pGlobal->pQt));
    gfmGroup_free(&(pGlobal->pHitbox));
    gfmTilemap_free(&(pGlobal->pTMap));
    entity_free(&(pGlobal->pGirl));
    entity_free(&(pGlobal->pBoy));
}

