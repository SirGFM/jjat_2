/**
 * @file src/teststate.c
 *
 * Simple state to test stuff
 */
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>

#include <jjat_2/teststate.h>

#include <stdlib.h>
#include <string.h>

struct stTeststate {
};
typedef struct stTeststate teststate;

/**
 * Alloc the test state and initialize all of its components
 *
 * @return GFraMe return value
 */
gfmRV test_init() {
    /** The alloc'ed state */
    teststate *pState;
    /** Return value */
    gfmRV rv;

    pState = 0;

    /* Store the alloc'ed state */
    pGame->pState = pState;
    rv = GFMRV_OK;
__ret:
    if (rv != GFMRV_OK && pState) {
        /* On error, free the state */
        pGame->pState = pState;
        test_free();
    }

    return rv;
}

/**
 * Update the test state
 *
 * @return GFraMe return value
 */
gfmRV test_update() {
    /** The alloc'ed state */
    teststate *pState;
    /** Return value */
    gfmRV rv;

    /* Retrieve the state as a test state */
    pState = (teststate*)pGame->pState;

    rv = GFMRV_OK;
__ret:

    return rv;
}

/**
 * Render the test state
 *
 * @return GFraMe return value
 */
gfmRV test_draw() {
    /** The alloc'ed state */
    teststate *pState;
    /** Return value */
    gfmRV rv;

    /* Retrieve the state as a test state */
    pState = (teststate*)pGame->pState;

    rv = GFMRV_OK;
__ret:

    return rv;
}

/**
 * Release all the resources used by the test state
 *
 * @return GFraMe return value
 */
void test_free() {
    /** The alloc'ed state */
    teststate *pState;

    /* Retrieve the state as a test state */
    pState = (teststate*)pGame->pState;

    if (pState) {
        free(pState);
    }
    pGame->pState = 0;
}

