/**
 * @file src/mainloop.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <base/mainloop.h>

#include <conf/state.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>

#include <jjat2/fx_group.h>
#include <jjat2/hitbox.h>
#include <jjat2/leveltransition.h>
#include <jjat2/playstate.h>
#include <jjat2/static.h>
#include <jjat2/ui.h>

#if defined(EMCC)
#  include <emscripten/emscripten.h>
#endif

#if !defined(EMCC)
err _loop(void *_null) {
#else
void _loop(void *_null) {
#endif
    err erv;
    gfmRV rv;

#if defined(EMCC)
    /* If on HTML5, must issue a frame */
    rv = gfm_issueFrame(game.pCtx);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
#endif

    /* Switch state */
    if (game.nextState != ST_NONE) {
        switch (game.nextState) {
            case ST_PLAYSTATE: erv = loadPlaystate(); break;
            case ST_LEVELTRANSITION: erv = setupLeveltransition(); break;
            default: erv = ERR_OK;
        }
        ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

        game.currentState = game.nextState;
        game.nextState = ST_NONE;
    }

    /* Wait for an event */
    rv = gfm_handleEvents(game.pCtx);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

#if defined(DEBUG)
    erv = updateDebugInput();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
    handleDebugInput();
#endif

    while (DO_UPDATE()) {
        rv = gfm_fpsCounterUpdateBegin(game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        erv = updateInput();
        ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
        handleInput();

        rv = gfm_getElapsedTime(&(game.elapsed), game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        /* Update the current state */
        switch (game.currentState) {
            case ST_PLAYSTATE: erv = updatePlaystate(); break;
            case ST_LEVELTRANSITION: erv = updateLeveltransition(); break;
            default: {}
        }
        ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

        rv = gfm_fpsCounterUpdateEnd(game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        DEBUG_STEP();
    }

    while (gfm_isDrawing(game.pCtx) == GFMRV_TRUE) {
        rv = gfm_drawBegin(game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        /* Render the current state */
        switch (game.currentState) {
            case ST_PLAYSTATE: erv = drawPlaystate(); break;
            case ST_LEVELTRANSITION: erv = drawLeveltransition(); break;
            default: {}
        }
        ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

        if (IS_QUADTREE_VISIBLE()) {
            rv = gfmQuadtree_drawBounds(collision.pStaticQt, game.pCtx, 0);
            ASSERT_TO(rv == GFMRV_QUADTREE_EMPTY
                    || rv == GFMRV_QUADTREE_NOT_INITIALIZED
                    || rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
            rv = gfmQuadtree_drawBounds(collision.pQt, game.pCtx, 0);
            ASSERT_TO(rv == GFMRV_QUADTREE_EMPTY
                    || rv == GFMRV_QUADTREE_NOT_INITIALIZED
                    || rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
        }

        rv = gfm_drawRenderInfo(game.pCtx, 0, 0/*x*/, 24/*y*/, 0);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);

        rv = gfm_drawEnd(game.pCtx);
        ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
    }

#if defined(EMCC)
    // If on HTML5, sleep until the next frame
    rv = gfm_waitFrame(game.pCtx);
    ASSERT_TO(rv == GFMRV_OK, erv = ERR_GFMERR, __ret);
#endif

    erv = ERR_OK;
__ret:
#if !defined(EMCC)
    return erv;
#else
    erv = ERR_OK;
#endif
}

/** Run the main loop until the game is closed */
err mainloop() {
    err erv;

    zeroizeGameGlobalCtx();
    erv = initPlaystate();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
    erv = initFxGroup();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
    erv = initLeveltransition();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
    erv = initUI();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
    erv = initHitboxes();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    /* Set initial state */
    game.nextState = ST_PLAYSTATE;

#if !defined(EMCC)
    while (erv == ERR_OK && gfm_didGetQuitFlag(game.pCtx) != GFMRV_TRUE) {
        erv = _loop(0);
    }
#else
    emscripten_set_main_loop_arg((em_arg_callback_func)_loop, 0, 0, 1);
#endif

    erv = ERR_OK;
__ret:
    freeHitboxes();
    freeUI();
    freeLeveltransition();
    freeFxGroup();
    freePlaystate();

    return erv;
}

