/**
 * @file src/jjat2/camera.c
 *
 * Controls the camera
 */

#include <base/error.h>
#include <base/game.h>
#include <jjat2/camera.h>
#include <jjat2/entity.h>
#include <GFraMe/gfmCamera.h>

#define CAMERA_X    80
#define CAMERA_Y    32
#define CAMERA_W    160
#define CAMERA_H    112

/**
 * Center the camera at both characters.
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 * @param  [ in]tween   Tween factor between characters (TODO)
 */
static inline err _centerCamera(entityCtx *pSwordy, entityCtx *pGunny
        , int tween) {
    int sx, sy, gx, gy;

    gfmSprite_getCenter(&sx, &sy, pSwordy->pSelf);
    gfmSprite_getCenter(&gx, &gy, pGunny->pSelf);
    gfmCamera_centerAtPoint(game.pCamera, (sx + gx) / 2, (sy + gy) / 2);

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
    return _centerCamera(pSwordy, pGunny, 0/*tween*/);
}

/**
 * Updates the camera position based on both characters
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 */
err updateCamera(entityCtx *pSwordy, entityCtx *pGunny) {
    return _centerCamera(pSwordy, pGunny, 0/*tween*/);
}

