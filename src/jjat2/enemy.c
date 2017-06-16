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
#include <jjat2/enemies/spiky.h>
#include <jjat2/enemies/turret.h>

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
    err erv;

    switch (t & T_MASK) {
        case T_EN_SPIKY:   erv = initSpiky(pEnt, pParser); break;
        case T_EN_WALKY:   erv = initWalky(pEnt, pParser); break;
        case T_EN_G_WALKY: erv = initGreenWalky(pEnt, pParser); break;
        case T_EN_TURRET: erv = initTurret(pEnt, pParser); break;
        default: {
            ASSERT(0, ERR_INVALIDTYPE);
        }
    }
    ASSERT(erv == ERR_OK, erv);

    pEnt->baseType = (t & T_BASE_MASK);

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

    switch (type & T_MASK) {
        case T_EN_SPIKY:   return preUpdateSpiky(pEnt);
        case T_EN_WALKY:   return preUpdateWalky(pEnt);
        case T_EN_G_WALKY: return preUpdateGreenWalky(pEnt);
        case T_EN_TURRET: return preUpdateTurret(pEnt);
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

    switch (type & T_MASK) {
        case T_EN_SPIKY:   return postUpdateSpiky(pEnt);
        case T_EN_WALKY:   return postUpdateWalky(pEnt);
        case T_EN_G_WALKY: return postUpdateGreenWalky(pEnt);
        case T_EN_TURRET: return postUpdateTurret(pEnt);
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

    switch (type & T_MASK) {
        case T_EN_SPIKY:   return drawSpiky(pEnt);
        case T_EN_WALKY:   return drawWalky(pEnt);
        case T_EN_G_WALKY: return drawGreenWalky(pEnt);
        case T_EN_TURRET: return drawTurret(pEnt);
        default: {
            return ERR_INVALIDTYPE;
        }
    }
}

