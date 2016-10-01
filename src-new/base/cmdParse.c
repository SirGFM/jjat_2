/**
 * @file src/base/cmdParse.c
 *
 * Module to parse command line options.
 *
 * Supported options:
 *  * --help | -h: Print usage
 *  * --backend | -b: Set the video backend {OpenGL, SDL, Software}
 *  * --pixel-resolution | -x: Set the initial upcaling factor
 *  * --FPS | -F: Set the game's initial (and maximum) FPS
 *  * --audio | -a: *TODO* Set the audio quality
 *  * --vsync | -v: Enable VSync
 *  * --fullscreen | -f: Init game in fullscreen mode
 *  * --save | -s: *TODO* Save the current configuration
 */
#include <base/error.h>
#include <base/setup.h>

#include <GFraMe/gframe.h>

#include <string.h>

/* TODO Define LOG */
#define LOG(...)

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
#define GET_PARAM() argv[i + 1]

/** Retrieve the current parameter as a number and store it on the variable
 * passed as parameter */
#define GET_NUM(num) \
  do { \
    char *pNum; \
    int tmp; \
    pNum = argv[i + 1]; \
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
err cmpParse(config *pConfig, int argc, char *argv[]) {
    int doSave = 0;

    CONFIG_INIT(*pConfig);

    /* TODO Check if there's any configuration to be loaded */

    DO_PARSE() {
        if (0) {}
        IS_FLAG("--help", "-h") {
            /* TODO Print usage */
        }
        IS_FLAG("--backend", "-b") {
            CHECK_PARAM();

            if (strcmp(GET_PARAM(), "OpenGL") == 0) {
                pConfig->videoBackend = GFM_VIDEO_SDL2;
            }
            else if (strcmp(GET_PARAM(), "SDL") == 0) {
                pConfig->videoBackend = GFM_VIDEO_GL3;
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
        IS_FLAG("--audio", "-a") {
            CHECK_PARAM();

            /* TODO Load audio configurations */
        }
        IS_FLAG("--vsync", "-v") {
            pConfig->vsync = 1;
        }
        IS_FLAG("--fullscreen", "-f") {
            pConfig->fullscreen = 1;
        }
        IS_FLAG("--save", "-s") {
            doSave = 1;
        }

        NEXT_TOKEN();
    }

    if (doSave) {
        /* TODO Save the configuration */
    }

    return ERR_OK;
}

