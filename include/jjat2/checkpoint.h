/**
 * @file include/jjat2/checkpoint.h
 */
#ifndef __JJAT2_CHECKPOINT_H__
#define __JJAT2_CHECKPOINT_H__

#include <base/error.h>
#include <conf/type.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>

struct stCheckpointCtx {
    leveltransitionData data;
};
typedef struct stCheckpointCtx checkpointCtx;

extern checkpointCtx checkpoint;

/**
 * Assign a new checkpoint, overwritting the previous one
 *
 * @param  [ in]pData Data for the level transition
 */
err setCheckpoint(leveltransitionData *pData);

/** Setup (and start) the level transition so the checkpoint is loaded. */
err loadCheckpoint();

#endif /* __JJAT2_CHECKPOINT_H__ */

