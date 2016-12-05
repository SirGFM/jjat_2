/**
 * @file include/jjat2/teleport.c
 *
 * Handles the teleporter gun.
 *
 * As soon as a bullet hits a valid target, a new effect shall be spawned. If
 * the target is an entity, the entity should be stored on 'pTarget'. Otherwise,
 * the target should be part of the tilemap and its position is only stored on
 * the effect's position.
 */
#ifndef __JJAT2_TELEPORT_H__
#define __JJAT2_TELEPORT_H__

#include <GFraMe/gfmGroup.h>

#include <jjat2/entity.h>

struct stTeleportCtx {
    /** Current node following whatever is being targeted */
    gfmGroupNode *pCurEffect;
    /** The current target (if an entity) */
    entityCtx *pTarget;
};
typedef struct stTeleportCtx teleportCtx;

/** The current target (if an entity) */
extern teleportCtx teleport;

/** Remove the previous target */
void resetTeleporterTarget();

/**
 * Setup the teleport target at a static position
 *
 * @param  [ in]x Target's center
 * @param  [ in]y Target's center
 */
err teleporterTargetPosition(int x, int y);

/**
 * Setup the teleport target at an entity
 *
 * @param  [ in]pTarget The entity
 */
err teleporterTargetEntity(entityCtx *pTarget);

#endif /* __JJAT2_TELEPORT_H__ */

