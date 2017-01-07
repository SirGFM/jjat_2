/**
 * @file src/jjat2/enemies.c
 *
 * Initialize and update every enemy
 */
#include <base/error.h>
#include <base/game.h>

#include <conf/type.h>

#include <jjat2/enemy.h>
#include <jjat2/entity.h>

#include <jjat2/enemies/walky.h>
#include <jjat2/enemies/g_walky.h>

#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmParser.h>

/**
 * Parse an enemy into the entity
 *
 * @param  [ in]pEnt    The entity
 * @param  [ in]pParser Parser that has just parsed an enemy
 * @param  [ in]t       Type of the parsed enemy
 */
err parseEnemy(entityCtx *pEnt, gfmParser *pParser, type t) {
    gfmRV rv;
    err erv;
    int x, y;

    rv = gfmParser_getPos(&x, &y, pParser);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (t) {
        case T_WALKY:   erv = initWalky(pEnt, x, y); break;
        case T_G_WALKY: erv = initGreenWalky(pEnt, x, y); break;
        default: {
            ASSERT(0, ERR_INVALIDTYPE);
        }
    }
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Update an enemy's physics
 *
 * @param  [ in]pEnt    The entity
 */
err preUpdateEnemy(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_WALKY:   return preUpdateWalky(pEnt);
        case T_G_WALKY: return preUpdateGreenWalky(pEnt);
        default: {
            return ERR_INVALIDTYPE;
        }
    }
}

/**
 * Set the enemy's animation and fix its entity's collision.
 *
 * @param  [ in]pEnt    The entity
 */
err postUpdateEnemy(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_WALKY:   return postUpdateWalky(pEnt);
        case T_G_WALKY: return postUpdateGreenWalky(pEnt);
        default: {
            return ERR_INVALIDTYPE;
        }
    }
}

/**
 * Draw an enemy
 *
 * @param  [ in]pEnt    The entity
 */
err drawEnemy(entityCtx *pEnt) {
    void *pChild;
    int type;
    gfmRV rv;

    rv = gfmSprite_getChild(&pChild, &type, pEnt->pSelf);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    switch (type) {
        case T_WALKY:   return drawWalky(pEnt);
        case T_G_WALKY: return drawGreenWalky(pEnt);
        default: {
            return ERR_INVALIDTYPE;
        }
    }
}

