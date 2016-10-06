/**
 * @file src/base/static.c
 *
 * Declare all static variables/contexts.
 */
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>

#include <string.h>

/** Game context */
gameCtx game;
/** Graphics context */
gfxCtx gfx;
/** Input context */
inputCtx input;

/** Initialize the uninitialized globals with all-zeros. */
void zeroizeGlobalCtx() {
    memset(&game, 0x0, sizeof(gameCtx));
    memset(&gfx, 0x0, sizeof(gfxCtx));
    memset(&input, 0x0, sizeof(input));
}

