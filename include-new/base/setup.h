/**
 * @file include/base/setup.h
 *
 * Implement all initial setup
 */
#ifndef __SETUP_H__
#define __SETUP_H__

#include <base/error.h>

#include <GFraMe/gframe.h>

#define V_WIDTH     320
#define V_HEIGHT    240
#define ORG         "com.gfmgamecorner"
#define TITLE       "JJAT+"

/** Structure filled with all parsed configurations */
struct stConfigCtx {
    /** Whether vsync is enabled */
    int vsync;
    /** Whether the game should start in fullscreen */
    int fullscreen;
    /** Initial window width */
    int wndWidth;
    /** Initial window height */
    int wndHeight;
    /** Initial FPS (base FPS and update/draw rate) */
    int fpsQuality;
    /** Index of fullscreen resolution (if on fullscreen mode) */
    int fullscreenResolution;
    /** Video backend */
    gfmVideoBackend videoBackend;
    /** Audio quality */
    gfmAudioQuality audioSettings;
};
typedef struct stConfigCtx configCtx;

/** Initialize the configuration struct with the default values */
#define CONFIG_INIT(c) \
  do { \
    (c).vsync = 0;\
    (c).fullscreenResolution = 0; \
    (c).fullscreen = 0;\
    (c).wndWidth = 640;\
    (c).wndHeight = 480;\
    (c).fpsQuality = 60;\
    (c).videoBackend = GFM_VIDEO_SDL2;\
    (c).audioSettings = gfmAudio_defQuality;\
  } while (0)

/**
 * Basic setup for the game.
 *
 * Load all basic configuration (from the command line and/or from a file on the
 * system). When the function returns (if successful), all sub-systems will be
 * ready for use:
 *  - The game window will be set up at the desired resolution (as defined on
 *    setup.h, where V_WIDTH and V_HEIGHT define the virtual window dimensions
 *    and the real window dimension is a multiple of that), audio will be ready
 *  - Audio will be ready to load and play songs at the requested quality
 *  - FPS will be configured and initialized
 *
 * Note that since the FPS is already configured, it's important to reset it
 * before starting the main loop. Otherwise, there may be some skipped frames on
 * startup.
 * Also, input must be manually set up later.
 *
 * @param  [ in]argc    Number of arguments received
 * @param  [ in]argv    List of arguments received
 * @return
 */
err setupGame(int argc, char *argv[]);

/**
 * Release all resources alloc'ed on 'setupGame'
 */
void cleanGame();

#endif /* __SETUP_H__ */

