/**
 * @file src/base/cmdParse.c
 *
 * Module to parse command line options.
 *
 * Supported options:
 *  -h | --help: Print usage
 *  -b | --backend: Set the video backend {OpenGL, SDL, Software}
 *  -x | --pixel-resolution: Set the initial upcaling factor
 *  -r | --resolution: Set the fullscreen resolution
 *  -F | --FPS: Set the game's initial (and maximum) FPS
 *  -a | --audio: *TODO* Set the audio quality
 *  -v | --vsync: Enable VSync
 *  -f | --fullscreen: Init game in fullscreen mode
 *  -l | --list: List all available resolution
#if defined(JJATENGINE)
 *  -c | --syncctr: Set character control as synchronous (i.e., move a single character at a time)
 *  -k | --keymap: Remap all keys to the specified configuration
 *  -s | --simpledraw: Slightly speed up drawing on some parts
#endif JJATENGINE
 *  -S | --save: *TODO* Save the current configuration
 *  -z | --lazy-load: Ignore if songs hasn't finished loading
 */
#include <base/cmdParse.h>
#include <base/error.h>
#include <base/game.h>
#include <conf/config.h>
#include <conf/game.h>

#include <GFraMe/gframe.h>

#include <stdio.h>
#include <string.h>

#define LOG(...) printf(__VA_ARGS__)

static void usage() {
    LOG("Options\n\n");
    LOG("  -b | --backend: Set the video backend {OpenGL, SDL, Software}\n");
    LOG("  -x | --pixel-resolution: Set the initial upcaling factor\n");
    LOG("  -F | --FPS: Set the game's initial (and maximum) FPS\n");
    LOG("  -r | --resolution: Set which resolution is to be used on fullscreen "
            "mode\n");
    LOG("  -a | --audio: *TODO* Set the audio quality\n");
    LOG("  -v | --vsync: Enable VSync\n");
    LOG("  -f | --fullscreen: Init game in fullscreen mode\n");
    LOG("  -l | --list: List all available resolution\n");
#if defined(JJATENGINE)
    LOG("  -c | --syncctr: Set character control as synchronous\n"
            "                  (i.e., move a single character at a time)\n");
    LOG("  -k | --keymap: Remap all keys to the specified configuration\n");
    LOG("  -s | --simpledraw: Slightly speed up drawing on some parts\n");
#endif /* JJATENGINE */
    LOG("  -S | --save: *TODO* Save the current configuration\n");
    LOG("  -z | --lazy-load: Ignore if songs hasn't finished loading\n");
    LOG("  -h | --help: Print usage\n");
}

/** Initialize the parsing context */
#define DO_PARSE() \
    int i = 0; \
    while (i < argc) \

/** Iterate to the next token */
#define NEXT_TOKEN() i++

/**
 * Check if a FLAG was parsed
 *
 * @param  [ in]l_cmd Long command (e.g., --help)
 * @param  [ in]s_cmd Short command (e.g., -h)
 */
#define IS_FLAG(l_cmd, s_cmd) \
  else if (strcmp(argv[i], l_cmd) == 0 || strcmp(argv[i], s_cmd) == 0)

/** Check if there's an option for the parsed arguments */
#define CHECK_PARAM() \
  do { \
    if (argc <= i + 1) { \
      LOG("Expected parameter but got nothing!"); \
      return ERR_MISSINGPARAM; \
    } \
    NEXT_TOKEN(); \
  } while (0)

/** Retrieve the current parameter as a volatile string */
#define GET_PARAM() argv[i]

/** Retrieve the current parameter as a number and store it on the variable
 * passed as parameter */
#define GET_NUM(num) \
  do { \
    char *pNum; \
    int tmp; \
    pNum = argv[i]; \
    tmp = 0; \
    while (*pNum != '\0') { \
        tmp = tmp * 10 + (*pNum) - '0'; \
        pNum++; \
    } \
    (num) = tmp; \
  } while (0)

/**
 * Parse every command
 *
 * @param  [out]pConfig Struct filled with all initial configurations
 * @param  [ in]argc    Number of arguments received
 * @param  [ in]argv    List of arguments received
 * @return           
 */
err cmdParse(configCtx *pConfig, int argc, char *argv[]) {
    int doSave = 0;

    CONFIG_INIT(*pConfig);

    /* TODO Check if there's any configuration to be loaded */

    DO_PARSE() {
        if (0) {}
        IS_FLAG("--backend", "-b") {
            CHECK_PARAM();

            if (strcmp(GET_PARAM(), "OpenGL") == 0) {
                pConfig->videoBackend = GFM_VIDEO_GL3;
            }
            else if (strcmp(GET_PARAM(), "SDL") == 0) {
                pConfig->videoBackend = GFM_VIDEO_SDL2;
            }
            else if (strcmp(GET_PARAM(), "Software") == 0) {
                pConfig->videoBackend = GFM_VIDEO_SWSDL2;
            }
            else {
                return ERR_ARGUMENTBAD;
            }
        }
        IS_FLAG("--pixel-resolution", "-x") {
            int size;
            CHECK_PARAM();


            GET_NUM(size);
            pConfig->wndWidth = V_WIDTH * size;
            pConfig->wndHeight = V_HEIGHT * size;
        }
        IS_FLAG("--FPS", "-F") {
            CHECK_PARAM();

            GET_NUM(pConfig->fpsQuality);
        }
        IS_FLAG("--resolution", "-r") {
            CHECK_PARAM();

            GET_NUM(pConfig->fullscreenResolution);
        }
        IS_FLAG("--audio", "-a") {
            CHECK_PARAM();

            /* TODO Load audio configurations */
        }
        IS_FLAG("--vsync", "-v") {
            pConfig->flags |= CFG_VSYNC;
        }
        IS_FLAG("--fullscreen", "-f") {
            pConfig->flags |= CFG_FULLSCREEN;
        }
#if defined(JJATENGINE)
        IS_FLAG("--syncctr", "-c") {
            pConfig->flags |= CFG_SYNCCONTROL;
        }
        IS_FLAG("--keymap", "-k") {
            CHECK_PARAM();

            pConfig->pKeyMap = GET_PARAM();
        }
        IS_FLAG("--simpledraw ", "-s") {
            pConfig->flags |= CFG_SIMPLEDRAW;
        }
#endif /* JJATENGINE */
        IS_FLAG("--save", "-S") {
            doSave = 1;
        }
        IS_FLAG("--list", "-l") {
            gfmRV rv;
            int len, i = 0;

            LOG("Available resolutions:\n");

            rv = gfm_queryResolutions(&len, game.pCtx);
            ASSERT(rv == GFMRV_OK, ERR_GFMERR);
            while (i < len) {
                int width, height, fps;

                rv = gfm_getResolution(&width, &height, &fps, game.pCtx, i);
                ASSERT(rv == GFMRV_OK, ERR_GFMERR);

                LOG("  %i: %ix%i@%iHz\n", i, width, height, fps);

                i++;
            }

            return ERR_FORCEEXIT;
        }
        IS_FLAG("--lazy-load", "-z") {
            pConfig->flags |= CFG_LAZYLOAD;
        }
        IS_FLAG("--help", "-h") {
            usage();

            return ERR_FORCEEXIT;
        }

        NEXT_TOKEN();
    }

    if (doSave) {
        /* TODO Save the configuration */
    }

    return ERR_OK;
}

