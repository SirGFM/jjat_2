/**
 * @file src/jjat2/leveltransition.c
 *
 * Display and hide the transition screen and tween the player to its new
 * position.
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/game.h>
#include <conf/type.h>

#include <jjat2/gunny.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/swordy.h>

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTilemap.h>

#include <string.h>

#define TRANSITION_TIME 750
#define WIDTH_IN_TILES  (V_WIDTH / 8 + 4)
#define HEIGHT_IN_TILES (V_HEIGHT / 8 + 4)
#define TM_DEFAULT_TILE -1

enum enLevelInfoFlags {
    LIF_NAME = 0x01
  , LIF_TGTX = 0x02
  , LIF_TGTY = 0x04
  , LIF_DIR  = 0x08
};
typedef enum enLevelInfoFlags levelInfoFlags;

/** Parseable information used when loading levels */
struct stLevelInfo {
    char *pName;
    int tgtX;
    int tgtY;
    int dir;
    levelInfoFlags required;
};
typedef struct stLevelInfo levelInfo;

/** Region where the name of maps accessible from the current one is stored */
static char _stMapsName[MAX_AREAS * (MAX_VALID_LEN + 1)];

/** Forward declaration of the transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES];

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

#define TWEEN(var_op, init_time) \
    do { \
        var_op ( ((TRANSITION_TIME - (lvltransition.timer - (init_time))) \
                * (START_POS)) / TRANSITION_TIME) \
            + ( ((lvltransition.timer - (init_time)) * (END_POS)) \
                / TRANSITION_TIME); \
    } while (0)

/**
 * Tween the tilemap INTO the screen
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 */
static void _tweenTilemapIn(int cx, int cy) {
    switch ((lvltransition.dir << TEL_DIR_BITS) & TEL_DIR_MASK) {
        case TEL_UP: {
            cx -= 16;
#define START_POS (V_HEIGHT)
#define END_POS   (-16)
            TWEEN(cy +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_DOWN: {
            cx -= 16;
#define START_POS (-HEIGHT_IN_TILES * 8)
#define END_POS   (-16)
            TWEEN(cy +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_LEFT: {
            cy -= 16;
#define START_POS (V_WIDTH)
#define END_POS   (-16)
            TWEEN(cx +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_RIGHT: {
            cy -= 16;
#define START_POS (-WIDTH_IN_TILES * 8)
#define END_POS   (-16)
            TWEEN(cx +=, 0);
#undef START_POS
#undef END_POS
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

/**
 * Tween a sprite in screen space toward the next position (coverted to screen
 * space)
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 * @param  [ in]initTime Time when the tween started
 */
static void _tweenPlayers(int cx, int cy, int initTime) {
    int dstX, dstY, srcX, srcY, tgtX, tgtY;

    /* Adjust the target position to be within the current camera */
    tgtX = (lvltransition.cachedTargetPosition & 0xffff) + cx;
    tgtY = ((lvltransition.cachedTargetPosition >> 16) & 0xffff) + cy;

    srcX = lvltransition.swordyPos & 0xffff;
    srcY = (lvltransition.swordyPos >> 16) & 0xffff;
#define START_POS (srcX)
#define END_POS   (tgtX)
            TWEEN(dstX =, initTime);
#undef START_POS
#undef END_POS
#define START_POS (srcY)
#define END_POS   (tgtY)
            TWEEN(dstY =, initTime);
#undef START_POS
#undef END_POS
    setSwordyPositionFromParser(&playstate.swordy, dstX, dstY);

    srcX = lvltransition.gunnyPos & 0xffff;
    srcY = (lvltransition.gunnyPos >> 16) & 0xffff;
#define START_POS (srcX)
#define END_POS   (tgtX)
            TWEEN(dstX =, initTime);
#undef START_POS
#undef END_POS
#define START_POS (srcY)
#define END_POS   (tgtY)
            TWEEN(dstY =, initTime);
#undef START_POS
#undef END_POS
    setGunnyPositionFromParser(&playstate.gunny, dstX, dstY);
}

/** Set both sprites position at the next position in world space */
static void _setPlayersPosition() {
    int tgtX, tgtY;

    /* Adjust the target position to be within the current camera */
    tgtX = lvltransition.cachedTargetPosition & 0xffff;
    tgtY = (lvltransition.cachedTargetPosition >> 16) & 0xffff;

    setSwordyPositionFromParser(&playstate.swordy, tgtX, tgtY);
    setGunnyPositionFromParser(&playstate.gunny, tgtX, tgtY);
}

/**
 * Tween the tilemap OUTSIDE the screen
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 */
static void _tweenTilemapOut(int cx, int cy) {
    switch ((lvltransition.dir << TEL_DIR_BITS) & TEL_DIR_MASK) {
        case TEL_UP: {
            cx -= 16;
#define START_POS (-16)
#define END_POS   (-HEIGHT_IN_TILES * 8)
            TWEEN(cy +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_DOWN: {
            cx -= 16;
#define START_POS (-16)
#define END_POS   (V_HEIGHT)
            TWEEN(cy +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_LEFT: {
            cy -= 16;
#define START_POS (-16)
#define END_POS   (-WIDTH_IN_TILES * 8)
            TWEEN(cx +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_RIGHT: {
            cy -= 16;
#define START_POS (-16)
#define END_POS   (V_WIDTH)
            TWEEN(cx +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

#undef TWEEN

/**
 * Retrieves the index of a level given its type
 *
 * @param  [ in]levelType The type of the loadzone object
 */
int getLoadzoneIndex(int levelType) {
    return (levelType & TEL_INDEX_MASK) >> TEL_INDEX_BITS;
}

/**
 * Prepare switching to a level transition
 *
 * @param  [ in]index Index of the loadzone
 */
void switchToLevelTransition(int index) {
    lvltransition.geometry.index = (uint8_t)index;
    game.nextState = ST_LEVELTRANSITION;
}

/** Retrieve the name of the level to be loaded */
char* getNextLevelName() {
    return lvltransition.pCachedName;
}

/**
 * Retrieve the transition data for a given level
 *
 * @param  [out]ppName Name of the target level
 * @param  [out]pTgtX  Target position within the level (in pixels)
 * @param  [out]pTgtY  Target position within the level (in pixels)
 * @param  [ in]index  Level's index
 */
err getLevelTransitionData(char **ppName, int *pTgtX, int *pTgtY, int index) {
    int pos;

    ASSERT(index < lvltransition.geometry.areasCount, ERR_INDEXOOB);

    pos = lvltransition.geometry.teleportPosition[index];

    *ppName = lvltransition.geometry.pNames[index];
    *pTgtX = pos & 0xffff;
    *pTgtY = (pos >> 16) & 0xffff;

    return ERR_OK;
}

/** Alloc all required resources */
err initLeveltransition() {
    gfmRV rv;
    int i;

    rv = gfmTilemap_getNew(&lvltransition.pTransition);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(lvltransition.pTransition, gfx.pSset8x8, WIDTH_IN_TILES
            , HEIGHT_IN_TILES, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_load(lvltransition.pTransition, _tilemap
            , WIDTH_IN_TILES * HEIGHT_IN_TILES, WIDTH_IN_TILES
            , HEIGHT_IN_TILES);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    i = 0;
    while (i < MAX_AREAS) {
        rv = gfmObject_getNew(&lvltransition.geometry.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        lvltransition.geometry.pNames[i] = _stMapsName
                + (MAX_VALID_LEN + 1) * i;
        i++;
    }

    return ERR_OK;
}

/** Releases all previously alloc'ed resources */
void freeLeveltransition() {
    int i;

    if (lvltransition.pTransition != 0) {
        gfmTilemap_free(&lvltransition.pTransition);
    }

    i = 0;
    while (i < MAX_AREAS) {
        gfmObject_free(&lvltransition.geometry.pAreas[i]);
        i++;
    }
}

/** Reset all objects parsed on the previous level */
void resetLeveltransition() {
    lvltransition.geometry.areasCount = 0;
}

/** Setup static collision against all level transition objects */
err calculateStaticLeveltransition() {
    gfmRV rv;
    int i;

    i = 0;
    while (i < lvltransition.geometry.areasCount) {
        rv = gfmObject_setFixed(lvltransition.geometry.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmObject_update(lvltransition.geometry.pAreas[i], game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmQuadtree_populateObject(collision.pStaticQt
                , lvltransition.geometry.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        i++;
    }

    return ERR_OK;
}

/**
 * Load information used to load the level from the parser
 *
 * @param  [ in]pParser The parser
 * @param  [ in]pInfo   The parsed information
 */
static err parseLevelInfo(gfmParser *pParser, levelInfo *pInfo) {
    gfmRV rv;
    int i, l;

    rv = gfmParser_getNumProperties(&l, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    pInfo->dir = -1;
    pInfo->tgtX = -1;
    pInfo->tgtY = -1;
    pInfo->pName = 0;

    i = 0;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, playstate.pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (strcmp(pKey, "tgt_x") == 0) {
            pInfo->tgtX = _getInt(pVal);
        }
        else if (strcmp(pKey, "tgt_y") == 0) {
            pInfo->tgtY = _getInt(pVal);
        }
        else if (strcmp(pKey, "dest") == 0) {
            ASSERT(strlen(pVal) < MAX_VALID_LEN, ERR_PARSINGERR);
            pInfo->pName = pVal;
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

    ASSERT(!(pInfo->required & LIF_DIR) || pInfo->dir != -1, ERR_PARSINGERR);
    ASSERT(!(pInfo->required & LIF_TGTX)
            || (pInfo->tgtX > -1 && pInfo->tgtX < 0x00200), ERR_PARSINGERR);
    ASSERT(!(pInfo->required & LIF_TGTY)
            || (pInfo->tgtY > -1 && pInfo->tgtY < 0x00200), ERR_PARSINGERR);
    ASSERT(!(pInfo->required & LIF_NAME) || pInfo->pName != 0, ERR_PARSINGERR);

    return ERR_OK;
}

/**
 * Parse a loadzone
 *
 * @param  [ in]pParser The parser pointing at a loadzone
 */
err parseLoadzone(gfmParser *pParser) {
    levelInfo info;
    gfmRV rv;
    err erv;
    int count, h, type, w, x, y;

    count = lvltransition.geometry.areasCount;
    ASSERT(count < MAX_AREAS, ERR_BUFFERTOOSMALL);

    rv = gfmParser_getPos(&x, &y, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    info.required = (LIF_NAME | LIF_TGTX | LIF_TGTY | LIF_DIR);
    erv = parseLevelInfo(pParser, &info);
    ASSERT(erv == ERR_OK, erv);

    strcpy(lvltransition.geometry.pNames[count], info.pName);
    lvltransition.geometry.teleportPosition[count] = ((info.tgtY * 8) << 16)
            | (info.tgtX * 8);

    type = T_LOADZONE;
    type |= TEL_INDEX_MASK & (count << TEL_INDEX_BITS);
    type |= info.dir;

    rv = gfmObject_init(lvltransition.geometry.pAreas[count], x, y, w, h
            , 0/*child*/, type);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.geometry.areasCount++;
    return ERR_OK;
}

/**
 * Parse a invisible wall
 *
 * @param  [ in]pParser The parser pointing at a invisible wall
 */
err parseInvisibleWall(gfmParser *pParser) {
    gfmObject *pObj;
    gfmRV rv;
    int h, w, x, y;

    ASSERT(lvltransition.geometry.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    pObj = lvltransition.geometry.pAreas[lvltransition.geometry.areasCount];

    rv = gfmParser_getPos(&x, &y, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmObject_init(pObj, x, y, w, h, 0/*child*/, T_FLOOR_NOTP);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.geometry.areasCount++;
    return ERR_OK;
}

/**
 * Parse a checkpoint
 *
 * @param  [ in]pParser The parser pointing at a checkpoint
 */
err parseCheckpoint(gfmParser *pParser) {
    gfmObject *pObj;
    levelInfo info;
    gfmRV rv;
    err erv;
    int count, h, type, w, x, y;

    count = lvltransition.geometry.areasCount;
    ASSERT(count < MAX_AREAS, ERR_BUFFERTOOSMALL);

    pObj = lvltransition.geometry.pAreas[lvltransition.geometry.areasCount];

    rv = gfmParser_getPos(&x, &y, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    info.required = (LIF_NAME | LIF_TGTX | LIF_TGTY);
    erv = parseLevelInfo(pParser, &info);
    ASSERT(erv == ERR_OK, erv);

    strcpy(lvltransition.geometry.pNames[count], info.pName);
    lvltransition.geometry.teleportPosition[count] = ((info.tgtY * 8) << 16)
            | (info.tgtX * 8);

    type = T_CHECKPOINT;
    type |= TEL_INDEX_MASK & (count << TEL_INDEX_BITS);
    rv = gfmObject_init(pObj, x, y, w, h, 0/*child*/, type);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.geometry.areasCount++;
    return ERR_OK;
}

/**
 * Prepare level transition into a generic level
 *
 * @param  [ in]levelName Level's name
 * @param  [ in]tgtX      Starting position of players
 * @param  [ in]tgtY      Starting position of players
 * @param  [ in]dir       Movement direction of the transition overlay
 */
err setupGenericLeveltransition(char *levelName, int tgtX, int tgtY
        , levelTransitionFlags dir) {
    gfmRV rv;
    int x, y;

    ASSERT(tgtX <= 0xffff, ERR_ARGUMENTBAD);
    ASSERT(tgtY <= 0xffff, ERR_ARGUMENTBAD);

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    switch (dir) {
        case TEL_UP: {
            x -= 16;
            y += V_HEIGHT;
        } break;
        case TEL_DOWN: {
            x -= 16;
            y -= HEIGHT_IN_TILES * 8;
        } break;
        case TEL_LEFT: {
            x += V_WIDTH;
            y -= 16;
        } break;
        case TEL_RIGHT: {
            x -= WIDTH_IN_TILES * 8;
            y -= 16;
        } break;
        default: {
            ASSERT(0, ERR_ARGUMENTBAD);
        }
    }

    rv = gfmTilemap_setPosition(lvltransition.pTransition, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    gfmSprite_getPosition(&x, &y, playstate.swordy.pSelf);
    lvltransition.swordyPos = ((y & 0xffff) << 16) | (x & 0xffff);
    gfmSprite_getPosition(&x, &y, playstate.gunny.pSelf);
    lvltransition.gunnyPos = ((y & 0xffff) << 16) | (x & 0xffff);

    lvltransition.pCachedName = levelName;
    lvltransition.dir = 0xff & (dir >> TEL_DIR_BITS);
    lvltransition.cachedTargetPosition = (tgtY << 16) | tgtX;
    lvltransition.flags = 0;
    lvltransition.timer = 0;

    return ERR_OK;
}

/** Prepare the transition animation */
err setupLeveltransition() {
    void *pChild;
    char *pName;
    gfmRV rv;
    int index, type, tgtX, tgtY;
    levelTransitionFlags dir;

    /* Skip setup as it has already been done */
    if (lvltransition.flags & LT_CHECKPOINT) {
        lvltransition.flags = 0;
        clearPlaystateLevelFlag();
        return ERR_OK;
    }

    index = lvltransition.geometry.index;
    ASSERT(index < lvltransition.geometry.areasCount, ERR_INDEXOOB);

    rv = gfmObject_getChild(&pChild, &type
            , lvltransition.geometry.pAreas[index]);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    pName = lvltransition.geometry.pNames[index];
    dir = type & TEL_DIR_MASK;
    tgtX = lvltransition.geometry.teleportPosition[index] & 0xffff;
    tgtY = (lvltransition.geometry.teleportPosition[index] >> 16) & 0xffff;

    return setupGenericLeveltransition(pName, tgtX, tgtY, dir);
}

/** Update the transition animation */
err updateLeveltransition() {
    gfmRV rv;
    int x, y;

    /* TODO Check that the operation was started */

    lvltransition.timer += game.elapsed;

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (lvltransition.timer < TRANSITION_TIME) {
        _tweenTilemapIn(x, y);
    }
    else if (lvltransition.timer < 2 * TRANSITION_TIME) {
        _tweenPlayers(x, y, TRANSITION_TIME);

        gfmTilemap_setPosition(lvltransition.pTransition, x - 16, y - 16);
    }
    else if (lvltransition.timer < 3 * TRANSITION_TIME) {
        if (!(lvltransition.flags & LT_LOADED)) {
            err erv;

            _setPlayersPosition();

            /* Load level & reset the FPS counter to ensure there's no lag */
            erv = loadPlaystate();
            ASSERT(erv == ERR_OK, erv);
            rv = gfm_resetFPS(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            lvltransition.flags |= LT_LOADED;

            /* Fix the transition layer position */
            gfmCamera_getPosition(&x, &y, game.pCamera);
            gfmTilemap_setPosition(lvltransition.pTransition, x - 16, y - 16);
        }
        _tweenTilemapOut(x, y);
    }
    else {
        /** Manually set the state so it doesn't get reloaded */
        game.currentState = ST_PLAYSTATE;
    }

    return ERR_OK;
}

/** Render the transition animation */
err drawLeveltransition() {
    gfmRV rv;
    err erv;

    erv = drawPlaystate();
    ASSERT(erv == ERR_OK, erv);
    rv =  gfmTilemap_draw(lvltransition.pTransition, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = drawPlayers();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** The transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES] = {
    -1, -1, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101
  , 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101
  , 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, -1, -1

  , -1, -1, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102
  , 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102
  , 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, -1, -1

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , -1, -1, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104
  , 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104
  , 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, -1, -1

  , -1, -1, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105
  , 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105
  , 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, -1, -1
};

