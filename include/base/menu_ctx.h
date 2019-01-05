/**
 * @file include/base/menu_ctx.h
 *
 * Generic implementation of a menu.
 */
#ifndef __BASE_MENU_CTX_H__
#define __BASE_MENU_CTX_H__

#include <base/error.h>
#include <conf/game.h>
#include <GFraMe/gfmText.h>

#define DEF_ACTIVE_TILE     0
#define DEF_INACTIVE_TILE   4032

/* NOTE: typeof is a GCC extension... */
#define getOptsSize(__OPTS__) (sizeof(__OPTS__) / sizeof(typeof(__OPTS__[0])))

#define MAX_WIDTH      (V_WIDTH / 8)

struct stMenuCtx {
    /* == "public" ===================== */
    /** Function called when 'accept' is pressed */
    err (*callback)(int vpos, int hpos);
    gfmText *pText;
    /** List of menu options (e.g., "NEW GAME", "EXIT") */
    char **vopts;
    /** List of sub-options, per option (e.g., { { "1P", "2P" } , 0 }) */
    char ***hopts;
    /** Number of sub-options per option (e.g., { 2, 0 }) */
    int *hoptsCount;
    /** Current index within an option's sub-options */
    int *hpos;
    /** Number of options */
    int vcount;
    /** Offset to the first 8x8 bitmap tile for the active text */
    int activeOffset;
    /** Offset to the first 8x8 bitmap tile for the inactive text */
    int inactiveOffset;
    /* == "private" ==================== */
    /** Current direction the cursor is moving to */
    int dir;
    /** Delay until the cursor may automatically move again */
    int delay;
    /** Current position */
    int vpos;
};
typedef struct stMenuCtx menuCtx;

/** Updates the menu */
err updateMenuCtx(menuCtx *ctx);

/** Draws the menu */
err drawMenuCtx(menuCtx *ctx);

#endif /* __BASE_MENU_CTX_H__ */
