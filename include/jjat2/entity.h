/**
 * @file include/jjat2/entity.h
 *
 * Base structure for physical entity (player & mobs)
 */
#ifndef __JJAT2_ENTITY_H__
#define __JJAT2_ENTITY_H__

#include <base/error.h>

#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmSprite.h>

#include <stdint.h>

/** Every sprite is facing right, by default. This enumeration reflects that to
 * make the code mode readable */
enum {
    DIR_RIGHT = 0
  , DIR_LEFT  = 1
};

enum enEntityFlag {
    EF_ALIVE           = 0x00001
  , EF_DEACTIVATE      = 0x00002
  , EF_SKIP_COLLISION  = 0x00004
  , EF_HAS_CARRIER     = 0x00008
  , EF_AVAILABLEF_FLAG = 0x10000

  , EF_AVAILABLE_BIT   = 16
};
typedef enum enEntityFlag entityFlag;

/** Absolute speed when airborne sprites are set as 'floating' */
#define FLOAT_SPEED     32
/** Maximum speed at which entities may fall */
#define MAX_FALL_SPEED  410

/** Default grace time to while jumping is possible after leaving the ground */
#define DEF_JUMP_GRACE  FRAMES_TO_MS(5)

/**
 * Calculate the initial jump speed.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_SPEED(timeToAppex, jumpHeight) \
  (-2.0 * TILES_TO_PX(jumpHeight) / FRAMES_TO_S(timeToAppex))

/**
 * Calculate the gravity acceleration.
 *
 * @param  [ in]timeToAppex Time, in 60FPS-frames, from the ground to the appex
 * @param  [ in]jumpHeight  Jump height in 8px-tiles
 */
#define JUMP_ACCELERATION(timeToAppex, jumpHeight) \
  (2.0 * TILES_TO_PX(jumpHeight) / (FRAMES_TO_S(timeToAppex) * \
    FRAMES_TO_S(timeToAppex)))

/** Convert from tiles to pixels. Each tile is considered to be 8x8 pixels. */
#define TILES_TO_PX(n) (n * 8)

/** Convert from frames to seconds. To make it framerate independent, this
 * considers the game to be running at 60 FPS. */
#define FRAMES_TO_S(n) (n / 60.0)
/** Convert from frames to milliseconds. To make it framerate independent, this
 * considers the game to be running at 60 FPS. */
#define FRAMES_TO_MS(n) (n * 16)

struct stEntityCtx {
    /** The internal sprite */
    gfmSprite *pSelf;
    /** Sprite (if any) that is carrying this entity */
    struct stEntityCtx *pCarrying;
    /** Entity's sight, if any */
    gfmHitbox *pSight;
    /** Generic entity flags */
    entityFlag flags;
    /** Time, in milliseconds, while jump may be pressed after leaving the
     * ground */
    int16_t jumpGrace;
    /** Initial jump velocity */
    int16_t jumpVelocity;
    /**
     * Velocity set when short-hopping.
     *
     * On short-hop:
     *   if (vy < shorthopVelocity * 2) vy *= 0.5;
     *   else if (vy < shorthopVelocity) vy = shorthopVelocity
     */
    int16_t shorthopVelocity;
    /** Default gravity when standing or jumping */
    int16_t standGravity;
    /** Gravity set whenever the entity is falling. May be useful to make the
     * control feel better. */
    int16_t fallGravity;
    /** Currently playing animation */
    uint8_t currentAnimation;
    /** Number of animations */
    uint8_t maxAnimation;
};
typedef struct stEntityCtx entityCtx;

/** NOTE: There's no freeEntity because it's usually static */

/**
 * Initialize the entity based on the previously set attributes
 *
 * @param  [ in]entity    The entity
 */
void initEntity(entityCtx *entity);

/**
 * Set the current animation of the entity
 *
 * @param  [ in]entity    The entity
 * @param  [ in]animation Index of the animation to be played
 * @param  [ in]force     Whether the animation should be reset even if it was
 *                        being previously played
 */
err setEntityAnimation(entityCtx *entity, int animation, int force);

/**
 * Update the entity's jump, based on the last frame and the jump button. This
 * function also updates the gravity (if fall gravity is different from jump).
 *
 * Changing to a jump state/animation is delegated to the caller. If jump gets
 * issued, the sprite's vertical velocity is set accordingly and ERR_DIDJUMP is
 * returned.
 *
 * @param  [ in]entity The entity
 * @param  [ in]jumpBt State of the jump button
 */
err updateEntityJump(entityCtx *entity, gfmInputState jumpBt);

/**
 * Collide the entity's sprite only against the static world
 *
 * @param  [ in]entity The entity
 */
err collideEntityStatic(entityCtx *entity);

/**
 * Collide the entity's sprite against the world
 *
 * @param  [ in]entity The entity
 */
err collideEntity(entityCtx *entity);

/**
 * Make an entity be carried by another,
 *
 * NOTE: To avoid collision issues with the environment, any entity that is
 * being carried should run a second collision against all static objects. This
 * solves potential zipping through platforms.
 *
 * @param  [ in]entity   The entity
 * @param  [ in]carrying The carrying the entity
 */
void carryEntity(entityCtx *entity, entityCtx *carrying);

/**
 * Finalize updating the entity's physics
 *
 * @param  [ in]entity   The entity
 */
err preUpdateEntity(entityCtx *entity);

/**
 * Post update an entity
 *
 * During this stage, the entity fixes its collision against the world and any
 * carrying sprite.
 *
 * @param  [ in]entity   The entity
 */
err postUpdateEntity(entityCtx *entity);

/**
 * Draw the entity
 *
 * @param  [ in]entity   The entity
 */
err drawEntity(entityCtx *entity);

/**
 * If the entity is offscreen, draw an 8x8 icon on its edge
 *
 * @param [ in]entity The entity
 * @param [ in]tile   The index/tile of the entity's icon
 */
err drawEntityIcon(entityCtx *entity, int tile);

/**
 * Updates an entity's direction according to its velocity.
 *
 * @param  [ in]entity   The entity
 */
void setEntityDirection(entityCtx *entity);

/**
 * Simple collision check between two entities
 *
 * If this function detects collision, it ensures one entity will be carried by
 * the other, and only that.
 *
 * @param  [ in]entA One of the entities
 * @param  [ in]entB The other entity
 */
void collideTwoEntities(entityCtx *entA, entityCtx *entB);

/**
 * Do some damage to the entity
 *
 * @param  [ in]entity The entity
 * @param  [ in]damage How much damage was dealt
 */
void hitEntity(entityCtx *entity, int damage);

/**
 * Kill the entity
 *
 * @param  [ in]entity The entity
 */
void killEntity(entityCtx *entity);

/**
 * Flip the entity if it reaches an edge
 *
 * @param  [ in]entity The entity
 * @param  [ in]vx     The entity's velocity
 */
void flipEntityOnEdge(entityCtx *entity, double vx);

#endif /* __JJAT2_ENTITY_H__ */

