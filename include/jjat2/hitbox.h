/**
 * @file include/jjat/hitbox.h
 *
 * Manages fixed hitboxes, that last until the next level is loaded, and
 * transient ones, that last until the following frame.
 */
#ifndef __JJAT2_HITBOX_H__
#define __JJAT2_HITBOX_H__

#include <base/error.h>
#include <GFraMe/gfmHitbox.h>
#include <stdint.h>

#define MAX_HITBOXES    32

struct stHitboxesCtx {
    /** The list of hitboxes */
    gfmHitbox *pList;
    /** How many fixed hitboxes there are, from the start of the list */
    uint8_t used;
    /** Transient hitboxes count, starting at the end of the list */
    uint8_t tmpUsed;
};
typedef struct stHitboxesCtx hitboxesCtx;

extern hitboxesCtx hitboxes;

/** Initialize the context */
err initHitboxes();

/** Free the context */
void freeHitboxes();

/** Reset the context for recycling */
void resetHitboxes();

/** Reset only the transient hitboxes for recycling (should be called every
 * frame) */
void resetTmpHitboxes();

/**
 * Spawn a hitbox that should last until changing the level. It returns the
 * spawned object.
 *
 * @param  [ in]pCtx   Context associated with the hitbox
 * @param  [ in]x      The hitbox position
 * @param  [ in]y      The hitbox position
 * @param  [ in]width  The hitbox dimensions
 * @param  [ in]height The hitbox dimensions
 * @param  [ in]type   The hitbox type
 */
gfmHitbox* spawnFixedHitbox(void *pCtx, int x, int y, int width, int height, int type);

/**
 * Spawn a hitbox that should last until the next frame. It returns the
 * spawned object.
 *
 * @param  [ in]pCtx   Context associated with the hitbox
 * @param  [ in]x      The hitbox position
 * @param  [ in]y      The hitbox position
 * @param  [ in]width  The hitbox dimensions
 * @param  [ in]height The hitbox dimensions
 * @param  [ in]type   The hitbox type
 */
gfmHitbox* spawnTmpHitbox(void *pCtx, int x, int y, int width, int height, int type);

/** Collide every hitbox */
err collideHitbox();

#endif /* __JJAT2_HITBOX_H__ */

