/**
 * @file src/jjat2/camera.c
 *
 * Controls the camera
 */

#include <base/error.h>
#include <base/game.h>
#include <conf/game.h>
#include <jjat2/camera.h>
#include <jjat2/entity.h>
#include <jjat2/ui.h>
#include <GFraMe/gfmCamera.h>

#define CAMERA_W    8
#define CAMERA_H    8
#define CAMERA_X    ((V_WIDTH - CAMERA_W) / 2)
#define CAMERA_Y    ((V_HEIGHT - CAMERA_H) / 2)
/** These values for the tween may only became an issue if a sprite goes beyond
 * position 1048575 (2^19)... i.e., unlikely */
#define MIN_TWEEN   0
#define MAX_TWEEN   1024
#define REST_TWEEN  ((MAX_TWEEN - MIN_TWEEN) / 2)

/** Accumulate the current tween factor */
static int _tween;

/** Enumerate all possible tween directions */
enum enTweenDirection{
    NO_TWEEN        = 0x00
  , TWEEN_TO_SWORDY = 0x01
  , TWEEN_TO_GUNNY  = 0x02
  , TWEEN_TO_CENTER = (TWEEN_TO_SWORDY | TWEEN_TO_GUNNY)
};
typedef enum enTweenDirection tweenDirection;

/** Simple interpolation between two values */
static inline int _tweenValue(int a, int b, int tween) {
    int valA, valB;

    valB = b * tween;
    valA = a * (MAX_TWEEN - tween);

    return (valA + valB) / MAX_TWEEN;
}

/**
 * Center the camera at both characters.
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 * @param  [ in]tween   Tween factor between characters
 */
static inline err _centerCamera(entityCtx *pSwordy, entityCtx *pGunny
        , int tween) {
    int cx, cy, gx, gy, sx, sy, x, y;

    gfmSprite_getCenter(&sx, &sy, pSwordy->pSelf);
    gfmSprite_getCenter(&gx, &gy, pGunny->pSelf);
    x = _tweenValue(sx, gx, tween);
    y = _tweenValue(sy, gy, tween);
    gfmCamera_centerAtPoint(game.pCamera, x, y);

    /* Update the UI position */
    gfmCamera_getPosition(&cx, &cy, game.pCamera);
    if ((y - cy) < UI_BORDER_DIMENSION) {
        /* Center is within the upper part of the screen */
        setUIPosition(UI_BOTTOM);
    }
    else if (V_HEIGHT - (y - cy) < UI_BORDER_DIMENSION) {
        /* Center is within the lower part of the screen */
        setUIPosition(UI_TOP);
    }

    return ERR_OK;
}

/**
 * Center the camera at both characters.
 *
 * Different from updateCamera, this function won't tween the position.
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 */
err resetCameraPosition(entityCtx *pSwordy, entityCtx *pGunny) {
    gfmRV rv;

    rv = gfmCamera_setDeadzone(game.pCamera, CAMERA_X, CAMERA_Y, CAMERA_W
            , CAMERA_H);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    _tween = REST_TWEEN;
    return _centerCamera(pSwordy, pGunny, _tween);
}

/**
 * Check if a sprite moved on the last/current frame
 *
 * @param  [ in]pSpr The sprite
 * @return           1 = moved, 0 = didn't
 */
static int _didSpriteMove(gfmSprite *pSpr) {
    int lx, ly, x, y;

    gfmSprite_getCenter(&x, &y, pSpr);
    gfmSprite_getLastCenter(&lx, &ly, pSpr);

    return x != lx || y != ly;
}

/**
 * Updates the camera position based on both characters
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 */
err updateCamera(entityCtx *pSwordy, entityCtx *pGunny) {
    tweenDirection dir = NO_TWEEN;

    if ((game.flags & AC_SWORDY) == AC_SWORDY
            && _didSpriteMove(pSwordy->pSelf)) {
        dir |= TWEEN_TO_SWORDY;
    }
    if ((game.flags & AC_GUNNY) == AC_GUNNY && _didSpriteMove(pGunny->pSelf)) {
        dir |= TWEEN_TO_GUNNY;
    }

    switch (dir) {
        case TWEEN_TO_SWORDY: {
            if (_tween - game.elapsed > MIN_TWEEN) {
                _tween -= game.elapsed;
            }
            else {
                _tween = MIN_TWEEN;
            }
        } break;
        case TWEEN_TO_GUNNY: {
            if (_tween + game.elapsed < MAX_TWEEN) {
                _tween += game.elapsed;
            }
            else {
                _tween = MAX_TWEEN;
            }
        } break;
        case NO_TWEEN:
        case TWEEN_TO_CENTER: {
            if (_tween < REST_TWEEN && _tween + game.elapsed < REST_TWEEN) {
                _tween += game.elapsed;
            }
            else if (_tween > REST_TWEEN &&
                    _tween - game.elapsed > REST_TWEEN) {
                _tween -= game.elapsed;
            }
            else {
                _tween = REST_TWEEN;
            }
        } break;
    }

    return _centerCamera(pSwordy, pGunny, _tween);
}

