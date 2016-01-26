/**
 * @file src/level.c
 *
 * Uses a gfmParser to load the desired level
 */
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>

#include <jjat_2/level.h>
#include <jjat_2/type.h>

char *pStrDict[] = {
    "terrain"
};
int pTypeDict[] = {
    T_TERRAIN
};
const int dictLen = sizeof(pTypeDict) / sizeof(int);

/**
 * Do actually load the current level
 *
 * @param  [ in]pTilemap Path to the tilemap
 * @param  [ in]mapLen   Length of the path to the tilemap
 * @param  [ in]pObjects Path to the objects
 * @param  [ in]objLen   Length of the path to the objects
 * @return               GFraMe return value
 */
static gfmRV level_doLoad(char *pTilemap, int mapLen, char *pObjects,
        int objLen) {
    /* Parses all objects in pObjects */
    gfmParser *pParser;
    /** Return value */
    gfmRV rv;

    pParser = 0;

    /* Load the tilemap */
    rv = gfmTilemap_loadf(pGlobal->pTMap, pGame->pCtx, pTilemap, mapLen,
            pStrDict, pTypeDict, dictLen);
    ASSERT(rv == GFMRV_OK, rv);

    /* TODO Parse & load the objects */

    rv = GFMRV_OK;
__ret:
    if (pParser) {
        gfmParser_free(&pParser);
    }

    return rv;
}

/**
 * Loads a level
 *
 * @param  [ in]index The index of the desired level
 * @return            GFraMe return value
 */
gfmRV level_load(int index) {
    /* Path for tilemap and objects */
    char pMap[] = "levels/map_tm_000.gfm";
    char pObj[] = "levels/map_obj_000.gfm";
    /* Index of the digit being written */
    int i;

    /* Set the actual level to be loaded */
    i = 0;
    while (index > 0) {
        char c;

        /* Convert the current digit to a ASCII character */
        c = index % 10;
        index /= 10;
        c += '0';

        /* Insert it at the string */
        pMap[sizeof(pMap) - 5 - i] = c;
        pObj[sizeof(pObj) - 5 - i] = c;
        i++;
    }

    /* Do actually load it */
    return level_doLoad(pMap, sizeof(pMap) - 1, pObj, sizeof(pObj) - 1);
}

/**
 * Load the test level
 *
 * @return            GFraMe return value
 */
gfmRV level_loadTest() {
#define TEST_TM "levels/map_test_tm.gfm"
#define TEST_OBJ "levels/map_test_obj.gfm"
    return level_doLoad(TEST_TM, sizeof(TEST_TM) - 1, TEST_OBJ,
            sizeof(TEST_OBJ) - 1);
}

