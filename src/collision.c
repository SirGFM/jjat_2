/**
 * @file src/collision.c
 *
 * Declare only the collision function.
 *
 * In a way to try and keep this saner, collision "sets" (e.g., if types 'a',
 * 'b' and and 'c' collide against 'a', 'd' or 'f') are described on a separated
 * json file, which automatically (i.e., from the Makefile) generates the
 * described switch-case code.
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <conf/type.h>

#include <jjat2/checkpoint.h>
#include <jjat2/entity.h>
#include <jjat2/fx_group.h>
#include <jjat2/gunny.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/teleport.h>
#include <jjat2/enemies/g_walky.h>

#include <GFraMe/gfmError.h>
#include <GFraMe/gfmObject.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

#if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
#  include <stdlib.h>
#  include <signal.h>
#endif

/** Hold all pointers (and the type) for a colliding object */
struct stCollisionNode {
    gfmObject *pObject;
    gfmSprite *pSprite;
    void *pChild;
    int type;
};
typedef struct stCollisionNode collisionNode;

#define SPIKE_OFFSET    4

/** Merge two types into a single one */
#define MERGE_TYPES(type1, type2) \
    (TYPE(type1) | (TYPE(type2) << T_BITS))

/**
 * Check if two entities of the specified type collided.
 *
 * It requires two integers:
 *   - isFirstCase: Set if collision triggered for the first case. If 0, it was
 *                  triggered by the second case (and, therefore, the objects
 *                  must be swapped).
 *   - fallthrough: Set to fallthrough the following cases without modifying
 *                  isFirstCase. Only required if multiples CASEs are handled by
 *                  a single block.
 */
#define CASE(type1, type2) \
    case (MERGE_TYPES(type1, type2)): \
        if (!fallthrough) { \
            isFirstCase = 1; \
        } \
        fallthrough = 1; \
    case (MERGE_TYPES(type2, type1)): \
        fallthrough = 1;

/** Collide against self type */
#define SELFCASE(type) \
    case (MERGE_TYPES(type, type)):

/** Ignore collision between entities of different types */
#define IGNORE(type1, type2) \
    case (MERGE_TYPES(type1, type2)): \
    case (MERGE_TYPES(type2, type1)):

/**
 * Retrieve the type and all the children for a given object.
 *
 * @param  [ in]pNode The object (with a valid gfmObject)
 */
static void _getSubtype(collisionNode *pNode) {
    gfmObject_getChild(&pNode->pChild, &pNode->type, pNode->pObject);

    if (pNode->type == gfmType_sprite && pNode->pChild) {
        pNode->pSprite = (gfmSprite*)pNode->pChild;
        gfmSprite_getChild(&pNode->pChild, &pNode->type, pNode->pSprite);
    }
}

/**
 * Remove the bullet and spawn an explosion in its place
 *
 * @param  [ in]bullet The bullet that collided
 */
static void _explodeStar(collisionNode *bullet) {
    gfmSprite *pSpr;
    int x, y;

    gfmSprite_getPosition(&x, &y, bullet->pSprite);
    gfmGroup_removeNode((gfmGroupNode*)bullet->pChild);
    pSpr = spawnFx(x, y, 4/*w*/, 4/*h*/, 0/*dir*/, 250/*ttl*/, FX_STAR_EXPLOSION
            , T_FX);
    if (pSpr) {
        gfmSprite_setOffset(pSpr, -1, -1);
    }
}

/**
 * Handle collision between a floor object and an entity
 */
static inline err _defaultFloorCollision(collisionNode *floor
        , collisionNode *entity) {
    gfmRV rv;

#if defined(JJATENGINE)
    /* Ignore collision if against a platform that hasn't been activated */
    if (TYPE(floor->type) == T_BLUE_PLATFORM
            && !(game.sessionFlags & SF_BLUE_ACTIVE)) {
        return ERR_OK;
    }
#endif /* JJATENGINE */

    rv = gfmObject_justOverlaped(floor->pObject, entity->pObject);
    if (rv == GFMRV_TRUE) {
        gfmCollision dir;

        gfmObject_getCurrentCollision(&dir, entity->pObject);

        if (!((dir & gfmCollision_up) && (dir & gfmCollision_hor))) {
            gfmObject_collide(floor->pObject, entity->pObject);
        }
        else {
            /* Fix colliding against corners when there are two
             * separated objects in a wall */
            gfmObject_separateHorizontal(floor->pObject, entity->pObject);
        }

        if (dir & gfmCollision_down) {
            gfmObject_setVerticalVelocity(entity->pObject, 0);
            /* Corner case!! If the entity would get stuck on a
             * corner, push 'em toward the platform */
            if (dir & gfmCollision_left) {
                int x, y;
                gfmObject_getPosition(&x, &y, entity->pObject);
                gfmObject_setPosition(entity->pObject, x - 1, y - 1);
            }
            else if (dir & gfmCollision_right) {
                int x, y;
                gfmObject_getPosition(&x, &y, entity->pObject);
                gfmObject_setPosition(entity->pObject, x + 1, y - 1);
            }
        }
        else if ((dir & gfmCollision_up) && !(dir & gfmCollision_hor)) {
            int y;
            gfmObject_setVerticalVelocity(entity->pObject, 0);
            gfmObject_getVerticalPosition(&y, entity->pObject);
            gfmObject_setVerticalPosition(entity->pObject, y + 1);

        }
    } /* if (rv == GFMRV_TRUE) */
#if  defined(JJATENGINE)
    else if (collision.flags & CF_FIXTELEPORT) {
        /* Entity was (possibly) just teleported into the ground.
         * Manually check and fix it */
        if (gfmObject_isOverlaping(floor->pObject, entity->pObject)
                == GFMRV_TRUE) {
            int fh, fy, ph, py;

            gfmObject_getVerticalPosition(&fy, floor->pObject);
            gfmObject_getHeight(&fh, floor->pObject);
            gfmObject_getVerticalPosition(&py, entity->pObject);
            gfmObject_getHeight(&ph, entity->pObject);

            if (py >= fy) {
                gfmObject_setVerticalPosition(entity->pObject, fy + fh);
            }
            else if (py + ph <= fy + fh) {
                gfmObject_setVerticalPosition(entity->pObject, fy - ph);
            }
        }
    }
#endif  /* JJATENGINE */

    return ERR_OK;
}

/** Collision when a teleport bullet should be ignored */
static inline err _ignoreTeleportBullet(collisionNode *bullet
        , collisionNode *other) {
    gfmGroupNode *pNode;
    gfmRV rv;

    pNode = (gfmGroupNode*)bullet->pChild;
    /* TODO Play vanish animation */
    rv = gfmGroup_removeNode(pNode);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Collision between any entity and a teleport bullet */
static inline err _setTeleportEntity(collisionNode *bullet
        , collisionNode *other) {
    gfmGroupNode *pNode;
    entityCtx *pEntity;
    err erv;
    gfmRV rv;

    pNode = (gfmGroupNode*)bullet->pChild;
    pEntity = (entityCtx*)other->pChild;

    /* Check if visible */
    if (gfm_isSpriteInsideCamera(game.pCtx, pEntity->pSelf) == GFMRV_TRUE ) {
        if (TYPE(other->type) != T_EN_G_WALKY
                || onGreenWalkyAttacked(pEntity, bullet->pObject) == ERR_OK) {
            erv = teleporterTargetEntity(pEntity);
            ASSERT(erv == ERR_OK, erv);
        }
    }
    rv = gfmGroup_removeNode(pNode);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Collision between a floor and a teleport bullet */
static inline err _setTeleportFloor(collisionNode *bullet
        , collisionNode *floor) {
    gfmGroupNode *pNode;
    gfmObject *pBullet, *pFloor;
    err erv;
    gfmRV rv;

#if defined(JJATENGINE)
    /* Ignore collision if against a platform that hasn't been activated */
    if (TYPE(floor->type) == T_BLUE_PLATFORM
            && !(game.sessionFlags & SF_BLUE_ACTIVE)) {
        return ERR_OK;
    }
#endif /* JJATENGINE */

    pBullet = bullet->pObject;
    pNode = (gfmGroupNode*)bullet->pChild;
    pFloor = floor->pObject;

    /* Check if visible */
    if (GFMRV_TRUE == gfm_isObjectInsideCamera(game.pCtx, pFloor)) {
        int cx, x, y;
        teleportPosition pos;

        /* Since the bullet only moves horizontally, use the floor's
         * horizontal position and the bullet's vertical */
        rv = gfmObject_getLastCenter(&cx, &y, pBullet);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfmObject_getHorizontalPosition(&x, pFloor);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /* Ugly, hacky assumption: if the center - half the width is
         * to the right of the floor's left side, then it's to the
         * floor's right */
        if (cx - BULLET_WIDTH / 2 >= x) {
            int w;
            /* The bullet collided to the left, so adjust its position */
            rv = gfmObject_getWidth(&w, pFloor);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
            x += w;
            x += BULLET_WIDTH / 2;
            pos = TP_LEFT;
        }
        else {
            x -= BULLET_WIDTH / 2;
            pos = TP_RIGHT;
        }

        erv = teleporterTargetPosition(x, y, pos);
        ASSERT(erv == ERR_OK, erv);
    }
    rv = gfmGroup_removeNode(pNode);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Handle collision between a floor object and a particle */
static inline err _floorProjectileCollision(collisionNode *floor
        , collisionNode *particle) {
    if (TYPE(particle->type) == T_EN_G_WALKY_ATK) {
        _explodeStar(particle);
    }

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Deny a single projectile, when it collides against something else */
static inline err _denyProjectile(collisionNode *projectile
        , collisionNode *other) {
    if (TYPE(projectile->type) == T_EN_G_WALKY_ATK) {
        _explodeStar(projectile);
    }

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Collide a player against another player's dummy */
static inline err _collidePlayerDummy(collisionNode *player
        , collisionNode *dummy) {
    int playerY, dummyY;

    gfmObject_getVerticalPosition(&playerY, player->pObject);
    gfmObject_getVerticalPosition(&dummyY, dummy->pObject);

    gfmObject_justOverlaped(player->pObject, dummy->pObject);
    if (playerY < dummyY) {
        gfmCollision dir;

        /* Only separate the player if it's above the dummy */
        gfmObject_separateVertical(player->pObject, dummy->pObject);
        gfmObject_getCurrentCollision(&dir, player->pObject);
        if (dir & gfmCollision_down) {
            gfmObject_setVerticalVelocity(player->pObject, 0);
        }
    }

    return ERR_OK;
}

/** Collide a loadzone against a player */
static inline err _collideLoadzonePlayer(collisionNode *loadzone
        , collisionNode *player) {
    onHitLoadzone(player->type, (leveltransitionData*)loadzone->pChild);

    return ERR_OK;
}

/** Collide an environmental harm (spikes, lava(?) etc) against an entity */
static inline err _environmentalHarmEntity(collisionNode *harm
        , collisionNode *entity) {
    gfmCollision dir;
    int py, sy, ph;

    gfmObject_justOverlaped(harm->pObject, entity->pObject);
    gfmObject_getCurrentCollision(&dir, entity->pObject);

    gfmObject_getVerticalPosition(&py, entity->pObject);
    gfmObject_getHeight(&ph, entity->pObject);
    gfmObject_getVerticalPosition(&sy, harm->pObject);

    if (py + ph < sy + SPIKE_OFFSET) {
        /* Does nothing unless within the collideable aread */
    }
    else if (dir & gfmCollision_hor) {
        /* Collide horizontally to avoid clipping */
        gfmObject_separateHorizontal(harm->pObject, entity->pObject);
    }
    else {
        /* Kill the entity */
        killEntity((entityCtx*)entity->pChild);
    }

    return ERR_OK;
}

/** Collision between a checkpoint and a player */
static inline err _checkpointCollision(collisionNode *checkpoint
        , collisionNode *player) {
    gfmSprite *pSpr;
    int h, w, x, y;
    err erv;

    gfmObject_getPosition(&x, &y, checkpoint->pObject);
    gfmObject_getDimensions(&w, &h, checkpoint->pObject);

    /* Set the checkpoint */
    erv = setCheckpoint((leveltransitionData*)checkpoint->pChild);
    ASSERT(erv == ERR_OK, erv);

    pSpr = spawnFx(x, y, w, h, 0/*dir*/, checkpointSavedDuration
            , FX_CHECKPOINT_SAVED, T_FX);
    ASSERT(pSpr != 0, ERR_BUFFERTOOSMALL);

    gfmObject_setType(checkpoint->pObject, T_FX);

    return ERR_OK;
}

/** Collision between an spiky and another entity */
static inline err _spikyEntityCollision(collisionNode *spiky
        , collisionNode *ent) {
    if (gfmObject_justOverlaped(spiky->pObject, ent->pObject) == GFMRV_TRUE) {
        gfmCollision col;
        int dir;

        gfmSprite_getCollision(&col, ((entityCtx*)spiky->pChild)->pSelf);
        gfmSprite_getDirection(&dir, ((entityCtx*)spiky->pChild)->pSelf);

        if ((dir == DIR_RIGHT && (col & gfmCollision_right))
                || (dir == DIR_LEFT && (col & gfmCollision_left))) {
            int damage;

            damage = spiky->type >> T_BITS;
            hitEntity((entityCtx*)ent->pChild, damage);
            collision.flags |= CF_SKIP;
        }
    }

    return ERR_OK;
}

/** Collision between two entities */
static inline err _entityCollision(collisionNode *entA, collisionNode *entB) {
    if (TYPE(entA->type) == T_EN_SPIKY && TYPE(entB->type) != T_EN_SPIKY) {
        return _spikyEntityCollision(entA, entB);
    }
    else if (TYPE(entB->type) == T_EN_SPIKY && TYPE(entA->type) != T_EN_SPIKY) {
        return _spikyEntityCollision(entB, entA);
    }
    else {
        if (entA != entB) {
            collideTwoEntities((entityCtx*)entA->pChild
                    , (entityCtx*)entB->pChild);
        }

        return ERR_OK;
    }
}

/** Collision between a g_walky's view and an entity */
static inline err _gWalkyViewEntityCollision(collisionNode *view
        , collisionNode *entity) {
    triggerGreenWalkyAttack((entityCtx*)view->pChild);

    return ERR_OK;
}

/** Collision between a sword attack and a projectile */
static inline err _swordReflectProjectile(collisionNode *sword
        , collisionNode *projectile) {
    double vx;
    int dir;

    gfmSprite_getHorizontalVelocity(&vx, projectile->pSprite);
    gfmSprite_getDirection(&dir, projectile->pSprite);
    gfmSprite_setHorizontalVelocity(projectile->pSprite, -vx);
    gfmSprite_setDirection(projectile->pSprite, !dir);

    gfmSprite_setType(sword->pSprite, T_SWORD_FX);
    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/** Collision between an attack and an entity */
static inline err _attackEntity(collisionNode *attack
        , collisionNode *entity) {
    int damage;

    damage = attack->type >> T_BITS;
    if (TYPE(attack->type) == T_ATK_SWORD) {
        /* TODO Make this damage based */
        damage = 1000;
    }

    if (TYPE(entity->type) == T_EN_G_WALKY) {
        gfmObject_justOverlaped(attack->pObject, entity->pObject);
        if (onGreenWalkyAttacked((entityCtx*)entity->pChild, attack->pObject)
                == ERR_OK) {
            hitEntity((entityCtx*)entity->pChild, damage);
        }
    }
    else {
        hitEntity((entityCtx*)entity->pChild, damage);
    }

    /* De-activate the attack */
    switch (TYPE(attack->type)) {
        case T_ATK_SWORD: {
            gfmSprite_setType(attack->pSprite, T_SWORD_FX);
        } break;
        case T_EN_G_WALKY_ATK: {
            _explodeStar(attack);
        } break;
    }

    collision.flags |= CF_SKIP;
    return ERR_OK;
}

/**
 * Continue handling collision.
 * 
 * Different from the other functions on this module, this one is declared on
 * src/collision.c (instead of src/base/collision.c). This decision was made
 * because this function shall be modified for each game.
 *
 * @param  [ in]pQt The current quadtree
 */
err doCollide(gfmQuadtreeRoot *pQt) {
    /** GFraMe return value */
    gfmRV rv;

    /* Continue colliding until the quadtree finishes (or collision is
     * skipped) */
    rv = GFMRV_QUADTREE_OVERLAPED;
    collision.flags  &= ~CF_SKIP;
    while (rv != GFMRV_QUADTREE_DONE && !(collision.flags & CF_SKIP)) {
        collisionNode node1, node2;
        err erv;
        int isFirstCase;
        int fallthrough;

        /* Retrieve the two overlaping objects and their types */
        rv = gfmQuadtree_getOverlaping(&node1.pObject, &node2.pObject
                , pQt);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        _getSubtype(&node1);
        _getSubtype(&node2);

        /* Merge both types into a single one, so it's easier to compare */
        isFirstCase = 0;
        fallthrough = 0;

        /* Import the filtered collision tuples */
        #include <auto/collisioncases.c>

        /** Update the quadtree (so any other collision is detected) */
        rv = gfmQuadtree_continue(pQt);
        ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE,
                ERR_GFMERR);
    }

    return ERR_OK;
}

