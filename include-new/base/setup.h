/**
 * @file include/base/setup.h
 */
#ifndef __SETUP_H__
#define __SETUP_H__

#include <GFraMe/gframe.h>

#define V_WIDTH     320
#define V_HEIGHT    240

struct stConfig {
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
    /** Video backend */
    gfmVideoBackend videoBackend;
    /** Audio quality */
    gfmAudioQuality audioSettings;
};
typedef struct stConfig config;

/** Initialize the configuration struct with the default values */
#define CONFIG_INIT(c) \
  do { \
    (c).vsync = 0;\
    (c).fullscreen = 0;\
    (c).wndWidth = 640;\
    (c).wndHeight = 480;\
    (c).fpsQuality = 60;\
    (c).videoBackend = GFM_VIDEO_SDL2;\
    (c).audioSettings = gfmAudio_defQuality;\
  } while (0)

#endif /* __SETUP_H__ */

