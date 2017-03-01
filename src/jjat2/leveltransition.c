/**
 * @file src/jjat2/leveltransition.c
 *
 * Display and hide the transition screen and tween the player to its new
 * position.
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>

#include <conf/game.h>
#include <conf/type.h>

#include <jjat2/gunny.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/swordy.h>

#include <GFraMe/gfmCamera.h>
#include <GFraMe/gfmHitbox.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>
#include <GFraMe/gfmTilemap.h>

#include <string.h>

#define TRANSITION_TIME 750
#define WIDTH_IN_TILES  (V_WIDTH / 8 + 4)
#define HEIGHT_IN_TILES (V_HEIGHT / 8 + 4)
#define TM_DEFAULT_TILE -1

/** Forward declaration of the transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES];

#define TWEEN(var_op, init_time) \
    do { \
        var_op ( ((TRANSITION_TIME - (lvltransition.timer - (init_time))) \
                * (START_POS)) / TRANSITION_TIME) \
            + ( ((lvltransition.timer - (init_time)) * (END_POS)) \
                / TRANSITION_TIME); \
    } while (0)

/**
 * Tween the tilemap INTO the screen
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 */
static void _tweenTilemapIn(int cx, int cy) {
    switch (lvltransition.dir) {
        case TEL_UP: {
            cx -= 16;
#define START_POS (V_HEIGHT)
#define END_POS   (-16)
            TWEEN(cy +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_DOWN: {
            cx -= 16;
#define START_POS (-HEIGHT_IN_TILES * 8)
#define END_POS   (-16)
            TWEEN(cy +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_LEFT: {
            cy -= 16;
#define START_POS (V_WIDTH)
#define END_POS   (-16)
            TWEEN(cx +=, 0);
#undef START_POS
#undef END_POS
        } break;
        case TEL_RIGHT: {
            cy -= 16;
#define START_POS (-WIDTH_IN_TILES * 8)
#define END_POS   (-16)
            TWEEN(cx +=, 0);
#undef START_POS
#undef END_POS
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

/**
 * Tween a sprite in screen space toward the next position (coverted to screen
 * space)
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 * @param  [ in]initTime Time when the tween started
 */
static void _tweenPlayers(int cx, int cy, int initTime) {
    int dstX, dstY, srcX, srcY, tgtX, tgtY;

    /* Adjust the target position to be within the current camera */
    tgtX = ((int)lvltransition.tgtX) + cx;
    tgtY = ((int)lvltransition.tgtY) + cy;

    srcX = lvltransition.swordyX;
    srcY = lvltransition.swordyY;
#define START_POS (srcX)
#define END_POS   (tgtX)
            TWEEN(dstX =, initTime);
#undef START_POS
#undef END_POS
#define START_POS (srcY)
#define END_POS   (tgtY)
            TWEEN(dstY =, initTime);
#undef START_POS
#undef END_POS
    setSwordyPositionFromParser(&playstate.swordy, dstX, dstY);

    srcX = lvltransition.gunnyX;
    srcY = lvltransition.gunnyY;
#define START_POS (srcX)
#define END_POS   (tgtX)
            TWEEN(dstX =, initTime);
#undef START_POS
#undef END_POS
#define START_POS (srcY)
#define END_POS   (tgtY)
            TWEEN(dstY =, initTime);
#undef START_POS
#undef END_POS
    setGunnyPositionFromParser(&playstate.gunny, dstX, dstY);
}

/** Set both sprites position at the next position in world space */
static void _setPlayersPosition() {
    int tgtX, tgtY;

    /* Adjust the target position to be within the current camera */
    tgtX = lvltransition.tgtX;
    tgtY = lvltransition.tgtY;

    setSwordyPositionFromParser(&playstate.swordy, tgtX, tgtY);
    setGunnyPositionFromParser(&playstate.gunny, tgtX, tgtY);
}

/**
 * Tween the tilemap OUTSIDE the screen
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 */
static void _tweenTilemapOut(int cx, int cy) {
    switch (lvltransition.dir) {
        case TEL_UP: {
            cx -= 16;
#define START_POS (-16)
#define END_POS   (-HEIGHT_IN_TILES * 8)
            TWEEN(cy +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_DOWN: {
            cx -= 16;
#define START_POS (-16)
#define END_POS   (V_HEIGHT)
            TWEEN(cy +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_LEFT: {
            cy -= 16;
#define START_POS (-16)
#define END_POS   (-WIDTH_IN_TILES * 8)
            TWEEN(cx +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
        case TEL_RIGHT: {
            cy -= 16;
#define START_POS (-16)
#define END_POS   (V_WIDTH)
            TWEEN(cx +=, 2 * TRANSITION_TIME);
#undef START_POS
#undef END_POS
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

#undef TWEEN

/**
 * Prepare switching to a level transition
 *
 * @param  [ in]pNextLevel Next level data
 */
void switchToLevelTransition(leveltransitionData *pNextLevel) {
    lvltransition.pNextLevel = pNextLevel;
    game.nextState = ST_LEVELTRANSITION;
}

/** Alloc all required resources */
err initLeveltransition() {
    gfmRV rv;

    rv = gfmTilemap_getNew(&lvltransition.pTransition);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_init(lvltransition.pTransition, gfx.pSset8x8, WIDTH_IN_TILES
            , HEIGHT_IN_TILES, TM_DEFAULT_TILE);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmTilemap_load(lvltransition.pTransition, _tilemap
            , WIDTH_IN_TILES * HEIGHT_IN_TILES, WIDTH_IN_TILES
            , HEIGHT_IN_TILES);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Releases all previously alloc'ed resources */
void freeLeveltransition() {
    if (lvltransition.pTransition != 0) {
        gfmTilemap_free(&lvltransition.pTransition);
    }
}

/** Prepare the transition animation */
err setupLeveltransition() {
    gfmRV rv;
    int x, y;

    ASSERT(lvltransition.pNextLevel != 0, ERR_INDEXOOB);

    lvltransition.dir = lvltransition.pNextLevel->dir;
    lvltransition.srcX = lvltransition.pNextLevel->srcX;
    lvltransition.srcY = lvltransition.pNextLevel->srcY;
    lvltransition.tgtX = lvltransition.pNextLevel->tgtX;
    lvltransition.tgtY = lvltransition.pNextLevel->tgtY;

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    switch (lvltransition.pNextLevel->dir) {
        case TEL_UP: {
            x -= 16;
            y += V_HEIGHT;
        } break;
        case TEL_DOWN: {
            x -= 16;
            y -= HEIGHT_IN_TILES * 8;
        } break;
        case TEL_LEFT: {
            x += V_WIDTH;
            y -= 16;
        } break;
        case TEL_RIGHT: {
            x -= WIDTH_IN_TILES * 8;
            y -= 16;
        } break;
        default: {
            ASSERT(0, ERR_ARGUMENTBAD);
        }
    }

    rv = gfmTilemap_setPosition(lvltransition.pTransition, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    gfmSprite_getPosition(&x, &y, playstate.swordy.pSelf);
    lvltransition.swordyX = (uint16_t)x;
    lvltransition.swordyY = (uint16_t)y;
    gfmSprite_getPosition(&x, &y, playstate.gunny.pSelf);
    lvltransition.gunnyX = (uint16_t)x;
    lvltransition.gunnyY = (uint16_t)y;

    lvltransition.flags = 0;
    lvltransition.timer = 0;

    return ERR_OK;
}

/** Update the transition animation */
err updateLeveltransition() {
    int dx, dy, x, y;
    gfmRV rv;
    err erv;

    dx = (int)lvltransition.tgtX - (int)lvltransition.srcX;
    dy = (int)lvltransition.tgtY - (int)lvltransition.srcY;

    switch (lvltransition.dir) {
        case TEL_UP: {
            dy -= 32;
        } break;
        case TEL_DOWN: {
            dy += 32;
        } break;
        case TEL_LEFT: {
            dx -= 32;
        } break;
        case TEL_RIGHT: {
            dx += 32;
        } break;
    }

    gfmSprite_setPosition(playstate.swordy.pSelf
            , dx + (int)lvltransition.swordyX, dy + (int)lvltransition.swordyY);
    gfmSprite_setPosition(playstate.gunny.pSelf
            , dx + (int)lvltransition.gunnyX, dy + (int)lvltransition.gunnyY);

    erv = loadPlaystate();
    ASSERT(erv == ERR_OK, erv);
    rv = gfm_resetFPS(game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    lvltransition.flags |= LT_LOADED;

    game.currentState = ST_PLAYSTATE;

    return ERR_OK;
#if 0
    gfmRV rv;
    int x, y;

    /* TODO Check that the operation was started */

    lvltransition.timer += game.elapsed;

    rv = gfmCamera_getPosition(&x, &y, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (lvltransition.timer < TRANSITION_TIME) {
        _tweenTilemapIn(x, y);
    }
    else if (lvltransition.timer < 2 * TRANSITION_TIME) {
        _tweenPlayers(x, y, TRANSITION_TIME);

        gfmTilemap_setPosition(lvltransition.pTransition, x - 16, y - 16);
    }
    else if (lvltransition.timer < 3 * TRANSITION_TIME) {
        if (!(lvltransition.flags & LT_LOADED)) {
            err erv;

            _setPlayersPosition();

            /* Load level & reset the FPS counter to ensure there's no lag */
            erv = loadPlaystate();
            ASSERT(erv == ERR_OK, erv);
            rv = gfm_resetFPS(game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);

            lvltransition.flags |= LT_LOADED;

            /* Fix the transition layer position */
            gfmCamera_getPosition(&x, &y, game.pCamera);
            gfmTilemap_setPosition(lvltransition.pTransition, x - 16, y - 16);
        }
        _tweenTilemapOut(x, y);
    }
    else {
        /** Manually set the state so it doesn't get reloaded */
        game.currentState = ST_PLAYSTATE;
    }

    return ERR_OK;
#endif
}

/** Render the transition animation */
err drawLeveltransition() {
    gfmRV rv;
    err erv;

    erv = drawPlaystate();
    ASSERT(erv == ERR_OK, erv);
    rv =  gfmTilemap_draw(lvltransition.pTransition, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    erv = drawPlayers();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/** The transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES] = {
    -1, -1, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101
  , 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101
  , 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, -1, -1

  , -1, -1, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102
  , 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102
  , 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, -1, -1

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , 97, 98, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103
  , 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 99, 100

  , -1, -1, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104
  , 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104
  , 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, -1, -1

  , -1, -1, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105
  , 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105
  , 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, -1, -1
};

