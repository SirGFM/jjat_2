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

/** Ignore collision with entities of the same type */
#define IGNORESELF(type) \
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
        switch (MERGE_TYPES(node1.type, node2.type)) {
/*== COLLISION AGAINST DUMMY =================================================*/
            CASE(T_SWORDY, T_DUMMY_GUNNY)
            CASE(T_GUNNY, T_DUMMY_SWORDY) {
                gfmObject *pPlayer, *pDummy;
                int playerY, dummyY;

                if (isFirstCase) {
                    pPlayer = node1.pObject;
                    pDummy = node2.pObject;
                }
                else {
                    pPlayer = node2.pObject;
                    pDummy = node1.pObject;
                }
                gfmObject_getVerticalPosition(&playerY, pPlayer);
                gfmObject_getVerticalPosition(&dummyY, pDummy);

                gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (playerY < dummyY) {
                    gfmCollision dir;

                    /* Only separate the player if it's above the dummy */
                    gfmObject_separateVertical(node1.pObject, node2.pObject);
                    gfmObject_getCurrentCollision(&dir, pPlayer);
                    if (dir & gfmCollision_down) {
                        gfmObject_setVerticalVelocity(pPlayer, 0);
                    }
                }
            } break;
/*== CHANGE MAP ==============================================================*/
            CASE(T_LOADZONE, T_DUMMY_SWORDY)
            CASE(T_LOADZONE, T_DUMMY_GUNNY)
            CASE(T_LOADZONE, T_GUNNY)
            CASE(T_LOADZONE, T_SWORDY) {
                /* Setup level transition for the playstate */
                if (isFirstCase) {
                    onHitLoadzone(node2.type,
                            (leveltransitionData*)node1.pChild);
                }
                else {
                    onHitLoadzone(node1.type,
                            (leveltransitionData*)node2.pChild);
                }
            } break;
/*== ENVIRONMENT'S COLLISION =================================================*/
            CASE(T_SPIKE, T_EN_TURRET)
            CASE(T_FLOOR, T_EN_SPIKY)
            CASE(T_FLOOR, T_EN_WALKY)
            CASE(T_FLOOR, T_EN_TURRET)
            CASE(T_FLOOR, T_EN_G_WALKY)
            CASE(T_FLOOR, T_GUNNY)
            CASE(T_FLOOR, T_SWORDY)
            CASE(T_FLOOR_NOTP, T_EN_SPIKY)
            CASE(T_FLOOR_NOTP, T_EN_WALKY)
            CASE(T_FLOOR_NOTP, T_EN_TURRET)
            CASE(T_FLOOR_NOTP, T_EN_G_WALKY)
            CASE(T_FLOOR_NOTP, T_GUNNY)
            CASE(T_FLOOR_NOTP, T_SWORDY) {
                collisionNode *entity;
                if (isFirstCase) {
                    entity = &node2;
                }
                else {
                    entity = &node1;
                }
                rv = gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (rv == GFMRV_TRUE) {
                    gfmCollision dir;

                    gfmObject_getCurrentCollision(&dir, entity->pObject);

                    if (!((dir & gfmCollision_up) && (dir & gfmCollision_hor))) {
                        gfmObject_collide(node1.pObject, node2.pObject);
                    }
                    else {
                        /* Fix colliding against corners when there are two
                         * separated objects in a wall */
                        gfmObject_separateHorizontal(node1.pObject
                                , node2.pObject);
                    }

                    if (dir & gfmCollision_down) {
                        gfmObject_setVerticalVelocity(entity->pObject, 0);
                        /* Corner case!! If the entity would get stuck on a
                         * corner, push 'em toward the platform */
                        if (dir & gfmCollision_left) {
                            int x, y;
                            gfmObject_getPosition(&x, &y, entity->pObject);
                            gfmObject_setPosition(entity->pObject, x - 1
                                    , y - 1);
                        }
                        else if (dir & gfmCollision_right) {
                            int x, y;
                            gfmObject_getPosition(&x, &y, entity->pObject);
                            gfmObject_setPosition(entity->pObject, x + 1
                                    , y - 1);
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
                    if (gfmObject_isOverlaping(node1.pObject, node2.pObject)
                            == GFMRV_TRUE) {
                        collisionNode *floor;
                        int fh, fy, ph, py;

                        if (isFirstCase) {
                            floor = &node1;
                        }
                        else {
                            floor = &node2;
                        }

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
                rv = GFMRV_OK;
            } break;
            CASE(T_SPIKE, T_EN_SPIKY)
            CASE(T_SPIKE, T_EN_WALKY)
            CASE(T_SPIKE, T_EN_G_WALKY)
            CASE(T_SPIKE, T_GUNNY)
            CASE(T_SPIKE, T_SWORDY) {
                collisionNode *entity;
                collisionNode *spike;
                gfmCollision dir;
                int py, sy, ph;

                if (isFirstCase) {
                    entity = &node2;
                    spike = &node1;
                }
                else {
                    entity = &node1;
                    spike = &node2;
                }

                gfmObject_justOverlaped(node1.pObject, node2.pObject);
                gfmObject_getCurrentCollision(&dir, entity->pObject);

                gfmObject_getVerticalPosition(&py, entity->pObject);
                gfmObject_getHeight(&ph, entity->pObject);
                gfmObject_getVerticalPosition(&sy, spike->pObject);

                if (py + ph < sy + SPIKE_OFFSET) {
                    /* Does nothing unless within the collideable aread */
                }
                else if (dir & gfmCollision_hor) {
                    /* Collide horizontally to avoid clipping */
                    gfmObject_separateHorizontal(node1.pObject
                            , node2.pObject);
                }
                else {
                    /* Kill the entity */
                    killEntity((entityCtx*)entity->pChild);
                }

                rv = GFMRV_OK;
            } break;
            CASE(T_FLOOR_NOTP, T_EN_G_WALKY_ATK)
            CASE(T_FLOOR, T_EN_G_WALKY_ATK) {
                if (isFirstCase) {
                    _explodeStar(&node2);
                }
                else {
                    _explodeStar(&node1);
                }
                rv = GFMRV_OK;
                collision.flags |= CF_SKIP;
            } break;
            CASE(T_CHECKPOINT, T_GUNNY)
            CASE(T_CHECKPOINT, T_SWORDY) {
                collisionNode *checkpoint;
                gfmSprite *pSpr;
                int h, w, x, y;
                err erv;

                if (isFirstCase) {
                    checkpoint = &node1;
                }
                else {
                    checkpoint = &node2;
                }

                gfmObject_getPosition(&x, &y, checkpoint->pObject);
                gfmObject_getDimensions(&w, &h, checkpoint->pObject);

                /* Set the checkpoint */
                erv = setCheckpoint((leveltransitionData*)checkpoint->pChild);
                ASSERT(erv == ERR_OK, erv);

                pSpr = spawnFx(x, y, w, h, 0/*dir*/, checkpointSavedDuration
                        , FX_CHECKPOINT_SAVED, T_FX);
                ASSERT(pSpr != 0, ERR_BUFFERTOOSMALL);

                gfmObject_setType(checkpoint->pObject, T_FX);
            } break;
/*== ENTITIES'S COLLISION ====================================================*/
            SELFCASE(T_EN_SPIKY)
            SELFCASE(T_EN_WALKY)
            SELFCASE(T_EN_G_WALKY)
            SELFCASE(T_EN_TURRET)
            CASE(T_EN_SPIKY, T_EN_TURRET)
            CASE(T_EN_G_WALKY, T_EN_TURRET)
            CASE(T_EN_WALKY, T_EN_TURRET)
            CASE(T_SWORDY, T_EN_TURRET)
            CASE(T_GUNNY, T_EN_TURRET)
            CASE(T_EN_G_WALKY, T_EN_SPIKY)
            CASE(T_EN_WALKY, T_EN_G_WALKY)
            CASE(T_SWORDY, T_EN_G_WALKY)
            CASE(T_GUNNY, T_EN_G_WALKY)
            CASE(T_SWORDY, T_EN_WALKY)
            CASE(T_GUNNY, T_EN_WALKY)
            CASE(T_SWORDY, T_GUNNY) {
                entityCtx *entA;
                entityCtx *entB;

                entA = (entityCtx*)node1.pChild;
                entB = (entityCtx*)node2.pChild;

                if (entA != entB) {
                    collideTwoEntities(entA, entB);
                }
                rv = GFMRV_OK;
            } break;
            CASE(T_EN_WALKY, T_EN_SPIKY)
            CASE(T_SWORDY, T_EN_SPIKY)
            CASE(T_GUNNY, T_EN_SPIKY) {
                entityCtx *pPlayer, *pEnemy;
                int damage;

                if (isFirstCase) {
                    pPlayer = (entityCtx*)node1.pChild;
                    pEnemy = (entityCtx*)node2.pChild;
                    damage = (node2.type >> T_BITS);
                }
                else {
                    pPlayer = (entityCtx*)node2.pChild;
                    pEnemy = (entityCtx*)node1.pChild;
                    damage = (node1.type >> T_BITS);
                }

                rv = gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (rv == GFMRV_TRUE) {
                    gfmCollision col;
                    int dir;

                    gfmSprite_getCollision(&col, pEnemy->pSelf);
                    gfmSprite_getDirection(&dir, pEnemy->pSelf);

                    if ((dir == DIR_RIGHT && (col & gfmCollision_right))
                            || (dir == DIR_LEFT && (col & gfmCollision_left))) {
                        hitEntity(pPlayer, damage);
                        collision.flags |= CF_SKIP;
                    }
                }
                rv = GFMRV_OK;
            } break;
            CASE(T_EN_G_WALKY_VIEW, T_SWORDY)
            CASE(T_EN_G_WALKY_VIEW, T_GUNNY)
            CASE(T_EN_G_WALKY_VIEW, T_EN_WALKY) {
                if (isFirstCase) {
                    triggerGreenWalkyAttack((entityCtx*)node1.pChild);
                }
                else {
                    triggerGreenWalkyAttack((entityCtx*)node2.pChild);
                }
            } break;
/*== SWORDY'S ATTACK =========================================================*/
            CASE(T_ATK_SWORD, T_EN_G_WALKY_ATK)
            CASE(T_ATK_SWORD, T_TEL_BULLET) {
                /* Reflect the bullet */
                gfmSprite *pBullet;
                double vx;
                int dir;

                if (isFirstCase) {
                    pBullet = node2.pSprite;
                    gfmSprite_setType(node1.pSprite, T_SWORD_FX);
                }
                else {
                    pBullet = node1.pSprite;
                    gfmSprite_setType(node2.pSprite, T_SWORD_FX);
                }
                gfmSprite_getHorizontalVelocity(&vx, pBullet);
                gfmSprite_getDirection(&dir, pBullet);
                gfmSprite_setHorizontalVelocity(pBullet, -vx);
                gfmSprite_setDirection(pBullet, !dir);

                collision.flags |= CF_SKIP;
            } break;
            CASE(T_ATK_SWORD, T_EN_SPIKY)
            CASE(T_ATK_SWORD, T_EN_WALKY) {
                gfmSprite *pSword;
                entityCtx *walky;

                if (isFirstCase) {
                    walky = (entityCtx*)node2.pChild;
                    pSword = node1.pSprite;
                }
                else {
                    walky = (entityCtx*)node1.pChild;
                    pSword = node2.pSprite;
                }

                killEntity(walky);
                gfmSprite_setType(pSword, T_SWORD_FX);

                collision.flags |= CF_SKIP;
            } break;
            CASE(T_ATK_SWORD, T_EN_G_WALKY) {
                collisionNode *pSword;
                entityCtx *gWalky;

                if (isFirstCase) {
                    gWalky = (entityCtx*)node2.pChild;
                    pSword = &node1;
                }
                else {
                    gWalky = (entityCtx*)node1.pChild;
                    pSword = &node2;
                }

                gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (onGreenWalkyAttacked(gWalky, pSword->pObject) == ERR_OK) {
                    killEntity(gWalky);
                    gfmSprite_setType(pSword->pSprite, T_SWORD_FX);

                    collision.flags |= CF_SKIP;
                }
            } break;
            IGNORE(T_ATK_SWORD, T_SWORDY)
            IGNORE(T_ATK_SWORD, T_GUNNY)
            IGNORE(T_ATK_SWORD, T_FLOOR)
            IGNORE(T_ATK_SWORD, T_FLOOR_NOTP)
            IGNORE(T_ATK_SWORD, T_FX)
            IGNORE(T_ATK_SWORD, T_EN_TURRET)
            IGNORE(T_ATK_SWORD, T_SPIKE)
            IGNORESELF(T_ATK_SWORD)
            break;
/*== GUNNY'S BULLET ==========================================================*/
            CASE(T_TEL_BULLET, T_EN_SPIKY)
            CASE(T_TEL_BULLET, T_EN_WALKY)
            CASE(T_TEL_BULLET, T_SWORDY) {
                gfmGroupNode *pNode;
                entityCtx *pEntity;
                err erv;

                if (isFirstCase) {
                    pNode = (gfmGroupNode*)node1.pChild;
                    pEntity = (entityCtx*)node2.pChild;
                }
                else {
                    pNode = (gfmGroupNode*)node2.pChild;
                    pEntity = (entityCtx*)node1.pChild;
                }

                /* Check if visible */
                if (GFMRV_TRUE == gfm_isSpriteInsideCamera(game.pCtx
                            , pEntity->pSelf)) {
                    erv = teleporterTargetEntity(pEntity);
                    ASSERT(erv == ERR_OK, erv);
                }
                rv = gfmGroup_removeNode(pNode);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);

                collision.flags |= CF_SKIP;
            } break;
            CASE(T_TEL_BULLET, T_EN_G_WALKY) {
                collisionNode *pNode;
                entityCtx *pGWalky;
                err erv;

                if (isFirstCase) {
                    pNode = &node1;
                    pGWalky = (entityCtx*)node2.pChild;
                }
                else {
                    pNode = &node2;
                    pGWalky = (entityCtx*)node1.pChild;
                }

                gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (onGreenWalkyAttacked(pGWalky, pNode->pObject) == ERR_OK) {
                    /* Check if visible */
                    if (GFMRV_TRUE == gfm_isSpriteInsideCamera(game.pCtx
                                , pGWalky->pSelf)) {
                        erv = teleporterTargetEntity(pGWalky);
                        ASSERT(erv == ERR_OK, erv);
                    }
                    rv = gfmGroup_removeNode((gfmGroupNode*)pNode->pChild);
                    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
                }

                collision.flags |= CF_SKIP;
            } break;
            CASE(T_TEL_BULLET, T_FLOOR) {
                gfmGroupNode *pNode;
                gfmObject *pBullet, *pFloor;
                err erv;

                if (isFirstCase) {
                    pBullet = node1.pObject;
                    pNode = (gfmGroupNode*)node1.pChild;
                    pFloor = node2.pObject;
                }
                else {
                    pBullet = node2.pObject;
                    pNode = (gfmGroupNode*)node2.pChild;
                    pFloor = node1.pObject;
                }

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
            } break;
            CASE(T_TEL_BULLET, T_EN_TURRET)
            CASE(T_TEL_BULLET, T_SPIKE)
            CASE(T_TEL_BULLET, T_FLOOR_NOTP) {
                gfmGroupNode *pNode;

                if (isFirstCase) {
                    pNode = (gfmGroupNode*)node1.pChild;
                }
                else {
                    pNode = (gfmGroupNode*)node2.pChild;
                }

                /* TODO Play vanish animation */

                rv = gfmGroup_removeNode(pNode);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);
                collision.flags |= CF_SKIP;
            } break;
            IGNORE(T_TEL_BULLET, T_GUNNY)
            IGNORE(T_TEL_BULLET, T_FX)
            IGNORESELF(T_TEL_BULLET)
            break;
/*== DAMAGE TO ENTITY ========================================================*/
            CASE(T_SWORDY, T_EN_G_WALKY_ATK)
            CASE(T_GUNNY, T_EN_G_WALKY_ATK)
            CASE(T_EN_SPIKY, T_EN_G_WALKY_ATK)
            CASE(T_EN_WALKY, T_EN_G_WALKY_ATK) {
                entityCtx *pEnt;
                int damage;

                /* Ugly hack: the damage should be stored on the unused part of
                 * the type */
                if (isFirstCase) {
                    pEnt = (entityCtx*)node1.pChild;
                    damage = node2.type;
                    _explodeStar(&node2);
                }
                else {
                    pEnt = (entityCtx*)node2.pChild;
                    damage = node1.type;
                    _explodeStar(&node1);
                }

                damage >>= T_BITS;
                hitEntity(pEnt, damage);
            } break;
            CASE(T_EN_G_WALKY, T_EN_G_WALKY_ATK) {
                entityCtx *pEnt;
                gfmObject *pObject;
                int damage;

                /* Ugly hack: the damage should be stored on the unused part of
                 * the type */
                if (isFirstCase) {
                    pEnt = (entityCtx*)node1.pChild;
                    pObject = node2.pObject;
                    damage = node2.type;
                    _explodeStar(&node2);
                }
                else {
                    pEnt = (entityCtx*)node2.pChild;
                    pObject = node1.pObject;
                    damage = node1.type;
                    _explodeStar(&node1);
                }

                if (onGreenWalkyAttacked(pEnt, pObject) == ERR_OK) {
                    damage >>= T_BITS;
                    hitEntity(pEnt, damage);
                }
            } break;
            CASE(T_EN_TURRET, T_EN_G_WALKY_ATK) {
                if (isFirstCase) {
                    _explodeStar(&node2);
                }
                else {
                    _explodeStar(&node1);
                }
            } break;
/*== SWORDY'S ATTACK TRAIL (AFTER HITTING ANYTHING) ==========================*/
            IGNORE(T_SWORD_FX, T_EN_SPIKY)
            IGNORE(T_SWORD_FX, T_EN_WALKY)
            IGNORE(T_SWORD_FX, T_EN_G_WALKY)
            IGNORE(T_SWORD_FX, T_EN_G_WALKY_ATK)
            IGNORE(T_SWORD_FX, T_EN_G_WALKY_VIEW)
            IGNORE(T_SWORD_FX, T_EN_TURRET)
            IGNORE(T_SWORD_FX, T_SWORDY)
            IGNORE(T_SWORD_FX, T_GUNNY)
            IGNORE(T_SWORD_FX, T_FLOOR)
            IGNORE(T_SWORD_FX, T_FLOOR_NOTP)
            IGNORE(T_SWORD_FX, T_FX)
            IGNORE(T_SWORD_FX, T_ATK_SWORD)
            IGNORE(T_SWORD_FX, T_TEL_BULLET)
            IGNORE(T_SWORD_FX, T_SPIKE)
            IGNORESELF(T_SWORD_FX)
            break;
/*== COLLISION-LESS EFFECTS ==================================================*/
            IGNORE(T_EN_G_WALKY_VIEW, T_FX)
            IGNORE(T_EN_G_WALKY_VIEW, T_ATK_SWORD)
            IGNORE(T_EN_G_WALKY_VIEW, T_TEL_BULLET)
            IGNORE(T_EN_G_WALKY_VIEW, T_EN_G_WALKY_ATK)
            IGNORE(T_EN_G_WALKY_VIEW, T_EN_G_WALKY)
            IGNORE(T_EN_G_WALKY_VIEW, T_EN_SPIKY)
            IGNORE(T_EN_G_WALKY_VIEW, T_FLOOR)
            IGNORE(T_EN_G_WALKY_VIEW, T_FLOOR_NOTP)
            IGNORE(T_EN_G_WALKY_VIEW, T_SPIKE)
            IGNORESELF(T_EN_G_WALKY_VIEW)
            IGNORE(T_CHECKPOINT, T_FLOOR)
            IGNORE(T_CHECKPOINT, T_FLOOR_NOTP)
            IGNORE(T_CHECKPOINT, T_TEL_BULLET)
            IGNORE(T_CHECKPOINT, T_ATK_SWORD)
            IGNORE(T_CHECKPOINT, T_SWORD_FX)
            IGNORE(T_CHECKPOINT, T_FX)
            IGNORE(T_CHECKPOINT, T_EN_G_WALKY_ATK)
            IGNORE(T_CHECKPOINT, T_EN_G_WALKY)
            IGNORE(T_CHECKPOINT, T_EN_WALKY)
            IGNORE(T_CHECKPOINT, T_EN_SPIKY)
            IGNORE(T_EN_G_WALKY_ATK, T_SPIKE)
            IGNORE(T_EN_G_WALKY_ATK, T_TEL_BULLET)
            IGNORE(T_LOADZONE, T_FX)
            IGNORE(T_LOADZONE, T_SWORD_FX)
            IGNORE(T_LOADZONE, T_ATK_SWORD)
            IGNORE(T_LOADZONE, T_TEL_BULLET)
            IGNORE(T_LOADZONE, T_EN_G_WALKY_ATK)
            IGNORE(T_LOADZONE, T_EN_G_WALKY)
            IGNORE(T_LOADZONE, T_EN_WALKY)
            IGNORE(T_LOADZONE, T_EN_SPIKY)
            IGNORE(T_LOADZONE, T_FLOOR)
            IGNORE(T_LOADZONE, T_FLOOR_NOTP)
            IGNORE(T_LOADZONE, T_SPIKE)
            IGNORE(T_FX, T_EN_G_WALKY_ATK)
            IGNORE(T_FX, T_EN_G_WALKY)
            IGNORE(T_FX, T_EN_WALKY)
            IGNORE(T_FX, T_EN_SPIKY)
            IGNORE(T_FX, T_EN_TURRET)
            IGNORE(T_FX, T_SWORDY)
            IGNORE(T_FX, T_GUNNY)
            IGNORE(T_FX, T_FLOOR)
            IGNORE(T_FX, T_FLOOR_NOTP)
            IGNORE(T_FX, T_SPIKE)
            IGNORESELF(T_FX)
            IGNORESELF(T_EN_G_WALKY_ATK)

            IGNORE(T_HAZARD, T_DUMMY_SWORDY)
            IGNORE(T_PLAYER, T_DUMMY_SWORDY)
            IGNORE(T_FLOOR, T_DUMMY_SWORDY)
            IGNORE(T_ENEMY, T_DUMMY_SWORDY)
            IGNORE(T_FX, T_DUMMY_SWORDY)
            IGNORE(T_FLOOR_NOTP, T_DUMMY_SWORDY)
            IGNORE(T_ATK_SWORD, T_DUMMY_SWORDY)
            IGNORE(T_TEL_BULLET, T_DUMMY_SWORDY)
            IGNORE(T_CHECKPOINT, T_DUMMY_SWORDY)
            IGNORE(T_SPIKE, T_DUMMY_SWORDY)
            IGNORE(T_EN_WALKY, T_DUMMY_SWORDY)
            IGNORE(T_EN_G_WALKY, T_DUMMY_SWORDY)
            IGNORE(T_EN_G_WALKY_ATK, T_DUMMY_SWORDY)
            IGNORE(T_EN_G_WALKY_VIEW, T_DUMMY_SWORDY)
            IGNORE(T_EN_SPIKY, T_DUMMY_SWORDY)
            IGNORE(T_EN_TURRET, T_DUMMY_SWORDY)
            IGNORE(T_SWORD_FX, T_DUMMY_SWORDY)

            IGNORE(T_HAZARD, T_DUMMY_GUNNY)
            IGNORE(T_PLAYER, T_DUMMY_GUNNY)
            IGNORE(T_FLOOR, T_DUMMY_GUNNY)
            IGNORE(T_ENEMY, T_DUMMY_GUNNY)
            IGNORE(T_FX, T_DUMMY_GUNNY)
            IGNORE(T_FLOOR_NOTP, T_DUMMY_GUNNY)
            IGNORE(T_ATK_SWORD, T_DUMMY_GUNNY)
            IGNORE(T_TEL_BULLET, T_DUMMY_GUNNY)
            IGNORE(T_CHECKPOINT, T_DUMMY_GUNNY)
            IGNORE(T_SPIKE, T_DUMMY_GUNNY)
            IGNORE(T_EN_WALKY, T_DUMMY_GUNNY)
            IGNORE(T_EN_G_WALKY, T_DUMMY_GUNNY)
            IGNORE(T_EN_G_WALKY_ATK, T_DUMMY_GUNNY)
            IGNORE(T_EN_G_WALKY_VIEW, T_DUMMY_GUNNY)
            IGNORE(T_EN_SPIKY, T_DUMMY_GUNNY)
            IGNORE(T_EN_TURRET, T_DUMMY_GUNNY)
            IGNORE(T_SWORD_FX, T_DUMMY_GUNNY)

            break;
            /* On Linux, a SIGINT is raised any time a unhandled collision
             * happens. When debugging, GDB will stop here and allow the user to
             * check which types weren't handled */
            default: {
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
                /* Unfiltered collision, do something about it */
                raise(SIGINT);
                rv = GFMRV_INTERNAL_ERROR;
#  endif
            }
        } /* switch (MERGE_TYPES(node1.type, node2.type)) */
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        /** Update the quadtree (so any other collision is detected) */
        rv = gfmQuadtree_continue(pQt);
        ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE,
                ERR_GFMERR);
    }

    return ERR_OK;
}

