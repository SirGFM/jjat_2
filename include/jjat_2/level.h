/**
 * @file include/jjat_2/level.h
 *
 * Uses a gfmParser to load the desired level
 */
#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <GFraMe/gfmError.h>

/**
 * Loads a level
 *
 * @param  [ in]index The index of the desired level
 * @return            GFraMe return value
 */
gfmRV level_load(int index);

/**
 * Load the test level
 *
 * @return            GFraMe return value
 */
gfmRV level_loadTest();

#endif /* __LEVELLOADER_H__ */

