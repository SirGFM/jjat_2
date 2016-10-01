/**
 * @file include/base/cmdParse.h
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
#ifndef __CMD_PARSE_H__
#define __CMD_PARSE_H__

#include <base/error.h>
#include <base/setup.h>

/**
 * Parse every command
 *
 * @param  [out]pConfig Struct filled with all initial configurations
 * @param  [ in]argc    Number of arguments received
 * @param  [ in]argv    List of arguments received
 * @return           
 */
err cmpParse(config *pConfig, int argc, char *argv[]);

#endif /* __CMD_PARSE_H__ */

