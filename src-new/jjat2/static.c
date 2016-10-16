/**
 * @file src/jjat2/static.c
 *
 * Declare all static variables/contexts.
 */
#include <jjat2/playstate.h>

#include <string.h>

/** The game's playstate */
playstateCtx playstate;

/** Initialize the uninitialized 'local globals' (i.e., the ones defined for the
 * game itself, and not for the template)  with all-zeros. */
void zeroizeGameGlobalCtx() {
    memset(&playstate, 0x0, sizeof(playstateCtx));
}

