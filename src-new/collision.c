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
#include <jjat2/swordy.h>
#include <jjat2/teleport.h>

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
    collision.skip = 0;
    while (rv != GFMRV_QUADTREE_DONE && !collision.skip) {
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
            CASE(T_FLOOR, T_GUNNY)
            CASE(T_FLOOR, T_SWORDY) {
                collisionNode *player;
                if (isFirstCase) {
                    player = &node2;
                }
                else {
                    player = &node1;
                }
                rv = gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (rv == GFMRV_TRUE) {
                    gfmCollision dir;
                    gfmObject_getCurrentCollision(&dir, player->pObject);
                    gfmObject_collide(node1.pObject, node2.pObject);
                    if (dir & gfmCollision_down) {
                        gfmObject_setVerticalVelocity(player->pObject, 0);
                        /* Corner case!! If the player would get stuck on a
                         * corner, push 'em toward the platform */
                        if (dir & gfmCollision_left) {
                            int x, y;
                            gfmObject_getPosition(&x, &y, player->pObject);
                            gfmObject_setPosition(player->pObject, x - 1
                                    , y - 1);
                        }
                        else if (dir & gfmCollision_right) {
                            int x, y;
                            gfmObject_getPosition(&x, &y, player->pObject);
                            gfmObject_setPosition(player->pObject, x + 1
                                    , y - 1);
                        }
                    }
                    else if (dir & gfmCollision_up) {
                        int y;
                        gfmObject_setVerticalVelocity(player->pObject, 0);
                        gfmObject_getVerticalPosition(&y, player->pObject);
                        gfmObject_setVerticalPosition(player->pObject, y + 1);
                    }
                }
                rv = GFMRV_OK;
            } break;
            CASE(T_SWORDY, T_GUNNY) {
                swordyCtx *swordy;
                gunnyCtx *gunny;
                if (isFirstCase) {
                    swordy = (swordyCtx*)node1.pChild;
                    gunny = (gunnyCtx*)node2.pChild;
                }
                else {
                    gunny = (gunnyCtx*)node1.pChild;
                    swordy = (swordyCtx*)node2.pChild;
                }

                rv = gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (rv == GFMRV_TRUE) {
                    gfmCollision sdir, gdir;
                    gfmSprite_getCurrentCollision(&sdir, swordy->entity.pSelf);
                    gfmSprite_getCurrentCollision(&gdir, gunny->entity.pSelf);
                    if (sdir & gfmCollision_down) {
                        /* Swordy is above gunny */
                        carryEntity(&swordy->entity, gunny->entity.pSelf);
                    }
                    else if (gdir & gfmCollision_down) {
                        /* Gunny is above swordy */
                        carryEntity(&gunny->entity, swordy->entity.pSelf);
                    }
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

                collision.skip = 1;
            } break;
            IGNORE(T_ATK_SWORD, T_SWORDY)
            IGNORE(T_ATK_SWORD, T_GUNNY)
            IGNORE(T_ATK_SWORD, T_FLOOR)
            IGNORE(T_ATK_SWORD, T_FX)
            IGNORESELF(T_ATK_SWORD)
            break;
/*== GUNNY'S BULLET ==========================================================*/
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
                if (GFMRV_TRUE == gfm_isSpriteInsideCamera(game.pCtx, pEntity->pSelf)) {
                    erv = teleporterTargetEntity(pEntity);
                    ASSERT(erv == ERR_OK, erv);
                }
                rv = gfmGroup_removeNode(pNode);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);

                collision.skip = 1;
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

                    /* Since the bullet only moves horizontally, use the floor's
                     * horizontal position and the bullet's vertical */
                    rv = gfmObject_getLastCenter(&cx, &y, pBullet);
                    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
                    rv = gfmObject_getHorizontalPosition(&x, pFloor);
                    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

                    /* Ugly, hacky assumption: if the center - half the width is
                     * to the right of the floor's left side, then it's to the
                     * floor's right */
                    if (cx - 8 >= x) {
                        int w;
                        /* The bullet collided to the left, so adjust its position */
                        rv = gfmObject_getWidth(&w, pFloor);
                        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
                        x += w;
                    }

                    erv = teleporterTargetPosition(x, y);
                    ASSERT(erv == ERR_OK, erv);
                }
                rv = gfmGroup_removeNode(pNode);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);
                collision.skip = 1;
            } break;
            IGNORE(T_TEL_BULLET, T_GUNNY)
            IGNORE(T_TEL_BULLET, T_FX)
            IGNORESELF(T_TEL_BULLET)
            break;
/*== SWORDY'S ATTACK TRAIL (AFTER HITTING ANYTHING) ==========================*/
            IGNORE(T_SWORD_FX, T_SWORDY)
            IGNORE(T_SWORD_FX, T_GUNNY)
            IGNORE(T_SWORD_FX, T_FLOOR)
            IGNORE(T_SWORD_FX, T_FX)
            IGNORE(T_SWORD_FX, T_ATK_SWORD)
            IGNORE(T_SWORD_FX, T_TEL_BULLET)
            IGNORESELF(T_SWORD_FX)
            break;
/*== COLLISION-LESS EFFECTS ==================================================*/
            IGNORE(T_FX, T_SWORDY)
            IGNORE(T_FX, T_GUNNY)
            IGNORE(T_FX, T_FLOOR)
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

