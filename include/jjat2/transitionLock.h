/**
 * @file include/jjat2/transitionLock.h
 */
#ifndef __JJAT2_TRANSITIONLOCK_H__
#define __JJAT2_TRANSITIONLOCK_H__

#include <GFraMe/gfmSprite.h>
#include <stdint.h>

/** Maximum number of different passages in a single load zone (usually 1 and
 * rarelly 2... but better keep it safe) */
#define MAX_LOCK_PER_LZ 2

struct stTLData {
    /** Starting position of the lock (in world space and in pixels). This
     * ignores the "generator" (the tile on the wall). */
    uint16_t x;
    uint16_t y;
    /** Dimensions of the lock, in tiles (ignoring the tile on the wall) */
    uint8_t width;
    uint8_t height;
};

struct stTransitionLock {
    /** Dummy sprite for the transition lock. It keeps track of the animation
     * for the top most tile, which is then used as a base for the other
     * tiles. */
    gfmSprite *pDummySprite;
    /** Current animation (IDLE, ACTIVE, FADE_OUT, FADE_IN) */
    uint8_t curAnim;
    /** Number of locks (exits) in this object */
    uint8_t count;
    /** Data for each connected lock */
    struct stTLData[MAX_LOCK_PER_LZ];
};
typedef struct stTransitionLock transitionLock;

#endif /* __JJAT2_TRANSITIONLOCK_H__ */

