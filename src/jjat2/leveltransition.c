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

#include <jjat2/camera.h>
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

#define TRANSITION_TIME 500
#define WIDTH_IN_TILES  (V_WIDTH / 8 + 4)
#define HEIGHT_IN_TILES (V_HEIGHT / 8 + 4)
#define TM_DEFAULT_TILE -1
#define FIX_THRESHOLD 8
#define FIX_BASE_MS 30

/** Forward declaration of the transition tilemap */
static int _tilemap[WIDTH_IN_TILES * HEIGHT_IN_TILES];

/**
 * Linearly Interpolate between two points A and B. The interpolation is
 * calculated for the time lvltransition.timer.
 *
 * @param  [ in]a        The starting position
 * @param  [ in]b        The ending position
 * @param  [ in]initTime Time the interpolation started
 * @return               The interpolated position
 */
static inline int _tween(int a, int b, int initTime) {
    int dt;

    /* TODO Use a better interpolation algorithm */
    dt = lvltransition.timer - initTime;
    return (a * (TRANSITION_TIME - dt) / TRANSITION_TIME)
            + (b * dt / TRANSITION_TIME);
}

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
            cy += _tween(V_HEIGHT, -16, 0);
        } break;
        case TEL_DOWN: {
            cx -= 16;
            cy += _tween(-HEIGHT_IN_TILES * 8, -16, 0);
        } break;
        case TEL_LEFT: {
            cy -= 16;
            cx += _tween(V_WIDTH, -16, 0);
        } break;
        case TEL_RIGHT: {
            cy -= 16;
            cx += _tween(-WIDTH_IN_TILES * 8, -16, 0);
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

/**
 * Tween a sprite in screen space toward the next position (coverted to screen
 * space)
 *
 * NOTE 1: Target position is expected to be within the camera!
 * NOTE 2: Source position is expected to be stored on lvltransition
 *
 * @param  [ in]pSpr     The player's sprite
 * @param  [ in]srcX     The initial position
 * @param  [ in]srcY     The initial position
 * @param  [ in]tgtX     Target position
 * @param  [ in]tgtY     Target position
 * @param  [ in]initTime Time when the tween started
 */
static void _tweenPlayer(gfmSprite *pSpr, int srcX, int srcY, int tgtX, int tgtY
        , int initTime) {
    int dstX, dstY;

    dstX =_tween(srcX, tgtX, initTime);
    dstY =_tween(srcY, tgtY, initTime);
    gfmSprite_setPosition(pSpr, dstX, dstY);
}

/**
 * Tween the tilemap OUTSIDE the screen
 *
 * @param  [ in]cx       The camera position
 * @param  [ in]cy       The camera position
 * @param  [ in]initTime Time when the tween started
 */
static void _tweenTilemapOut(int cx, int cy, int initTime) {
    switch (lvltransition.dir) {
        case TEL_UP: {
            cx -= 16;
            cy += _tween(-16, -HEIGHT_IN_TILES * 8, initTime);
        } break;
        case TEL_DOWN: {
            cx -= 16;
            cy += _tween(-16, V_HEIGHT, initTime);
        } break;
        case TEL_LEFT: {
            cy -= 16;
            cx += _tween(-16, -WIDTH_IN_TILES * 8, initTime);
        } break;
        case TEL_RIGHT: {
            cy -= 16;
            cx += _tween(-16, V_WIDTH, initTime);
        } break;
    }

    gfmTilemap_setPosition(lvltransition.pTransition, cx, cy);
}

/**
 * Prepare switching to a level transition
 *
 * @param  [ in]pNextLevel Next level data
 */
void switchToLevelTransition(leveltransitionData *pNextLevel) {
    lvltransition.flags = 0;
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

    lvltransition.timer = 0;

    return ERR_OK;
}

/**
 * Get measures that should be added to a player's initial position to reach its
 * final position
 */
static void _getRelativePosition(int *x, int *y) {
    *x = (int)lvltransition.tgtX - (int)lvltransition.srcX;
    *y = (int)lvltransition.tgtY - (int)lvltransition.srcY;

    switch (lvltransition.dir) {
        case TEL_UP: {
            *y -= 32;
        } break;
        case TEL_DOWN: {
            *y += 32;
        } break;
        case TEL_LEFT: {
            *x -= 32;
        } break;
        case TEL_RIGHT: {
            *x += 32;
        } break;
    }
}

/** Update the transition animation */
err updateLeveltransition() {
    err erv;
    gfmRV rv;
    int ch, cw, cx, cy;

    /* TODO Check that the operation was started */

    lvltransition.timer += game.elapsed;

    rv = gfmCamera_getPosition(&cx, &cy, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmCamera_getDimensions(&cw, &ch, game.pCamera);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    if (lvltransition.timer < TRANSITION_TIME) {
        _tweenTilemapIn(cx, cy);
    }
    else if (lvltransition.timer < 2 * TRANSITION_TIME) {
        int tgtX, tgtY;

        /* Move players to the center of the screen */
        tgtX = cx + cw / 2;
        tgtY = cy + ch / 2;

        _tweenPlayer(playstate.swordy.pSelf, lvltransition.swordyX
                , lvltransition.swordyY, tgtX, tgtY, TRANSITION_TIME);
        _tweenPlayer(playstate.gunny.pSelf, lvltransition.gunnyX
                , lvltransition.gunnyY, tgtX, tgtY, TRANSITION_TIME);

        gfmTilemap_setPosition(lvltransition.pTransition, cx - 16, cy - 16);
    }
    else if (!(lvltransition.flags & LT_LOADED)) {
        erv = loadPlaystate();
        ASSERT(erv == ERR_OK, erv);
        rv = gfm_resetFPS(game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        lvltransition.flags |= LT_LOADED;

        /* After loading the stage, adjust the camera position and center the
         * players (and the transition effect) within it */
        if (lvltransition.flags & LT_CHECKPOINT) {
            gfmSprite_setPosition(playstate.swordy.pSelf, lvltransition.tgtX
                    , lvltransition.tgtY);
            gfmSprite_setPosition(playstate.gunny.pSelf, lvltransition.tgtX
                    , lvltransition.tgtY);
        }
        else {
            int dx, dy;

            _getRelativePosition(&dx, &dy);

            gfmSprite_setPosition(playstate.swordy.pSelf
                    , dx + lvltransition.swordyX, dy + lvltransition.swordyY);
            gfmSprite_setPosition(playstate.gunny.pSelf
                    , dx + lvltransition.gunnyX, dy + lvltransition.gunnyY);
        }

        erv = resetCameraPosition(&playstate.swordy, &playstate.gunny);
        ASSERT(erv == ERR_OK, erv);

        gfmCamera_getPosition(&cx, &cy, game.pCamera);
        gfmSprite_setPosition(playstate.swordy.pSelf, cx + cw / 2, cy + ch / 2);
        gfmSprite_setPosition(playstate.gunny.pSelf, cx + cw / 2, cy + ch / 2);
        gfmTilemap_setPosition(lvltransition.pTransition, cx - 16, cy - 16);
    }
    else if (lvltransition.timer < 3 * TRANSITION_TIME) {
        int srcX, srcY;


        srcX = cx + cw / 2;
        srcY = cy + ch / 2;

        if (lvltransition.flags & LT_CHECKPOINT) {
            _tweenPlayer(playstate.swordy.pSelf, srcX, srcY, lvltransition.tgtX
                    , lvltransition.tgtY, 2 * TRANSITION_TIME);
            _tweenPlayer(playstate.gunny.pSelf, srcX, srcY, lvltransition.tgtX
                    , lvltransition.tgtY, 2 * TRANSITION_TIME);
        }
        else {
            int dx, dy;

            _getRelativePosition(&dx, &dy);

            _tweenPlayer(playstate.swordy.pSelf, srcX, srcY
                    , dx + lvltransition.swordyX, dy + lvltransition.swordyY
                    , 2 * TRANSITION_TIME);
            _tweenPlayer(playstate.gunny.pSelf, srcX, srcY
                    , dx + lvltransition.gunnyX, dy + lvltransition.gunnyY
                    , 2 * TRANSITION_TIME);
        }
    }
    else if (lvltransition.timer < 4 * TRANSITION_TIME) {
        _tweenTilemapOut(cx, cy, 3 * TRANSITION_TIME);
    }
    else {
        /* Manually set the state so it doesn't get reloaded */
        game.currentState = ST_PLAYSTATE;

        /* Also, ensure the players are on the correct position (and not off by
         * one pixel) */
        if (lvltransition.flags & LT_CHECKPOINT) {
            gfmSprite_setPosition(playstate.swordy.pSelf, lvltransition.tgtX
                    , lvltransition.tgtY);
            gfmSprite_setPosition(playstate.gunny.pSelf, lvltransition.tgtX
                    , lvltransition.tgtY);
        }
        else {
            int dx, dy;

            _getRelativePosition(&dx, &dy);

            /* If it's an upward transition and either of them were put a few
             * pixels above a transition, push them until they are touching it.
             * This fixes activating the fix to the downward transition bug.
             * This only happens for 30 FPS or lower. */
            if (lvltransition.dir == TEL_UP && game.elapsed > FIX_BASE_MS) {
                if (dy + lvltransition.swordyY + swordy_height >=
                        lvltransition.tgtY - FIX_THRESHOLD) {
                    dy = lvltransition.tgtY - lvltransition.swordyY
                            - swordy_height;
                }
                else if (dy + lvltransition.gunnyY + gunny_height >=
                        lvltransition.tgtY - FIX_THRESHOLD) {
                    dy = lvltransition.tgtY - lvltransition.gunnyY
                            - gunny_height;
                }
            }

            gfmSprite_setPosition(playstate.swordy.pSelf
                    , dx + lvltransition.swordyX, dy + lvltransition.swordyY);
            gfmSprite_setPosition(playstate.gunny.pSelf
                    , dx + lvltransition.gunnyX, dy + lvltransition.gunnyY);
        }
    }

    return ERR_OK;
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

