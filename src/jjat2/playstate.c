/**
 * @file src/jjat2/playstate.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/game.h>

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmTilemap.h>

#include <jjat2/camera.h>
#include <jjat2/checkpoint.h>
#include <jjat2/dictionary.h>
#include <jjat2/enemy.h>
#include <jjat2/event.h>
#include <jjat2/fx_group.h>
#include <jjat2/gunny.h>
#include <jjat2/hitbox.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/swordy.h>
#include <jjat2/teleport.h>
#include <jjat2/ui.h>

#include <string.h>

#if defined(JJAT_ENABLE_BACKGROUND)
static int pBgAnimData[] = {
/*len|fps|loop|data... */
   2 , 2 ,  0 , 926,927,
   2 , 2 ,  0 , 990,991,
   6 , 8 ,  0 , 927,928,929,930,931,926,
   6 , 8 ,  0 , 991,992,993,994,995,990,
};
static int bgAnimDataLen = sizeof(pBgAnimData) / sizeof(int);
#endif /* JJAT_ENABLE_BACKGROUND */

/** Region where the names of maps accessible from the current one are stored */
static char _stMapsName[MAX_AREAS * (MAX_VALID_LEN + 1)];

enum enLevelInfoFlags {
    LIF_NAME = 0x01
  , LIF_TGTX = 0x02
  , LIF_TGTY = 0x04
  , LIF_DIR  = 0x08
};
typedef enum enLevelInfoFlags levelInfoFlags;

#define swordy_icon 106
#define gunny_icon  107

enum {
    PF_TEL_SWORDY = 0x01
  , PF_TEL_GUNNY  = 0x02
};

/** Distance between the actual tile and its inactive counter-part */
#define UNACTIVE_TILE_OFFSET 6

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
#if defined(JJAT_ENABLE_BACKGROUND)
    rv = gfmTilemap_getNew(&playstate.pBackground);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(playstate.pBackground, gfx.pSset8x8, TM_MAX_WIDTH
            , TM_MAX_HEIGHT, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    /* Load the animations into the BG */
    rv = gfmTilemap_addAnimations(playstate.pBackground, pBgAnimData
            , bgAnimDataLen);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#endif /* JJAT_ENABLE_BACKGROUND */

    i = 0;
    while (i < MAX_ENTITIES) {
        rv = gfmSprite_getNew(&playstate.entities[i].pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        i++;
    }

    /* Alloc the areas list and prepare every data */
    rv = gfmHitbox_getNewList(&playstate.pAreas, MAX_AREAS);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* TODO Load session flags */
    game.sessionFlags = 0;

    return ERR_OK;
}

/** If the playstate has been initialized, properly free it up. */
void freePlaystate() {
    int i;

    if (playstate.pAreas != 0) {
        gfmHitbox_free(&playstate.pAreas);
    }
    if (playstate.pMap != 0) {
        gfmTilemap_free(&playstate.pMap);
    }
#if defined(JJAT_ENABLE_BACKGROUND)
    if (playstate.pBackground != 0) {
        gfmTilemap_free(&playstate.pBackground);
    }
#endif /* JJAT_ENABLE_BACKGROUND */
    if (playstate.pParser != 0) {
        gfmParser_free(&playstate.pParser);
    }
    freeSwordy(&playstate.swordy);
    freeGunny(&playstate.gunny);

    i = 0;
    while (i < MAX_ENTITIES) {
        gfmSprite_free(&playstate.entities[i].pSelf);
        i++;
    }
}

/**
 * Setup pointers within a hitboxCtx's level transition data.
 *
 * @param  [ in]i Context's index
 */
static void _setupLevelTransitionData(int i) {
    playstate.data[i].ltData.pName = _stMapsName + (MAX_VALID_LEN + 1) * i;
}

/**
 * Setup loading the next map.
 *
 * @param  [ in]type Type of entity that entered the loadzone
 * @param  [ in]pData Data of the loadzone that was hit
 */
void onHitLoadzone(int type, leveltransitionData *pData) {
    /* Check if the triggered level is the same as the stored one (or if no
     * level has been set yet) */
    if (playstate.pNextLevel != 0 && pData != playstate.pNextLevel) {
        return;
    }

    if (type == T_SWORDY || type == T_DUMMY_SWORDY) {
        playstate.flags |= PF_TEL_SWORDY;
    }
    else if (type == T_GUNNY || type == T_DUMMY_GUNNY) {
        playstate.flags |= PF_TEL_GUNNY;
    }

    playstate.pNextLevel = pData;
}

/** Updates the quadtree's bounds according to the currently loaded map */
static err _updateWorldSize() {
    gfmRV rv;
    int height, width;

    rv = gfmTilemap_getDimension(&width, &height, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmCamera_setWorldDimensions(game.pCamera, width, height);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /** Make the quadtree 4 tiles larger than the actual map */
    playstate.width = width + 32;
    playstate.height = height + 32;

    return ERR_OK;
}

/** Check the tilemap for any tile that has to have its graphics modified
 * (because of session flags) */
static err _updateActivableTiles() {
    int *pData;
    gfmRV rv;
    int i, height, width;

    /* TODO Add other flags */
    if (game.sessionFlags & (SF_BLUE_ACTIVE)) {
        /* If all switches were activated, no need to modify the tilemap */
        return ERR_OK;
    }

    rv = gfmTilemap_getData(&pData, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_getDimension(&width, &height, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* TODO Optmize this, if necessary */
    i = 0;
    while (i < height * width / TILE_DIMENSION / TILE_DIMENSION) {
        switch (pData[i]) {
            /* Blue tiles */
            case 1344:
            case 1345:
            case 1346:
            case 1347:
            case 1348:
            case 1349:
            case 1408:
            case 1409:
            case 1410:
            case 1411:
            case 1472:
            case 1473:
            case 1474: {
                if (!(game.sessionFlags & SF_BLUE_ACTIVE)) {
                    pData[i] += UNACTIVE_TILE_OFFSET;
                }
            } break;
            default: { /* Do nothing */ }
        }
        i++;
    }

    return ERR_OK;
}

/** Load the static quadtree */
static err _loadStaticQuadtree() {
    gfmRV rv;

    rv = gfmQuadtree_initRoot(collision.pStaticQt, -16/*x*/, -16/*y*/
            , playstate.width, playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_setStatic(collision.pStaticQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmQuadtree_enableContinuosCollision(collision.pStaticQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmQuadtree_populateTilemap(collision.pStaticQt, playstate.pMap);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmHitbox_populateQuadtree(playstate.pAreas, collision.pStaticQt
            , playstate.areasCount);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Retrieve an integer from a string
 *
 * @param  [ in]pStr The string
 * @return           The integer
 */
static int _getInt(char *pStr) {
    int ret = 0;
    while (*pStr) {
        ret = ret * 10 + (*pStr - '0');
        pStr++;
    }
    return ret;
}


/**
 * Load information used to load the level from the parser
 *
 * @param  [ in]pParser The parser
 * @param  [ in]pInfo   The parsed information
 */
static err _parseLevelInfo(gfmParser *pParser, leveltransitionData *pInfo
        , levelInfoFlags required) {
    gfmRV rv;
    int i, l;

    rv = gfmParser_getNumProperties(&l, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    do {
        int x, y;

        rv = gfmParser_getPos(&x, &y, pParser);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (x > 0) {
            pInfo->srcX = (uint16_t)x;
        }
        else {
            pInfo->srcX = 0;
        }
        if (y > 0) {
            pInfo->srcY = (uint16_t)y;
        }
        else {
            pInfo->srcY = 0;
        }
    } while (0);

    pInfo->dir = -1;
    pInfo->tgtX = 0xffff;
    pInfo->tgtY = 0xffff;
    pInfo->pName[0] = '\0';

    i = 0;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (strcmp(pKey, "tgt_x") == 0) {
            int val;

            val = _getInt(pVal);
            ASSERT(val >= 0 && val < 0x2000, ERR_PARSINGERR);
            pInfo->tgtX = (uint16_t)val * 8;
        }
        else if (strcmp(pKey, "tgt_y") == 0) {
            int val;

            val = _getInt(pVal);
            ASSERT(val >= 0 && val < 0x2000, ERR_PARSINGERR);
            pInfo->tgtY = (uint16_t)val * 8;
        }
        else if (strcmp(pKey, "dest") == 0) {
            ASSERT(strlen(pVal) < MAX_VALID_LEN, ERR_PARSINGERR);
            strcpy(pInfo->pName, pVal);
        }
        else if (strcmp(pKey, "dir") == 0) {
            if (strcmp(pVal, "left") == 0) {
                pInfo->dir = TEL_LEFT;
            }
            else if (strcmp(pVal, "right") == 0) {
                pInfo->dir = TEL_RIGHT;
            }
            else if (strcmp(pVal, "up") == 0) {
                pInfo->dir = TEL_UP;
            }
            else if (strcmp(pVal, "down") == 0) {
                pInfo->dir = TEL_DOWN;
            }
            else {
                ASSERT(0, ERR_PARSINGERR);
            }
        }

        i++;
    }

    ASSERT(!(required & LIF_DIR) || pInfo->dir != -1, ERR_PARSINGERR);
    ASSERT(!(required & LIF_TGTX) || pInfo->tgtX != 0xffff, ERR_PARSINGERR);
    ASSERT(!(required & LIF_TGTY) || pInfo->tgtY != 0xffff, ERR_PARSINGERR);
    ASSERT(!(required & LIF_NAME) || pInfo->pName[0] != '\0', ERR_PARSINGERR);
            

    return ERR_OK;
}

/**
 * Parse a loadzone
 *
 * @param  [ in]pParser The parser pointing at a loadzone
 */
static err _parseLoadzone(gfmParser *pParser) {
    leveltransitionData *pData;
    gfmRV rv;
    err erv;
    int h, w, x, y;

    ASSERT(playstate.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    _setupLevelTransitionData(playstate.areasCount);
    pData = &(playstate.data[playstate.areasCount].ltData);
    erv = _parseLevelInfo(pParser, pData
            , (LIF_NAME | LIF_TGTX | LIF_TGTY | LIF_DIR));
    ASSERT(erv == ERR_OK, erv);

    rv = gfmHitbox_initItem(playstate.pAreas, pData, x, y, w, h, T_LOADZONE
            , playstate.areasCount);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    playstate.areasCount++;
    return ERR_OK;
}

/**
 * Parse a invisible wall
 *
 * @param  [ in]pParser The parser pointing at a invisible wall
 */
static err _parseInvisibleWall(gfmParser *pParser) {
    gfmRV rv;
    int h, w, x, y;

    ASSERT(playstate.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmHitbox_initItem(playstate.pAreas, 0/*ctx*/, x, y, w, h, T_FLOOR_NOTP
            , playstate.areasCount);

    playstate.areasCount++;
    return ERR_OK;
}

/**
 * Parse a checkpoint
 *
 * @param  [ in]pParser    The parser pointing at a checkpoint
 * @param  [ in]pLevelName Name of the current level
 */
static err _parseCheckpoint(gfmParser *pParser, const char *pLevelName) {
    leveltransitionData *pData;
    gfmRV rv;
    int h, w, x, y;

    ASSERT(playstate.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    _setupLevelTransitionData(playstate.areasCount);
    pData = &(playstate.data[playstate.areasCount].ltData);
    /* pLevelName should be the name of the loaded file. Therefore, it must have
     * a valid length and terminator (unless it got corrupted...) */
    strcpy(pData->pName, pLevelName);
    /* Set the target position based on the checkpoint's position */
    pData->tgtX = (uint16_t)(x + w / 2);
    pData->tgtY = (uint16_t)(y + h - TILES_TO_PX(2));

    rv = gfmHitbox_initItem(playstate.pAreas, pData, x, y, w, h, T_CHECKPOINT
            , playstate.areasCount);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    playstate.areasCount++;
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
 *                        argument. Note that this parameter may come from a
 *                        previous loaded loadzone. Therefore, as soon as
 *                        objects start to get parsed, this may be overwritten
 * @param  [ in]setPlayer Whether the player position should be set from the map
 */
static err _loadLevel(char *levelName, int setPlayer) {
    char *pValidName;
    char stLevelName[MAX_LEVEL_NAME];
    int nameLen, pos;
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
    nameLen = strlen(levelName);
    ASSERT(nameLen <= MAX_LEVEL_NAME, ERR_INVALIDLEVELNAME);

    pos = 0;
    APPEND_POS("levels/");
    /* Store this position as the one with a valid (i.e., immutable name) */
    pValidName = stLevelName + pos;
    /* TODO Check if levelName is actually a valid path */
    APPEND_DYN(levelName, nameLen);

    /* Load the tilemap */
    APPEND("_fg_tm.gfm");
    rv = gfmTilemap_loadf(playstate.pMap, game.pCtx, stLevelName
            , pos + LEN("_fg_tm.gfm"), pDictNames, pDictTypes, dictLen);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = _updateWorldSize();
    ASSERT(erv == ERR_OK, erv);

    erv = _updateActivableTiles();
    ASSERT(erv == ERR_OK, erv);

#if defined(JJAT_ENABLE_BACKGROUND)
    if (game.flags & FX_PRETTYRENDER) {
        APPEND("_bg_tm.gfm");
        rv = gfmTilemap_loadf(playstate.pBackground, game.pCtx, stLevelName
                , pos + LEN("_bg_tm.gfm"), pDictNames, pDictTypes, dictLen);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
#endif /* JJAT_ENABLE_BACKGROUND */

    /* Load the objects */
    APPEND("_obj.gfm");
    rv = gfmParser_init(playstate.pParser, game.pCtx, stLevelName
            , pos + LEN("_obj.gfm"));
    ASSERT(erv == ERR_OK, erv);

    /* Since no other filed is opened after this, clamp the level name */
    pValidName[nameLen] = '\0';

    playstate.entityCount = 0;
    playstate.areasCount = 0;
    resetHitboxes();
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
            erv = _parseLoadzone(playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
        else if (strcmp(type, "invisible_wall") == 0) {
            erv = _parseInvisibleWall(playstate.pParser);
            ASSERT(erv == ERR_OK, erv);
        }
        else if (strcmp(type, "checkpoint") == 0) {
            erv = _parseCheckpoint(playstate.pParser, pValidName);
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
        else if (strcmp(type, "turret") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEnemy(pEnt, playstate.pParser, T_EN_TURRET);
            ASSERT(erv == ERR_OK, erv);
            playstate.entityCount++;
        }
        else if (strcmp(type, "door") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEvent(pEnt, playstate.pParser, T_DOOR);
            ASSERT(erv == ERR_OK, erv);
            playstate.entityCount++;
        }
        else if (strcmp(type, "pressure_pad") == 0) {
            entityCtx *pEnt = &playstate.entities[playstate.entityCount];
            erv = parseEvent(pEnt, playstate.pParser, T_PRESSURE_PAD);
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

    setMapTitle(pValidName);
    showUI();

    if (setPlayer) {
        leveltransitionData data;
        int h, tgtX, tgtY, w;

        rv = gfmSprite_getPosition(&tgtX, &tgtY, playstate.swordy.pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmSprite_getDimensions(&w, &h, playstate.swordy.pSelf);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Retrieve the position of the tile beneath the players.
         * The remainder of a division by 8 is always the 3 least significant
         * bits, so get those directly. */
        tgtX -= w;
        tgtX -= tgtX & 7;
        tgtY += h;
        tgtY -= tgtY & 7;

        /* Setting the player position implies that this is the first level on
         * this playthrough (either a new game or a loaded one). Therefore,
         * setup the checkpoint */
        data.pName = pValidName;
        data.tgtX = (uint16_t)tgtX;
        data.tgtY = (uint16_t)tgtY;
        erv = setCheckpoint(&data);
        ASSERT(erv == ERR_OK, erv);
    }

#undef LEN
#undef APPEND_DYN
#undef APPEND_POS
#undef APPEND
    return ERR_OK;
}

/** Setup the playstate so it may start to be executed */
err loadPlaystate() {
    if (playstate.pNextLevel == 0) {
        /* Load the default first level */
        return _loadLevel(FIRST_MAP, 1/*setPlayer*/);
    }
    else {
        /* Load the level pointed by the loadzone */
        return _loadLevel(playstate.pNextLevel->pName, 0/*setPlayer*/);
    }
}

/** Remove the flag that signals that no level is being loaded */
void clearPlaystateLevelFlag() {
    playstate.pNextLevel = 0;
}

/**
 * Handles colliding the active player with a invisible static dummy (placed at
 * the inactive player's position)
 *
 * @param  [ in]pActive   The active player
 * @param  [ in]pInactive The inactive player
 */
inline static void _handleAsyncCollision(entityCtx *pActive
        , entityCtx *pInactive) {
    void *pVoid;
    int type, h, w, x, y;
    gfmSprite_getDimensions(&w, &h, pInactive->pSelf);
    gfmSprite_getPosition(&x, &y, pInactive->pSelf);
    gfmSprite_getChild(&pVoid, &type, pInactive->pSelf);
    if ((type & T_SWORDY) == T_SWORDY) {
        type = T_DUMMY_SWORDY;
    }
    else if ((type & T_GUNNY) == T_GUNNY) {
        type = T_DUMMY_GUNNY;
    }
    spawnTmpHitbox(0/*pCtx*/, x, y, w, h, type);
}

/** Update the playstate */
err updatePlaystate() {
    gfmRV rv;
    err erv;
    int i;

    /* Local variables are cleared on the start of every frame, since they shall
     * be later set on event's pre-update */
    clearLocalVariables();

    playstate.flags &= ~(PF_TEL_SWORDY | PF_TEL_GUNNY);
    playstate.pNextLevel = 0;
    resetTmpHitboxes();

    rv = gfmQuadtree_initRoot(collision.pQt, -16/*x*/, -16/*y*/, playstate.width
            , playstate.height, 8/*depth*/, 16/*nodes*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmQuadtree_enableContinuosCollision(collision.pQt);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmTilemap_update(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#if defined(JJAT_ENABLE_BACKGROUND)
    if (game.flags & FX_PRETTYRENDER) {
        rv = gfmTilemap_update(playstate.pBackground, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
#endif /* JJAT_ENABLE_BACKGROUND */

    i = 0;
    while (i < playstate.entityCount) {
        switch (playstate.entities[i].baseType) {
            case T_ENEMY: erv = preUpdateEnemy(&playstate.entities[i]); break;
            case T_EVENT: erv = preUpdateEvent(&playstate.entities[i]); break;
        }
        ASSERT(erv == ERR_OK, erv);
        i++;
    }

    erv = preUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = preUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /* Fix the collision between both players, if only one is active */
    if ((game.flags & AC_BOTH) == AC_BOTH) {
        /* Ignore this on synchronous mode */
    }
    else if ((game.flags & AC_BOTH) == AC_SWORDY) {
        _handleAsyncCollision(&playstate.swordy, &playstate.gunny);
    }
    else if ((game.flags & AC_BOTH) == AC_GUNNY) {
        _handleAsyncCollision(&playstate.gunny, &playstate.swordy);
    }

    /* FX-group should be the last step of pre-update (so it runs after
     * everyting was spawned this frame) */
    erv = updateFxGroup();
    ASSERT(erv == ERR_OK, erv);

    erv = collideHitbox();
    ASSERT(erv == ERR_OK, erv);

    i = 0;
    while (i < playstate.entityCount) {
        switch (playstate.entities[i].baseType) {
            case T_ENEMY: erv = postUpdateEnemy(&playstate.entities[i]); break;
            case T_EVENT: erv = postUpdateEvent(&playstate.entities[i]); break;
        }
        ASSERT(erv == ERR_OK, erv);
        i++;
    }

    erv = postUpdateSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);
    erv = postUpdateGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /** Check if a loadzone was triggered by both players */
    if ((playstate.flags & (PF_TEL_SWORDY | PF_TEL_GUNNY))
            == (PF_TEL_SWORDY | PF_TEL_GUNNY)) {
        /* Setup & trigger level transition/loading */
        switchToLevelTransition(playstate.pNextLevel);
    }

    /* This may result in a few 1-frame display bugs for the target... */
    erv = updateTeleporterTarget();
    ASSERT(erv == ERR_OK, erv);
    erv = updateGunnyTeleport(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    erv = updateUI();
    ASSERT(erv == ERR_OK, erv);

    erv = updateCamera(&playstate.swordy, &playstate.gunny);
    ASSERT(erv == ERR_OK, erv);

    /* Reload the checkpoint */
    if (!(playstate.swordy.flags & EF_ALIVE)
            || !(playstate.gunny.flags & EF_ALIVE)) {
        erv = loadCheckpoint();
        ASSERT(erv == ERR_OK, erv);

        playstate.swordy.flags |= EF_ALIVE;
        playstate.gunny.flags |= EF_ALIVE;
    }

    return ERR_OK;
}

/** Draw the playstate */
err drawPlaystate() {
    gfmRV rv;
    err erv;
    int i, nodes, buckets;

#if defined(JJAT_ENABLE_BACKGROUND)
    if (game.flags & FX_PRETTYRENDER) {
        int height, width, x, y;
        int sx, sy, gx, gy;

        /* Retrieve the mean position of the players */
        gfmSprite_getCenter(&sx, &sy, playstate.swordy.pSelf);
        gfmSprite_getCenter(&gx, &gy, playstate.gunny.pSelf);
        x = (sx + gx) / 2 + 8;
        y = (sy + gy) / 2 + 8;

        gfmTilemap_getDimension(&width, &height, playstate.pBackground);

        if (width > playstate.width - 32) {
            width /= 2;
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
            x -= 8;
        }
        else {
            x = 0;
        }

        if (height > playstate.height - 32) {
            height /= 2;
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
            y -= 8;
        }
        else {
            y = 0;
        }

        gfmTilemap_setPosition(playstate.pBackground, x, y);

        rv = gfmTilemap_draw(playstate.pBackground, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
#endif /* JJAT_ENABLE_BACKGROUND */

    i = 0;
    while (i < playstate.entityCount) {
        switch (playstate.entities[i].baseType) {
            case T_ENEMY: erv = drawEnemy(&playstate.entities[i]); break;
            case T_EVENT: erv = drawEvent(&playstate.entities[i]); break;
        }
        ASSERT(erv == ERR_OK, erv);
        i++;
    }

    erv = drawGunny(&playstate.gunny);
    ASSERT(erv == ERR_OK, erv);
    erv = drawSwordy(&playstate.swordy);
    ASSERT(erv == ERR_OK, erv);

    rv = gfmTilemap_draw(playstate.pMap, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmGroup_draw(fx, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    erv = drawEntityIcon(&playstate.swordy, swordy_icon);
    ASSERT(erv == ERR_OK, erv);
    erv = drawEntityIcon(&playstate.gunny, gunny_icon);
    ASSERT(erv == ERR_OK, erv);

    erv = drawUI();
    ASSERT(erv == ERR_OK, erv);

    gfmQuadtree_getNumNodes(&nodes, &buckets, collision.pQt);
    gfmDebug_printf(game.pCtx, 0, 128, "DYNAMIC\nNODES  : %i\nBUCKETS: %i", nodes, buckets);
    gfmQuadtree_getNumNodes(&nodes, &buckets, collision.pStaticQt);
    gfmDebug_printf(game.pCtx, 0, 128+8*4, "STATIC\nNODES  : %i\nBUCKETS: %i", nodes, buckets);

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

