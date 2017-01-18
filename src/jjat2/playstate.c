/**
 * @file src/jjat2/playstate.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <jjat2/camera.h>
#include <jjat2/dictionary.h>
#include <jjat2/fx_group.h>
#include <jjat2/enemy.h>
#include <jjat2/gunny.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/swordy.h>
#include <jjat2/teleport.h>

#include <string.h>

#define PF_TEL_SHIFT 4
enum {
    PF_TEL_SWORDY = 0x01
  , PF_TEL_GUNNY  = 0x02
  , PF_TEL_LEVEL  = 0xf0
};
#define _setLevelIndex(index) \
    ((index << PF_TEL_SHIFT) & PF_TEL_LEVEL)
#define _getLevelIndex(flags) \
    ((flags & PF_TEL_LEVEL) >> PF_TEL_SHIFT)
#define _noLevelIndex \
    (PF_TEL_LEVEL >> PF_TEL_SHIFT)

/** Initialize the playstate so a level may be later loaded and played */
err initPlaystate() {
    gfmRV rv;
    err erv;
    int i;

    erv = initSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = initGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmParser_getNew(&playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_getNew(&playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(playstate.pMap, gfx.pSset8x8, TM_MAX_WIDTH
            , TM_MAX_HEIGHT, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_getNew(&playstate.pBackground);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(playstate.pBackground, gfx.pSset8x8, TM_MAX_WIDTH
            , TM_MAX_HEIGHT, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmSprite_getNew(&playstate.asyncDummy.pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(playstate.asyncDummy.pSelf, 0, 0, 2/*w*/, 2/*h*/
            , gfx.pSset8x8, 0/*offx*/, 0/*offy*/, &playstate.asyncDummy
            , T_PLAYER);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    i = 0;
    while (i < MAX_ENTITIES) {
        rv = gfmSprite_getNew(&playstate.entities[i].pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        i++;
    }

    playstate.flags |= PF_TEL_LEVEL;

    return ERR_OK;
}

/** If the playstate has been initialized, properly free it up. */
void freePlaystate() {
    int i;

    if (playstate.pMap != 0) {
        gfmTilemap_free(&playstate.pMap);
    }
    if (playstate.pBackground != 0) {
        gfmTilemap_free(&playstate.pBackground);
    }
    if (playstate.pParser != 0) {
        gfmParser_free(&playstate.pParser);
    }
    gfmSprite_free(&playstate.asyncDummy.pSelf);
    freeSwordy(&playstate.swordy);
    freeGunny(&playstate.gunny);

    i = 0;
    while (i < MAX_ENTITIES) {
        gfmSprite_free(&playstate.entities[i].pSelf);
        i++;
    }
}

/**
 * Setup loading the next map.
 *
 * @param  [ in]type Type of entity that entered the loadzone
 */
void onHitLoadzone(int type, int levelType) {
    int curLevel, level;

    /* Check if the triggered level is the same as the stored one (or if no
     * level has been set yet) */
    curLevel = _getLevelIndex(playstate.flags);
    level = getLoadzoneIndex(levelType);
    if (curLevel != _noLevelIndex && curLevel != level) {
        return;
    }

    if (type == T_SWORDY) {
        playstate.flags |= PF_TEL_SWORDY;
    }
    else if (type == T_GUNNY) {
        playstate.flags |= PF_TEL_GUNNY;
    }

    playstate.flags &= ~PF_TEL_LEVEL;
    playstate.flags |= _setLevelIndex(level);
}

/** Updates the quadtree's bounds according to the currently loaded map */
static err _updateWorldSize() {
    gfmRV rv;
    int height, width;

    rv = gfmTilemap_getDimension(&width, &height, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    playstate.width = width;
    playstate.height = height;

    rv = gfmCamera_setWorldDimensions(game.pCamera, playstate.width
            , playstate.height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /** Make the quadtree 4 tiles larger than the actual map */
    playstate.width += 32;
    playstate.height += 32;

    return ERR_OK;
}

/** Load the static quadtree */
static err _loadStaticQuadtree() {
    gfmRV rv;
    err erv;

    rv = gfmQuadtree_initRoot(collision.pStaticQt, -16/*x*/, -16/*y*/
            , playstate.width, playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_setStatic(collision.pStaticQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_populateTilemap(collision.pStaticQt, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = calculateStaticLeveltransition();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Load a level into the playstate
 *
 * Levels are expected to be on 'assets/levels/', and every level should have a
 * tilemap layer and an object layer, which should be named *_tm.gfm and
 * *_obj.gfm respectively.
 *
 * @param  [ in]levelName The name of the level, which will get the directory
 *                        prepended and then the layers and extension. Therefore
 *                        to load the level with layers on
 *                        "levels/map_test_tm.gfm" and "levels/map_test_obj.gfm"
 *                        one should call this function with "map_test" as its
 *                        argument.
 * @param  [ in]setPlayer Whether the player position should be set from the map
 */
static err _loadLevel(char *levelName, int setPlayer) {
    char stLevelName[MAX_LEVEL_NAME];
    int len, pos;
    gfmRV rv;
    err erv;

/** Length of a static string */
#define LEN(str) (sizeof(str) - 1)
/** Append a static string to the level name */
#define APPEND(str) \
    do { \
        memcpy(stLevelName + pos, str, sizeof(str)); \
    } while (0)
/** Append a static string to the level name and updates its position */
#define APPEND_POS(str) \
    do { \
        APPEND(str); \
        pos += LEN(str); \
    } while (0)
/** Append a dynamic string to the level name and updates its position */
#define APPEND_DYN(str, len) \
    do { \
        memcpy(stLevelName + pos, str, len); \
        pos += len; \
    } while (0)

    ASSERT(levelName != 0, ERR_ARGUMENTBAD);

    /* Level names will get "levels/", "_tm.gfm" and "_obj.gfm" concatenated, so
     * it must be at most this many characters long:
     *     MAX_LEVEL_NAME - (sizeof("levels/") - 1) - (sizeof("_obj.gfm") - 1) - 1
     * The last "- 1" is for the '\0'
     */
    len = strlen(levelName);
    ASSERT(len <= MAX_LEVEL_NAME, ERR_INVALIDLEVELNAME);

    pos = 0;
    APPEND_POS("levels/");
    /* TODO Check if levelName is actually a valid path */
    APPEND_DYN(levelName, len);

    /* Load the tilemap */
    APPEND("_fg_tm.gfm");
    rv = gfmTilemap_loadf(playstate.pMap, game.pCtx, stLevelName
            , pos + LEN("_fg_tm.gfm"), pDictNames, pDictTypes, dictLen);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = _updateWorldSize();
    ASSERT(erv == ERR_OK, erv);

    if (game.flags & FX_PRETTYRENDER) {
        APPEND("_bg_tm.gfm");
        rv = gfmTilemap_loadf(playstate.pBackground, game.pCtx, stLevelName
                , pos + LEN("_bg_tm.gfm"), pDictNames, pDictTypes, dictLen);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    /* Load the objects */
    APPEND("_obj.gfm");
    rv = gfmParser_init(playstate.pParser, game.pCtx, stLevelName
            , pos + LEN("_obj.gfm"));
    ASSERT(erv == ERR_OK, erv);

    resetLeveltransition();

    playstate.entityCount = 0;
    while (1) {
        char *type;
        err erv;

        ASSERT(playstate.entityCount < MAX_ENTITIES, ERR_BUFFERTOOSMALL);
        rv = gfmParser_parseNext(playstate.pParser);
        if (rv == GFMRV_PARSER_FINISHED) {
            break;
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        rv = gfmParser_getIngameType(&type, playstate.pParser);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (strcmp(type, "loadzone") == 0) {
            erv = parseLoadzone(playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
        else if (strcmp(type, "invisible_wall") == 0) {
            erv = parseInvisibleWall(playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
        else if (strcmp(type, "swordy_pos") == 0) {
            if (setPlayer) {
                erv = parseSwordy(&playstate.swordy, playstate.pParser);
                ASSERT(erv == ERR_OK, erv);
            }
        }
        else if (strcmp(type, "gunny_pos") == 0) {
            if (setPlayer) {
                erv = parseGunny(&playstate.gunny, playstate.pParser);
                ASSERT(erv == ERR_OK, erv);
            }
        }
        else if (strcmp(type, "walky") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEnemy(pEnt, playstate.pParser, T_EN_WALKY);
            ASSERT(erv == ERR_OK, erv);
            playstate.entityCount++;
        }
        else if (strcmp(type, "g_walky") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEnemy(pEnt, playstate.pParser, T_EN_G_WALKY);
            ASSERT(erv == ERR_OK, erv);
            playstate.entityCount++;
        }
        else if (strcmp(type, "spiky") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEnemy(pEnt, playstate.pParser, T_EN_SPIKY);
            ASSERT(erv == ERR_OK, erv);
            playstate.entityCount++;
        }
    }

    rv = gfmParser_reset(playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = _loadStaticQuadtree();
    ASSERT(erv == ERR_OK, erv);

    erv = resetCameraPosition(&playstate.swordy, &playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmGroup_killAll(fx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    resetTeleporterTarget();

#undef LEN
#undef APPEND_DYN
#undef APPEND_POS
#undef APPEND
    return ERR_OK;
}

/** Setup the playstate so it may start to be executed */
err loadPlaystate() {
    if ((playstate.flags & PF_TEL_LEVEL) == PF_TEL_LEVEL) {
        /* Load the default first level */
        return _loadLevel("map_map0", 1/*setPlayer*/);
    }
    else {
        /* Load the level pointed by the loadzone */
        return _loadLevel(getNextLevelName(), 0/*setPlayer*/);
    }
}

/**
 * Handles colliding the active player with a invisible static dummy (placed at
 * the inactive player's position)
 *
 * @param  [ in]pActive   The active player
 * @param  [ in]pInactive The inactive player
 * @param  [ in]pDummy    The dummy
 */
inline static void _handleAsyncCollision(entityCtx *pActive
        , entityCtx *pInactive, entityCtx *pDummy) {
    int x, y;
    gfmSprite_getDimensions(&x, &y, pInactive->pSelf);
    gfmSprite_setDimensions(pDummy->pSelf, x, y);
    gfmSprite_getPosition(&x, &y, pInactive->pSelf);
    gfmSprite_setPosition(pDummy->pSelf, x, y);
    gfmSprite_update(pDummy->pSelf, game.pCtx);

    collideTwoEntities(pActive, pDummy);
}

/** Update the playstate */
err updatePlaystate() {
    gfmRV rv;
    err erv;
    int i;

    playstate.flags &= ~(PF_TEL_SWORDY | PF_TEL_GUNNY);
    playstate.flags |= PF_TEL_LEVEL;

    rv = gfmQuadtree_initRoot(collision.pQt, -16/*x*/, -16/*y*/, playstate.width
            , playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    if (game.flags & FX_PRETTYRENDER) {
        rv = gfmTilemap_update(playstate.pBackground, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    i = 0;
    while (i < playstate.entityCount) {
        erv = preUpdateEnemy(&playstate.entities[i]);
        ASSERT(erv == ERR_OK, erv);
        i++;
    }

    erv = preUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = preUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /* FX-group should be the last step of pre-update (so it runs after
     * everyting was spawned this frame) */
    erv = updateFxGroup();
    ASSERT(erv == ERR_OK, erv);

    /* Fix the collision between both players, if only one is active */
    if ((game.flags & AC_BOTH) == AC_SWORDY) {
        _handleAsyncCollision(&playstate.swordy, &playstate.gunny
                , &playstate.asyncDummy);
    }
    else if ((game.flags & AC_BOTH) == AC_GUNNY) {
        _handleAsyncCollision(&playstate.gunny, &playstate.swordy
                , &playstate.asyncDummy);
    }

    i = 0;
    while (i < playstate.entityCount) {
        erv = postUpdateEnemy(&playstate.entities[i]);
        ASSERT(erv == ERR_OK, erv);
        i++;
    }


    erv = postUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = postUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /* This may result in a few 1-frame display bugs for the target... */
    erv = updateTeleporterTarget();
    ASSERT(erv == ERR_OK, erv);
    erv = updateGunnyTeleport(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /** Check if a loadzone was triggered by both players */
    if ((playstate.flags & (PF_TEL_SWORDY | PF_TEL_GUNNY))
            == (PF_TEL_SWORDY | PF_TEL_GUNNY)) {
        int curLevel;

        curLevel = _getLevelIndex(playstate.flags);

        /* Setup & trigger level transition/loading */
        switchToLevelTransition(curLevel);
    }

    erv = updateCamera(&playstate.swordy, &playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** Draw the playstate */
err drawPlaystate() {
    gfmRV rv;
    err erv;
    int i;

    if (game.flags & FX_PRETTYRENDER) {
#if 0
        int height, width, x, y;
        int sx, sy, gx, gy;

        /* Retrieve the mean position of the players */
        gfmSprite_getCenter(&sx, &sy, playstate.swordy.pSelf);
        gfmSprite_getCenter(&gx, &gy, playstate.gunny.pSelf);
        x = (sx + gx) / 2;
        y = (sy + gy) / 2;

        gfmTilemap_getDimension(&width, &height, playstate.pBackground);
        width /= 2;
        height /= 2;

        if (x < width) {
            x *= 8;
            x /= width;
            x = 8 - x;
        }
        else if (x > width) {
            x -= width;
            x *= 8;
            x /= width;
            x *= -1;
        }
        else {
            x = 0;
        }

        if (y < height) {
            y *= 8;
            y /= height;
            y = 8 - y;
        }
        else if (y > height) {
            y -= height;
            y *= 8;
            y /= height;
            y *= -1;
        }
        else {
            y = 0;
        }

        gfmTilemap_setPosition(playstate.pBackground, x, y);
#endif
        rv = gfmTilemap_draw(playstate.pBackground, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    i = 0;
    while (i < playstate.entityCount) {
        erv = drawEnemy(&playstate.entities[i]);
        ASSERT(erv == ERR_OK, erv);
        i++;
    }

    erv = drawGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);
    erv = drawSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmGroup_draw(fx, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_draw(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Draw only the players */
err drawPlayers() {
    err erv;
    gameFlags prettyRender;

    /* Disable pretty render to avoid a small graphical glitch */
    prettyRender = game.flags & FX_PRETTYRENDER;
    game.flags &= ~FX_PRETTYRENDER;

    erv = drawGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);
    erv = drawSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);

    game.flags |= prettyRender;

    return ERR_OK;
}

