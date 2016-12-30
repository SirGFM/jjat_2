/**
 * @file src/jjat2/static.c
 *
 * Declare all static variables/contexts.
 */
#include <jjat2/fx_group.h>
#include <jjat2/playstate.h>
#include <jjat2/teleport.h>

#include <string.h>

/** The game's playstate */
playstateCtx playstate;

/** The group of effects/hitboxes */
gfmGroup *fx;

/** The current target (if an entity) */
teleportCtx teleport;

/** Initialize the uninitialized 'local globals' (i.e., the ones defined for the
 * game itself, and not for the template)  with all-zeros. */
void zeroizeGameGlobalCtx() {
    memset(&playstate, 0x0, sizeof(playstateCtx));
    memset(&fx, 0x0, sizeof(gfmGroup*));
    memset(&teleport, 0x0, sizeof(teleportCtx));
}

