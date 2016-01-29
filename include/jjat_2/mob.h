/**
 * Generic mob struct. Every 'object' (players, enemies etc.) should inherit
 * from this.
 *
 * The struct should be only directly accessed for reading attributes. Going
 * through getter can be somewhat annoying (create a temporary variable in the
 * stack, call the getter, check its return value of the getter, finally use the
 * value).
 *
 * Another unusual decision (when considering my previous
 * games/prototypes/stuff) is that the mob structure should be alloc'ed by the
 * sub-class it self. It seems pointless and counter-productive to, in a game
 * (i.e., a "self-contained" binary, in contrast to a library/framework/engine
 * that may be used as input for another project), force the developer to call
 * another function to simply do what can be done in the basic (i.e., the
 * sub-class) level... Again, the whole point is: when a library is updated, it
 * shouldn't break whatever is linked against it, but when a game is updated, it
 * won't break anything that depends on it (there should be no such thing).
 */
#ifndef __MOB_STRUCT__
#define __MOB_STRUCT__

typedef struct stMob mob;

#endif /* __MOB_STRUCT__ */

#ifndef __MOB_H__
#define __MOB_H__

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

#include <jjat_2/type.h>

/** The mob struct */
struct stMob {
    /** The mob's sub-class (a player, an enemy etc.) */
    void *pChild;
    /** The actual sprite */
    gfmSprite *pSpr;
    /** Which object (if any) this is standing over */
    gfmObject *pStandOver;
    /** Current animation */
    int curAnim;
    /** How long since the last animation started */
    int lastAnimElapsed;
    /** The sub-class' type */
    jjType type;
};

/**
 * Update the currently playing animation
 *
 * @param  [ in]pMob The mob
 * @param  [ in]anim The animation's index
 * @return           GFraMe return value
 */
gfmRV mob_playAnim(mob *pMob, int anim);

/**
 * Calls its sub-class "pre-update", updates the mob (physically) and collide it
 * against the world
 *
 * This step should update the AI for enemies and handle inputs for the player
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_update(mob *pMob);

/**
 * Handles standing over another mob, being actually hurt (?) and calls the
 * sub-class "post-update". There, each different class should update its own
 * animations and what-not.
 *
 * NOTE: Must be called after all sprites have collided
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_postUpdate(mob *pMob);

/**
 * Draw the sprite
 *
 * @param  [ in]pMob The mob
 * @return           GFraMe return value
 */
gfmRV mob_draw(mob *pMob);

#endif /* __MOB_H__ */

