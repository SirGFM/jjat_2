/**
 * @file include/jjat2/entity.h
 *
 * Base structure for physical entity (player & mobs)
 */
#ifndef __JJAT2_ENTITY_H__
#define __JJAT2_ENTITY_H__

#include <base/error.h>

#include <GFraMe/gfmSprite.h>

/** Default grace time to while jumping is possible after leaving the ground */
#define DEF_JUMP_GRACE  FRAMES_TO_MS(5)

/**
 * Calculate the initial jump speed.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_SPEED(timeToAppex, jumpHeight) \
  (-2.0 * TILES_TO_PX(jumpHeight) / FRAMES_TO_MS(timeToAppex))

/**
 * Calculate the gravity acceleration.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_ACCELERATION(timeToAppex, jumpHeight) \
  (2.0 * TILES_TO_PX(jumpHeight) / (FRAMES_TO_MS(timeToAppex) * \
    FRAMES_TO_MS(timeToAppex)))

/** Convert from tiles to pixels. Each tile is considered to be 8x8 pixels. */
#define TILES_TO_PX(n) (n * 8)

/** Convert from frames to milliseconds. To make it framerate independent, this
 * considers the game to be running at 60 FPS. */
#define FRAMES_TO_MS(n) (n * 16)

struct stEntityCtx {
    /** The internal sprite */
    gfmSprite *pSelf;
    /** Time, in milliseconds, while jump may be pressed after leaving the
     * ground */
    int jumpGrace;
    /** Currently playing animation */
    int currentAnimation;
    /** Number of animations */
    int maxAnimation;
    /** Initial jump velocity */
    int jumpVelocity;
    /**
     * Velocity set when short-hopping.
     *
     * On short-hop:
     *   if (vy < shorthopVelocity * 2) vy *= 0.5;
     *   else if (vy < shorthopVelocity) vy = shorthopVelocity
     */
    int shorthopVelocity;
    /** Default gravity when standing or jumping */
    int standGravity;
    /** Gravity set whenever the entity is falling. May be useful to make the
     * control feel better. */
    int fallGravity;
};
typedef struct stEntityCtx entityCtx;

/** NOTE: There's no initEntity/freeEntity because it would simply repeat all
 * parameters of gfmSprite (therefore, becoming pretty useless) */

/**
 * Set the current animation of the entity
 *
 * @param  [ in]entity    The entity
 * @param  [ in]animation Index of the animation to be played
 * @param  [ in]force     Whether the animation should be reset even if it was
 *                        being previously played
 */
err setEntityAnimation(entityCtx *entity, int animation, int force);

#endif /* __JJAT2_ENTITY_H__ */

