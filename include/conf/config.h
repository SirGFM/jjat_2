/**
 * @file include/conf/config.h
 *
 * Describe the structure filled by cmdParse (and used during the game's
 * initialization).
 */
#ifndef __CONF_CONFIG_H__
#define __CONF_CONFIG_H__

#include <GFraMe/gframe.h>

enum enConfigFlags {
    CFG_VSYNC       = 0x01
  , CFG_FULLSCREEN  = 0x02
#if defined(JJATENGINE)
  , CFG_SYNCCONTROL = 0x04
#endif /* JJATENGINE */
};
typedef enum enConfigFlags configFlags;

/** Structure filled with all parsed configurations */
struct stConfigCtx {
#if defined(JJATENGINE)
    /** Alternative key mapping */
    char *pKeyMap;
#endif /* JJATENGINE */
    /** Bit-mask for flags */
    configFlags flags;
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
    (c).pKeyMap = 0; \
    (c).flags = 0;\
    (c).fullscreenResolution = 0; \
    (c).wndWidth = 640;\
    (c).wndHeight = 480;\
    (c).fpsQuality = 60;\
    (c).videoBackend = GFM_VIDEO_SDL2;\
    (c).audioSettings = gfmAudio_defQuality;\
  } while (0)

#endif /* __CONF_CONFIG_H__ */

