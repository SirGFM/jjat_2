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

#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmTilemap.h>

#include <string.h>

#define TRANSITION_TIME 750
#define WIDTH_IN_TILES  (V_WIDTH / 8 + 4)
#define HEIGHT_IN_TILES (V_HEIGHT / 8 + 4)
#define TM_DEFAULT_TILE -1

/** Region where the name of maps accessible from the current one is stored */
static char _stMapsName[MAX_AREAS * (MAX_VALID_LEN + 1)];

/** Forward declaration of the transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES];

/** Packed teleport/loadzone data that is stored within the type */
enum {
    TEL_UP         = 0x00000000
  , TEL_DOWN       = 0x10000000
  , TEL_LEFT       = 0x20000000
  , TEL_RIGHT      = 0x30000000

  , TEL_DIR_MASK   = 0x30000000
  , TEL_INDEX_MASK = 0x0ff00000
  , TEL_INDEX_BITS = 20
};

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
    lvltransition.index = (uint8_t)index;
    game.nextState = ST_LEVELTRANSITION;
}

/** Retrieve the name of the level to be loaded */
char* getNextLevelName() {
    ASSERT(lvltransition.index < lvltransition.areasCount, 0);
    return lvltransition.pNames[lvltransition.index];
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
        rv = gfmObject_getNew(&lvltransition.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        lvltransition.pNames[i] = _stMapsName + (MAX_VALID_LEN + 1) * i;
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
        gfmObject_free(&lvltransition.pAreas[i]);
        i++;
    }
}

/** Reset all objects parsed on the previous level */
void resetLeveltransition() {
    lvltransition.areasCount = 0;
}

/** Setup static collision against all level transition objects */
err calculateStaticLeveltransition() {
    gfmRV rv;
    int i;

    i = 0;
    while (i < lvltransition.areasCount) {
        rv = gfmObject_setFixed(lvltransition.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmObject_update(lvltransition.pAreas[i], game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmQuadtree_populateObject(collision.pStaticQt
                , lvltransition.pAreas[i]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        i++;
    }

    return ERR_OK;
}

/**
 * Parse a loadzone
 *
 * @param  [ in]pParser The parser pointing at a loadzone
 */
err parseLoadzone(gfmParser *pParser) {
    gfmObject *pObj;
    uint32_t *pos;
    char *pName;
    gfmRV rv;
    int dir, i, h, l, tgtX, tgtY, type, w, x, y;

    ASSERT(lvltransition.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    pObj = lvltransition.pAreas[lvltransition.areasCount];
    pos = &lvltransition.teleportPosition[lvltransition.areasCount];
    pName = lvltransition.pNames[lvltransition.areasCount];

    rv = gfmParser_getPos(&x, &y, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmParser_getNumProperties(&l, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    dir = -1;
    tgtX = -1;
    tgtY = -1;
    pName[0] = '\0';
    i = 0;
    while (i < l) {
        char *pKey, *pVal;

        rv = gfmParser_getProperty(&pKey, &pVal, playstate.pParser, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        if (strcmp(pKey, "tgt_x") == 0) {
            tgtX = _getInt(pVal);
        }
        else if (strcmp(pKey, "tgt_y") == 0) {
            tgtY = _getInt(pVal);
        }
        else if (strcmp(pKey, "dest") == 0) {
            ASSERT(strlen(pVal) < MAX_VALID_LEN, ERR_PARSINGERR);
            strcpy(pName, pVal);
        }
        else if (strcmp(pKey, "dir") == 0) {
            if (strcmp(pVal, "left") == 0) {
                dir = TEL_LEFT;
            }
            else if (strcmp(pVal, "right") == 0) {
                dir = TEL_RIGHT;
            }
            else if (strcmp(pVal, "up") == 0) {
                dir = TEL_UP;
            }
            else if (strcmp(pVal, "down") == 0) {
                dir = TEL_DOWN;
            }
            else {
                ASSERT(0, ERR_PARSINGERR);
            }
        }

        i++;
    }
    ASSERT(dir != -1, ERR_PARSINGERR);
    ASSERT(tgtX > -1 && tgtX < 0x10000, ERR_PARSINGERR);
    ASSERT(tgtY > -1 && tgtY < 0x10000, ERR_PARSINGERR);
    ASSERT(pName[0] != '\0', ERR_PARSINGERR);

    *pos = (tgtY << 16) | tgtX;

    type = T_LOADZONE;
    type |= TEL_INDEX_MASK & (lvltransition.areasCount << TEL_INDEX_BITS);
    type |= dir;
    rv = gfmObject_init(pObj, x, y, w, h, 0/*child*/, type);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.areasCount++;
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

    ASSERT(lvltransition.areasCount < MAX_AREAS, ERR_BUFFERTOOSMALL);

    pObj = lvltransition.pAreas[lvltransition.areasCount];

    rv = gfmParser_getPos(&x, &y, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmParser_getDimensions(&w, &h, playstate.pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmObject_init(pObj, x, y, w, h, 0/*child*/, T_FLOOR_NOTP);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.areasCount++;
    return ERR_OK;
}

/** Prepare the transition animation */
err setupLeveltransition() {
    void *pChild;
    gfmRV rv;
    int type, x, y;

    ASSERT(lvltransition.index < lvltransition.areasCount, ERR_INDEXOOB);

    rv = gfmObject_getChild(&pChild, &type
            , lvltransition.pAreas[lvltransition.index]);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    switch (type & TEL_DIR_MASK) {
        case TEL_UP: {
            x -= 16;
            y += HEIGHT_IN_TILES * 8;
        } break;
        case TEL_DOWN: {
            x -= 16;
            y -= HEIGHT_IN_TILES * 8;
        } break;
        case TEL_LEFT: {
            x += WIDTH_IN_TILES * 8;
            y -= 16;
        } break;
        case TEL_RIGHT: {
            x -= WIDTH_IN_TILES * 8;
            y -= 16;
        } break;
    }

    rv = gfmTilemap_setPosition(lvltransition.pTransition, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    lvltransition.timer = 0;

    return ERR_OK;
}

/** Update the transition animation */
err updateLeveltransition() {
    void *pChild;
    gfmRV rv;
    int type, x, y;

    ASSERT(lvltransition.index < lvltransition.areasCount, ERR_INDEXOOB);

    rv = gfmObject_getChild(&pChild, &type
            , lvltransition.pAreas[lvltransition.index]);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    lvltransition.timer += game.elapsed;

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

#define TWEEN(var_op) \
    do { \
        var_op ((TRANSITION_TIME - lvltransition.timer) * START_POS) / TRANSITION_TIME \
                + (lvltransition.timer * END_POS) / TRANSITION_TIME; \
    } while (0)

    if (lvltransition.timer < TRANSITION_TIME) {
        switch (type & TEL_DIR_MASK) {
            case TEL_UP: {
                x -= 16;
#define START_POS (V_HEIGHT)
#define END_POS   (-16)
                TWEEN(y +=);
#undef START_POS
#undef END_POS
            } break;
            case TEL_DOWN: {
                x -= 16;
#define START_POS (-HEIGHT_IN_TILES * 8)
#define END_POS   (-16)
                TWEEN(y +=);
#undef START_POS
#undef END_POS
            } break;
            case TEL_LEFT: {
                y -= 16;
#define START_POS (V_WIDTH)
#define END_POS   (-16)
                TWEEN(x +=);
#undef START_POS
#undef END_POS
            } break;
            case TEL_RIGHT: {
                y -= 16;
#define START_POS (-WIDTH_IN_TILES * 8)
#define END_POS   (-16)
                TWEEN(x +=);
#undef START_POS
#undef END_POS
            } break;
        }
    }

    rv = gfmTilemap_setPosition(lvltransition.pTransition, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

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

