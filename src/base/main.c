/**
 * @file src/main.c
 */
#include <base/collision.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/loadstate.h>
#include <base/mainloop.h>
#include <base/resource.h>
#include <base/setup.h>
#include <base/sfx.h>
#include <base/static.h>

/**
 * Entry point. Setup everything and handle cleaning up the game, when it exits
 *
 * @param  [ in]argc 
 * @param  [ in]argv 
 */
int main(int argc, char *argv[]) {
    err erv;

    zeroizeGlobalCtx();

    erv = setupGame(argc, argv);
    if (erv == ERR_FORCEEXIT) {
        erv = ERR_OK;
        goto __ret;
    }
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);

    erv = initGfx();
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);
    erv = initSfx();
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);
    erv = initResource();
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);

#if defined(JJATENGINE)
    if (!(game.flags & CMD_CUSTOMINPUT)) {
        erv = initInput();
        ASSERT_TO(erv == ERR_OK, erv = erv, __ret);
    }
#else /* !JJATENGINE */
    erv = initInput();
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);
#endif /* JJATENGINE */

    erv = setupCollision();
    ASSERT_TO(erv == ERR_OK, erv = erv, __ret);

    erv = mainloop();
__ret:
    cleanResource();
    cleanCollision();
    freeLoadstate();
    cleanGame();

    return erv;
}

