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

static char _stCheckpointName[MAX_VALID_LEN + 1];

/**
 * Assign a new checkpoint, overwritting the previous one
 *
 * @param  [ in]pData Data for the level transition
 */
err setCheckpoint(leveltransitionData *pData) {
    checkpoint.data.pName = _stCheckpointName;

    strcpy(checkpoint.data.pName, pData->pName);
    checkpoint.data.tgtX = pData->tgtX;
    checkpoint.data.tgtY = pData->tgtY;

    return ERR_OK;
}

/** Setup (and start) the level transition so the checkpoint is loaded. */
err loadCheckpoint() {
    /* TODO Set the direction from the velocity of the killed player (if any) */
    checkpoint.data.dir = TEL_UP;
    switchToLevelTransition(&checkpoint.data);
    /* TODO Properly set this */
    lvltransition.flags |= LT_CHECKPOINT;
    playstate.pNextLevel = &checkpoint.data;
    /* TODO Reset player life */

    return ERR_OK;
}

