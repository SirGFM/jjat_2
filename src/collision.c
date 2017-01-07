/**
 * @file src/collision.c
 *
 * Declare only the collision function.
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <conf/type.h>

#include <jjat2/entity.h>
#include <jjat2/gunny.h>
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
/*== PLAYER'S COLLISION ======================================================*/
            CASE(T_FLOOR, T_WALKY)
            CASE(T_FLOOR, T_G_WALKY)
            CASE(T_FLOOR, T_GUNNY)
            CASE(T_FLOOR, T_SWORDY)
            CASE(T_FLOOR_NOTP, T_WALKY)
            CASE(T_FLOOR_NOTP, T_G_WALKY)
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
                    collisionNode *floor;
                    int fy, py, h;

                    /* Entity was (possibly) just teleported into the ground.
                     * Manually check and fix it */
                    if (isFirstCase) {
                        floor = &node1;
                    }
                    else {
                        floor = &node2;
                    }

                    gfmObject_getVerticalPosition(&fy, floor->pObject);
                    gfmObject_getVerticalPosition(&py, entity->pObject);
                    gfmObject_getHeight(&h, entity->pObject);

                    if (py + h >= fy) {
                        gfmObject_setVerticalPosition(entity->pObject, fy - h);
                    }
                }
#endif  /* JJATENGINE */
                rv = GFMRV_OK;
            } break;
            CASE(T_SPIKE, T_WALKY)
            CASE(T_SPIKE, T_G_WALKY)
            CASE(T_SPIKE, T_GUNNY)
            CASE(T_SPIKE, T_SWORDY) {
                collisionNode *entity;
                collisionNode *spike;
                gfmCollision hdir, vdir;

                if (isFirstCase) {
                    entity = &node2;
                    spike = &node1;
                }
                else {
                    entity = &node1;
                    spike = &node2;
                }

                gfmObject_isOverlaping(node1.pObject, node2.pObject);
                gfmObject_getCurrentCollision(&vdir, entity->pObject);
                gfmObject_justOverlaped(node1.pObject, node2.pObject);
                gfmObject_getCurrentCollision(&hdir, entity->pObject);

                if (hdir & gfmCollision_hor) {
                    /* Collide horizontally to avoid clipping */
                    gfmObject_separateHorizontal(node1.pObject
                            , node2.pObject);
                }
                else if (vdir & gfmCollision_down) {
                    int py, sy, ph;

                    gfmObject_getVerticalPosition(&py, entity->pObject);
                    gfmObject_getHeight(&ph, entity->pObject);
                    gfmObject_getVerticalPosition(&sy, spike->pObject);

                    if (py + ph >= sy + SPIKE_OFFSET) {
                        /* Kill the entity */
                        ((entityCtx*)entity->pChild)->flags &= ~EF_ALIVE;
                    }
                }

                rv = GFMRV_OK;
            } break;
/*== ENTITIES'S COLLISION ====================================================*/
            SELFCASE(T_G_WALKY)
            SELFCASE(T_WALKY)
            CASE(T_WALKY, T_G_WALKY)
            CASE(T_SWORDY, T_G_WALKY)
            CASE(T_GUNNY, T_G_WALKY)
            CASE(T_SWORDY, T_WALKY)
            CASE(T_GUNNY, T_WALKY)
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
/*== SWORDY'S ATTACK =========================================================*/
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
            CASE(T_ATK_SWORD, T_WALKY) {
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

                walky->flags &= ~EF_ALIVE;
                gfmSprite_setType(pSword, T_SWORD_FX);

                collision.flags |= CF_SKIP;
            } break;
            CASE(T_ATK_SWORD, T_G_WALKY) {
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
                    gWalky->flags &= ~EF_ALIVE;
                    gfmSprite_setType(pSword->pSprite, T_SWORD_FX);

                    collision.flags |= CF_SKIP;
                }
            } break;
            IGNORE(T_ATK_SWORD, T_SWORDY)
            IGNORE(T_ATK_SWORD, T_GUNNY)
            IGNORE(T_ATK_SWORD, T_FLOOR)
            IGNORE(T_ATK_SWORD, T_FLOOR_NOTP)
            IGNORE(T_ATK_SWORD, T_FX)
            IGNORE(T_ATK_SWORD, T_SPIKE)
            IGNORESELF(T_ATK_SWORD)
            break;
/*== GUNNY'S BULLET ==========================================================*/
            CASE(T_TEL_BULLET, T_WALKY)
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
            CASE(T_TEL_BULLET, T_G_WALKY) {
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
/*== SWORDY'S ATTACK TRAIL (AFTER HITTING ANYTHING) ==========================*/
            IGNORE(T_SWORD_FX, T_G_WALKY)
            IGNORE(T_SWORD_FX, T_WALKY)
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
            IGNORE(T_FX, T_G_WALKY)
            IGNORE(T_FX, T_WALKY)
            IGNORE(T_FX, T_SWORDY)
            IGNORE(T_FX, T_GUNNY)
            IGNORE(T_FX, T_FLOOR)
            IGNORE(T_FX, T_FLOOR_NOTP)
            IGNORE(T_FX, T_SPIKE)
            IGNORESELF(T_FX)
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

