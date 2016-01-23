/**
 * @file include/jjat_2/teststate.h
 *
 * Simple state to test stuff
 */
#ifndef __TESTSTATE_H__
#define __TESTSTATE_H__

#include <GFraMe/gfmError.h>

/**
 * Alloc the test state and initialize all of its components
 *
 * @return GFraMe return value
 */
gfmRV test_init();

/**
 * Update the test state
 *
 * @return GFraMe return value
 */
gfmRV test_update();

/**
 * Render the test state
 *
 * @return GFraMe return value
 */
gfmRV test_draw();

/**
 * Release all the resources used by the test state
 *
 * @return GFraMe return value
 */
void test_free();

#endif /* __TESTSTATE_H__ */

