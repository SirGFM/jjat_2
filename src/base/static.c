/**
 * @file src/base/static.c
 *
 * Declare all static variables/contexts.
 */
#include <base/collision.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/loadstate.h>
#include <base/sfx.h>

#include <string.h>

/** Game context */
gameCtx game;
/** Graphics context */
gfxCtx gfx;
/** Input context */
inputCtx input;
/** Collision context */
collisionCtx collision;
/** The game's loadstate */
loadstateCtx loadstate;
/** Sound context */
sfxCtx sfx;

/** Initialize the uninitialized globals with all-zeros. */
void zeroizeGlobalCtx() {
    memset(&collision, 0x0, sizeof(collisionCtx));
    memset(&game, 0x0, sizeof(gameCtx));
    memset(&gfx, 0x0, sizeof(gfxCtx));
    memset(&input, 0x0, sizeof(inputCtx));
    memset(&loadstate, 0x0, sizeof(loadstateCtx));
    memset(&sfx, 0x0, sizeof(sfxCtx));
}

