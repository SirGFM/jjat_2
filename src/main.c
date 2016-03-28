/**
 * @files src/main.c
 *
 * Game entry point. Also manages update, rendering and switching states
 */
#include <base/assets.h>
#include <base/config.h>
#include <base/game_const.h>
#include <base/game_ctx.h>
#include <base/global.h>
#include <base/input.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gframe.h>

#include <jjat_2/state.h>
#include <jjat_2/teststate.h>

/** Required by malloc() and free() */
#include <stdlib.h>
/** Required by memset() */
#include <string.h>

/**
 * Main loop. Handles waiting for input, issuing update and draw, and switch the
 * current state
 *
 * @return GFraMe return value
 */
gfmRV main_loop() {
    /** Return value */
    gfmRV rv;

    while (gfm_didGetQuitFlag(pGame->pCtx) != GFMRV_TRUE) {
        if (pGame->nextState != ST_NONE) {
            /* Init the current state, if switching */
            switch (pGame->nextState) {
                case ST_TEST: test_init(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }

            pGame->curState = pGame->nextState;
            pGame->nextState = ST_NONE;
        }

        /* Wait for an event */
        rv = gfm_handleEvents(pGame->pCtx);
        ASSERT(rv == GFMRV_OK, rv);

#if defined(DEBUG)
        if (!(pGame->flags & GAME_RUN)) {
            gfmInput *pInput;

            pInput = 0;
            rv = gfm_getInput(&pInput, pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
            rv = gfmInput_updateVKey(pInput, pButton->dbgStep.handle);
            ASSERT(rv == GFMRV_OK, rv);
            rv = gfmInput_updateVKey(pInput, pButton->dbgPause.handle);
            ASSERT(rv == GFMRV_OK, rv);

            rv = input_updateDebugButtons();
            ASSERT(rv == GFMRV_OK, rv);
        }
#endif

#if !defined(DEBUG)
        while (gfm_isUpdating(pGame->pCtx) == GFMRV_TRUE) {
#else
        while (((pGame->flags & GAME_RUN) || (pGame->flags & GAME_STEP)) &&
                gfm_isUpdating(pGame->pCtx) == GFMRV_TRUE) {
#endif
            rv = gfm_fpsCounterUpdateBegin(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            rv = input_updateButtons();
            ASSERT(rv == GFMRV_OK, rv);

            rv = gfm_getElapsedTime(&(pGame->elapsed), pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            /* Update the current state */
            switch (pGame->curState) {
                case ST_TEST: rv = test_update(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

            rv = gfm_fpsCounterUpdateEnd(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            pGame->flags &= ~GAME_STEP;
        }

        while (gfm_isDrawing(pGame->pCtx) == GFMRV_TRUE) {
            rv = gfm_drawBegin(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);

            /* Render the current state */
            switch (pGame->curState) {
                case ST_TEST: rv = test_draw(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }
            ASSERT(rv == GFMRV_OK, rv);

            rv = gfm_drawEnd(pGame->pCtx);
            ASSERT(rv == GFMRV_OK, rv);
        }

        if (pGame->nextState != ST_NONE) {
            /* Clear the current state, if switching */
            switch (pGame->curState) {
                case ST_TEST: test_free(); break;
                default: ASSERT(0, GFMRV_INTERNAL_ERROR);
            }

            pGame->curState = ST_NONE;
        }
    }

    rv = GFMRV_OK;
__ret:
    return rv;
}

/**
 * Entry point. Setup everything and handle cleaning up the game, when it exits
 *
 * @param  [ in]argc 
 * @param  [ in]argv 
 */
int main(int argc, char *argv[]) {
    /** Memory block used by the game; One day, this will hopefully point to all
     * data used by the game, so cleaning the game will be as easy as free'ing
     * this pointer */
    void *pMem;
    /** Return value. Set either by an ASSERT that failed on as the return from
     * a call */
    gfmRV rv;

    /* Alloc all of the game's memory */
    pMem = malloc(SIZEOF_GAME_MEM);
    ASSERT(pMem, GFMRV_ALLOC_FAILED);
    memset(pMem, 0x0, SIZEOF_GAME_MEM);
    /* Set and initialize it */
    global_init(pMem);

    /* Initialize the framework (so the configurations may be loaded) */
    rv = gfm_getNew(&(pGame->pCtx));
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_initStatic(pGame->pCtx, ORG, TITLE);
    ASSERT(rv == GFMRV_OK, rv);

    /* Load the configurations */
    rv = config_load();
    ASSERT(rv == GFMRV_OK, rv);

    if (pConfig->flags & CFG_OPENGL3) {
        /* Set OpenGL 3.1 as the video backend */
        rv = gfm_setVideoBackend(pGame->pCtx, GFM_VIDEO_GL3);
    }
    else {
        /* Set SDL 2 as the video backend (this is already the default) */
        rv = gfm_setVideoBackend(pGame->pCtx, GFM_VIDEO_SDL2);
    }
    ASSERT(rv == GFMRV_OK, rv);

    if (pConfig->flags & CFG_FULLSCREEN) {
        /* Initialize the game window in fullscreen mode */
        rv = gfm_initGameFullScreen(pGame->pCtx, V_WIDTH, V_HEIGHT,
                pConfig->resolution, CAN_RESIZE, pConfig->flags & CFG_VSYNC);
    }
    else {
        /* Initialize the game window in windowed mode */
        rv = gfm_initGameWindow(pGame->pCtx, V_WIDTH, V_HEIGHT, pConfig->width,
                pConfig->height, CAN_RESIZE, pConfig->flags & CFG_VSYNC);
    }
    if (rv != GFMRV_OK) {
        /* On failure, write it to the file and exit */
        rv = config_saveError();
        goto __ret;
    }
    ASSERT(rv == GFMRV_OK, rv);

    /* Initialize the audio */
    rv = gfm_initAudio(pGame->pCtx, pConfig->audioQuality);
    ASSERT(rv == GFMRV_OK, rv);

    /* Bind keys */
    rv = input_init();
    ASSERT(rv == GFMRV_OK, rv);

    /* Load assets */
    rv = assets_load();
    ASSERT(rv == GFMRV_OK, rv);

    /* Initialize global variables (e.g., the quadtree) */
    rv = global_initUserVar();
    ASSERT(rv == GFMRV_OK, rv);

    /* Set the initial background color */
    rv = gfm_setBackground(pGame->pCtx, BG_COLOR);
    ASSERT(rv == GFMRV_OK, rv);

    /* Initialize the FPS counter */
    if (FPS_SSET) {
        rv = gfm_initFPSCounter(pGame->pCtx, FPS_SSET, FPS_INIT);
        ASSERT(rv == GFMRV_OK, rv);
        rv = gfm_setFPSCounterPos(pGame->pCtx, FPS_X, FPS_Y);
        ASSERT(rv == GFMRV_OK, rv);
    }

    /* Initialize the base FPS, the update rate and the draw rate */
    rv = gfm_setFPS(pGame->pCtx, pConfig->fps);
    ASSERT(rv == GFMRV_OK, rv);
    rv = gfm_setStateFrameRate(pGame->pCtx, pConfig->fps, pConfig->fps);
    ASSERT(rv == GFMRV_OK, rv);

    /* Set the initial state */
    pGame->nextState = INIT_STATE;
    pGame->flags |= GAME_RUN;

    /* Initialize the main loop */
    rv = main_loop();
    ASSERT(rv == GFMRV_OK, rv);

    rv = GFMRV_OK;
__ret:
    global_freeUserVar();
    if (pGame && pGame->pCtx) {
        /* Dealloc the game */
        gfm_free(&(pGame->pCtx));
    }

    /* Release all memory */
    free(pMem);

    return rv;
}

