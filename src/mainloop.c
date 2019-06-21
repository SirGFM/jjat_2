/**
 * @file src/mainloop.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/loadstate.h>
#include <base/mainloop.h>
#include <base/resource.h>
#include <base/sfx.h>

#include <conf/state.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmQuadtree.h>

#include <jjat2/fx_group.h>
#include <jjat2/hitbox.h>
#include <jjat2/leveltransition.h>
#include <jjat2/menustate.h>
#include <jjat2/playstate.h>
#include <jjat2/static.h>
#include <jjat2/ui.h>
#include <jjat2/menus/options.h>

#include <string.h>

/** Run the main loop until the game is closed */
err mainloop() {
    err erv;
    gfmRV rv;

    zeroizeGameGlobalCtx();
    erv = initDisplayList();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);
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
    erv = initMenustate();
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    erv = initLoadstate(gfx.pSset8x8, 0/*offset*/);
    ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

    /* Set initial state */
    game.nextState = ST_MENUSTATE;

    while (gfm_didGetQuitFlag(game.pCtx) != GFMRV_TRUE) {
        /* Switch state */
        if (game.nextState != ST_NONE) {
            switch (game.nextState) {
                case ST_PLAYSTATE: erv = loadPlaystate(); break;
                case ST_LEVELTRANSITION: erv = setupLeveltransition(); break;
                case ST_LOADSTATE: erv = ERR_NOTIMPLEMENTED; break;
                case ST_MENUSTATE: erv = loadMenustate(); break;
                default: {}
            }
            ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

            if (game.currentState == ST_MENUSTATE && game.nextState == ST_PLAYSTATE) {
                int len = 0;

                /* When (re)entering the playstate, set the inputs again */
                if (game.curInputMap != 0)
                    len = strlen(game.curInputMap);
                if (len > 0)
                    erv = initInputFromStr(game.curInputMap, len);
                else
                    erv = initInput();
                ASSERT(erv == ERR_OK, erv);
            }

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
            if (game.currentState != ST_LOADSTATE && isLoading()) {
                startLoadstate();
            }
            /* Check if should go back to the previous state during a update, to
             * avoid errors when drawing an invalid state */
            checkStopLoadstate();

            erv = playPendingSong();
            ASSERT_TO(erv == ERR_OK, NOOP(), __ret);

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
                case ST_LOADSTATE: erv = updateLoadstate(); break;
                case ST_MENUSTATE: erv = updateMenustate(); break;
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
                case ST_LOADSTATE: erv = drawLoadstate(); break;
                case ST_MENUSTATE: erv = drawMenustate(); break;
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
    }

    erv = ERR_OK;
__ret:
    freeHitboxes();
    freeUI();
    freeLeveltransition();
    freeFxGroup();
    freePlaystate();
    freeMenustate();
    freeDisplayList();

    return erv;
}

