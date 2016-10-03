/**
 * @file src/main.c
 */
#include <base/game.h>
#include <base/setup.h>

/**
 * Entry point. Setup everything and handle cleaning up the game, when it exits
 *
 * @param  [ in]argc 
 * @param  [ in]argv 
 */
int main(int argc, char *argv[]) {
    err erv;

    erv = setupGame(argc, argv);
    ASSERT_TO(erv == ERR_FORCEEXIT, erv = ERR_OK, __ret);
    ASSERT_TO(erv != ERR_OK, erv = erv, __ret);

    erv = ERR_OK;
__ret:
    cleanGame();
    return erv;
}

