/**
 * @file include/jjat2/checkpoint.h
 */
#ifndef __JJAT2_CHECKPOINT_H__
#define __JJAT2_CHECKPOINT_H__

#include <base/error.h>
#include <jjat2/playstate.h>

struct stCheckpointCtx {
    /** Name of the level */
    char pName[MAX_VALID_LEN];
    /** Position of the players */
    int tgtX;
    /** Position of the players */
    int tgtY;
};
typedef struct stCheckpointCtx checkpointCtx;

extern checkpointCtx checkpoint;

/**
 * Assign a new checkpoint, overwritting the previous one
 *
 * @param  [ in]pName Name of the level
 * @param  [ in]tgtX  Position of the players
 * @param  [ in]tgtY  Position of the players
 */
err setCheckpoint(char *pName, int tgtX, int tgtY);

/** Setup (and start) the level transition so the checkpoint is loaded. */
err loadCheckpoint();

#endif /* __JJAT2_CHECKPOINT_H__ */

