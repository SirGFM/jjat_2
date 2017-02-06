/**
 * @file src/jjat2/checkpoint.c
 */
#include <base/error.h>
#include <base/game.h>
#include <conf/state.h>
#include <GFraMe/gfmUtils.h>
#include <jjat2/checkpoint.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <string.h>

/**
 * Assign a new checkpoint, overwritting the previous one
 *
 * @param  [ in]pName Name of the level
 * @param  [ in]tgtX  Position of the players
 * @param  [ in]tgtY  Position of the players
 */
err setCheckpoint(char *pName, int tgtX, int tgtY) {
    int len;

    ASSERT(pName != 0, ERR_ARGUMENTBAD);

    len = strnlen(pName, MAX_VALID_LEN - 1);
    memcpy(checkpoint.pName, pName, len);
    checkpoint.pName[len] = '\0';

    checkpoint.tgtX = tgtX;
    checkpoint.tgtY = tgtY;

    return ERR_OK;
}

/** Setup (and start) the level transition so the checkpoint is loaded. */
err loadCheckpoint() {
    err erv;
    levelTransitionFlags dir;
    
    /* TODO Set the direction from the velocity of the killed player (if any) */
    dir = TEL_UP;
    erv = setupGenericLeveltransition(checkpoint.pName, checkpoint.tgtX
            , checkpoint.tgtY, dir);
    ASSERT(erv == ERR_OK, erv);

    game.nextState = ST_LEVELTRANSITION;

    return ERR_OK;
}

