/**
 * @file include/jjat2/camera.h
 *
 * Controls the camera
 */
#ifndef __JJAT2_CAMERA_H__
#define __JJAT2_CAMERA_H__

#include <base/error.h>
#include <jjat2/entity.h>

/**
 * Center the camera at both characters.
 *
 * Different from updateCamera, this function won't tween the position.
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 */
err resetCameraPosition(entityCtx *pSwordy, entityCtx *pGunny);

/**
 * Updates the camera position based on both characters
 *
 * @param  [ in]pSwordy 
 * @param  [ in]pGunny  
 */
err updateCamera(entityCtx *pSwordy, entityCtx *pGunny);

#endif /* __JJAT2_CAMERA_H__ */

