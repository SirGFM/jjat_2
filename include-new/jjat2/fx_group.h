/**
 * @file include/jjat2/fx_group.h
 *
 * Defines a group of sprites used for effects and/or temporary hitboxes.
 * Entities within this group may be physically controlled, but they doesn't
 * have to be.
 */
#ifndef __JJAT2_FX_GROUP_H__
#define __JJAT2_FX_GROUP_H__

#include <base/error.h>

#include <conf/type.h>

#include <GFraMe/gfmGroup.h>
#include <GFraMe/gfmSprite.h>

/** Maximum number of concurrent effects on screen */
#define MAX_FX_NUM  128

/** List of animations available */
enum enFxAnim {
      FX_SWORDY_SLASH_DOWN
    , FX_SWORDY_SLASH_UP
    , FX_GUNNY_BULLET
    , FX_TELEPORT_TARGET
    , FX_MAX
};
typedef enum enFxAnim fxAnim;

/** The group of effects/hitboxes */
extern gfmGroup *fx;

/**
 * Spawn a new effect at the desired position
 *
 * @param  [ in]x       Top left position of the effect
 * @param  [ in]y       Top left position of the effect
 * @param  [ in]w       Effect's width
 * @param  [ in]h       Effect's height
 * @param  [ in]flipped Whether the effect should be horizontally flipped
 * @param  [ in]ttl     How long the effect should live. <= 0 for infinity.
 * @param  [ in]anim    Animation played by the effect
 * @param  [ in]t       The effect type (relevant on collision)
 * @return              The spawned effect
 */
gfmSprite* spawnFx(int x, int y, int w, int h, int flipped, int ttl,
        fxAnim anim, type t);

/** Initialize the group */
err initFxGroup();

/** Free up all memory hold by a group */
void freeFxGroup();

/** Update and collide the effects */
err updateFxGroup();

#endif /* __JJAT2_FX_GROUP_H__ */

