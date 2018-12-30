/**
 * @file src/jjat2/static.c
 *
 * Declare all static variables/contexts.
 */
#include <jjat2/checkpoint.h>
#include <jjat2/fx_group.h>
#include <jjat2/leveltransition.h>
#include <jjat2/hitbox.h>
#include <jjat2/menustate.h>
#include <jjat2/playstate.h>
#include <jjat2/teleport.h>
#include <jjat2/ui.h>

#include <string.h>

/** The checkpoint context */
checkpointCtx checkpoint;

/** The UI context */
uiCtx ui;

/** The hitbox context */
hitboxesCtx hitboxes;

/** Animation for the level transition */
leveltransitionCtx lvltransition;

/** The game's playstate */
playstateCtx playstate;

/** The group of effects/hitboxes */
gfmGroup *fx;

/** The current target (if an entity) */
teleportCtx teleport;

/** The game's menustate */
menustateCtx menustate;

/** Initialize the uninitialized 'local globals' (i.e., the ones defined for the
 * game itself, and not for the template)  with all-zeros. */
void zeroizeGameGlobalCtx() {
    memset(&playstate, 0x0, sizeof(playstateCtx));
    memset(&fx, 0x0, sizeof(gfmGroup*));
    memset(&teleport, 0x0, sizeof(teleportCtx));
    memset(&hitboxes, 0x0, sizeof(hitboxesCtx));
    memset(&lvltransition, 0x0, sizeof(leveltransitionCtx));
    memset(&ui, 0x0, sizeof(uiCtx));
    memset(&checkpoint, 0x0, sizeof(checkpointCtx));
    memset(&menustate, 0x0, sizeof(menustateCtx));
}

