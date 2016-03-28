/**
 * Generic entity
 */
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmSprite.h>

struct stEntity {
    /** The actual sprite */
    gfmSprite *pSprite;
    /** Which object (if any) this is standing over */
    gfmObject *pStanding;
    /** Current animation */
    int curAnim;
    /** 32-bits for each entity to store its state (should be enough) */
    int entityData;
};
typedef struct stEntity entity;

/**
 * Release an entity and all of its resources
 *
 * @param  [out] ppEnt The entity
 */
void entity_free(entity **ppEnt);

/**
 * Update the currently playing animation
 *
 * @param  [ in]pEnt The entity
 * @param  [ in]anim The animation's index
 * @return           GFraMe return value
 */
gfmRV entity_playAnimation(entity *pEnt, int anim);

/**
 * If the entity is standing on another one, update it
 *
 * @param  [ in]pEnt The entity
 * @return           GFraMe return value
 */
gfmRV entity_updateStanding(entity *pEnt);

/**
 * Collide the entity's sprite against the world
 *
 * @param  [ in]pEnt The entity
 * @return           GFraMe return value
 */
gfmRV entity_collide(entity *pEnt);

#endif /* __ENTITY_H__ */

