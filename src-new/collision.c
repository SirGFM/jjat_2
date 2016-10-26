/**
 * @file src/collision.c
 *
 * Declare only the collision function.
 */
#include <base/collision.h>
#include <base/error.h>
#include <conf/type.h>

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
        if (!fallthrough) isFirstCase = 1; \
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
 */
err doCollide() {
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
                , collision.pQt);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        _getSubtype(&node1);
        _getSubtype(&node2);

        /* Merge both types into a single one, so it's easier to compare */
        isFirstCase = 0;
        switch (MERGE_TYPES(node1.type, node2.type)) {
            /* TODO Handle collisions.
             *
             * Don't forget to 'break' after each CASE! Also, don't forget to
             * IGNORE any collision that isn't triggered! */
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
                            gfmObject_setPosition(player->pObject, x - 1, y - 1);
                        }
                        else if (dir & gfmCollision_right) {
                            int x, y;
                            gfmObject_getPosition(&x, &y, player->pObject);
                            gfmObject_setPosition(player->pObject, x + 1, y - 1);
                        }
                    }
                }
                rv = GFMRV_OK;
            } break;
            CASE(T_SWORDY, T_GUNNY) {
                collisionNode *swordy, *gunny;
                if (isFirstCase) {
                    swordy = &node1;
                    gunny = &node2;
                }
                else {
                    gunny = &node1;
                    swordy = &node2;
                }
                rv = gfmObject_justOverlaped(node1.pObject, node2.pObject);
                if (rv == GFMRV_TRUE) {
                    gfmCollision dir;
                    gfmObject_getCurrentCollision(&dir, swordy->pObject);
                    if (dir & gfmCollision_down) {
                        /* TODO Swordy is above gunny */
                        gfmObject_setFixed(gunny->pObject);
                        gfmObject_separateVertical(node1.pObject, node2.pObject);
                        gfmObject_setVerticalVelocity(swordy->pObject, 0);
                        gfmObject_setMovable(gunny->pObject);
                    }
                    gfmObject_getCurrentCollision(&dir, gunny->pObject);
                    if (dir & gfmCollision_down) {
                        /* TODO Gunny is above swordy */
                        gfmObject_setFixed(swordy->pObject);
                        gfmObject_separateVertical(node1.pObject, node2.pObject);
                        gfmObject_setVerticalVelocity(gunny->pObject, 0);
                        gfmObject_setMovable(swordy->pObject);
                    }
                }
                rv = GFMRV_OK;
            } break;
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
        rv = gfmQuadtree_continue(collision.pQt);
        ASSERT(rv == GFMRV_QUADTREE_OVERLAPED || rv == GFMRV_QUADTREE_DONE,
                ERR_GFMERR);
    }

    return ERR_OK;
}

