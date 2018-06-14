#include <base/error.h>
#include <jjat/transitionLock.h>
#include <GFraMe/gfmSprite.h>
#include <stdint.h>

#define lockSpr_width 8
#define lockSpr_height 8
#define lockSpr_offx 0
#define lockSpr_offy 0

/* Index of the starting tile of any single "lane"/segment */
enum enLaneIdx {
    V_TOP = 813
  , V_MID = 877
  , V_BOT = 941
  , H_TOP = 1005
  , H_MID = 1069
  , H_BOT = 1133
};

/** List of animations */
enum enAnim {
    IDLE = 0
  , ACTIVE
  , OPEN
  , CLOSE
  , TR_LOCK_ANIM_COUNT
};

/** Transition lock animation data. Everything is 0-indexed so calculating the
 * frame for a given "lane"/segment is easier. */
static int pTrLockAnimData[] = {
/*           len|fps|loop|data... */
/*  IDLE  */  1 , 8 ,  0 , 0
/* ACTIVE */, 4 , 8 ,  1 , 1,0,2,0
/*  OPEN  */, 5 , 8 ,  0 , 3,4,5,6
/*  CLOSE */, 4 , 8 ,  0 , 6,5,4,3
};

/**
 * Initialize a transition lock
 *
 * @param  [ in]lock The object to be initialized
 */
err initTrLock(transitionLock *lock) {
    err erv;
    gfmRV rv;

    /* Initialize the sprite */
    rv = gfmSprite_getNew(&lock->pDummySprite);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_init(lock->pDummySprite, 0, 0, lockSpr_width, lockSpr_height
            , gfx.pSset8x8, lockSpr_offx, lockSpr_offy, lock, T_TR_LOCK);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmSprite_addAnimationsStatic(lock->pDummySprite, pTrLockAnimData);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/**
 * Release all memory alloc'ed by the structure.
 *
 * @param  [ in]lock The object to be freed
 */
void freeTrLock(transitionLock *lock) {
    if (lock->pDummySprite) {
        gfmSprite_free(&lock->pDummySprite);
    }
}

