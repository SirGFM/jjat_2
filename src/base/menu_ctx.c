#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/menu_ctx.h>
#include <conf/game.h>
#include <jjat2/menu_input.h>
#include <GFraMe/gfmText.h>
#include <string.h>

enum menuDir {
    md_none  = 0x00,
    md_left  = 0x01,
    md_right = 0x02,
    md_up    = 0x04,
    md_down  = 0x08,
};

enum textMode {
    tm_nonselected,
    tm_inactive,
    tm_selected,
};

#define PRESS_DELAY_MS 500
#define HOLD_DELAY_MS  125

static void updateCursorPosition(menuCtx *ctx) {
    int vpos = ctx->vpos;

    switch (ctx->dir) {
    case md_left:
        ctx->hpos[vpos]--;
        break;
    case md_right:
        ctx->hpos[vpos]++;
        break;
    case md_up:
        ctx->vpos--;
        break;
    case md_down:
        ctx->vpos++;
        break;
    }

    if (ctx->vpos < 0)
        ctx->vpos = ctx->vcount - 1;
    else if (ctx->vpos >= ctx->vcount)
        ctx->vpos = 0;
    vpos = ctx->vpos;

    if (ctx->hoptsCount[vpos] == 0)
        ; /* Do nothing */
    else if (ctx->hpos[vpos] < 0)
        ctx->hpos[vpos] = ctx->hoptsCount[vpos] - 1;
    else if (ctx->hpos[vpos] >= ctx->hoptsCount[vpos])
        ctx->hpos[vpos] = 0;

    switch (ctx->dir) {
    case md_left:
    case md_right:
        if (ctx->hposCb && ctx->hoptsCount[vpos] > 0)
            (*ctx->hposCb)(vpos, ctx->hpos[vpos]);
        break;
    case md_up:
    case md_down:
        { /* Do nothing */ }
    }
}

static void handleCursorMovement(menuCtx *ctx) {
#define CHECK_CURSOR(__DIR__) \
    do { \
        if (DID_JUST_PRESS_MENU(__DIR__)) { \
            ctx->dir = md_ ## __DIR__; \
            ctx->delay = PRESS_DELAY_MS; \
            updateCursorPosition(ctx); \
        } \
    } while (0)

    CHECK_CURSOR(up);
    CHECK_CURSOR(down);
    CHECK_CURSOR(left);
    CHECK_CURSOR(right);

#undef CHECK_CURSOR

    if (ctx->dir != md_none && ctx->delay > 0)
        ctx->delay -= game.elapsed;
    else if (ctx->dir != md_none && ctx->delay <= 0) {
        ctx->delay = HOLD_DELAY_MS;
        updateCursorPosition(ctx);
    }
}

err updateMenuCtx(menuCtx *ctx) {
    if (IS_MENU_RELEASED(up) && IS_MENU_RELEASED(down) &&
            IS_MENU_RELEASED(left) && IS_MENU_RELEASED(right))
        ctx->dir = md_none;

    handleCursorMovement(ctx);

    if (DID_JUST_PRESS_MENU(accept)) {
        int hpos = 0;

        // TODO Play accept SFX

        if (ctx->hoptsCount && ctx->hoptsCount[ctx->vpos] > 0)
            hpos = ctx->hpos[ctx->vpos];
        return (*ctx->acceptCb)(ctx->vpos, hpos);
    }

    return ERR_OK;
}

#define drawStrAt(ctx, str, x, y, mode) \
    drawTextAt(ctx, str, sizeof(str) - 1, x, y, mode)

static err drawTextAt(menuCtx *ctx, char *text, int len, int x, int y
        , enum textMode mode) {
    int tile;
    gfmRV rv;

    switch (mode) {
    case tm_selected:
        tile = ctx->activeOffset;
        break;
    case tm_inactive:
        tile = ctx->inactiveOffset;
        break;
    case tm_nonselected:
        tile = ctx->nonSelectedOffset;
        break;
    }

    rv = gfmText_setPosition(ctx->pText, x, y);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setSpriteset(ctx->pText, gfx.pSset8x8, tile);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setText(ctx->pText, text, len, 0/*doCopy*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_forceFinish(ctx->pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_draw(ctx->pText, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

static err drawText(menuCtx *ctx, char *text, int len, int y
        , enum textMode mode) {
    int x;

    x = (V_WIDTH / 8 - len) / 2;

    return drawTextAt(ctx, text, len, x * 8, y * 12, mode);
}

static err drawSuboptions(menuCtx *ctx, char **opts, int count, int x, int y
        , int activeIdx, int yOffset, enum textMode basicMode) {
    int i;
    err erv;

    for (i = 0; i < count; i++) {
        int len = strlen(opts[i]);
        enum textMode mode = basicMode;
        if (i == activeIdx)
            mode++;

        erv = drawTextAt(ctx, opts[i], len, x * 8, y * 12 + yOffset, mode);
        ASSERT(erv == ERR_OK, erv);
        x += len + 1;
    }

    return ERR_OK;
}

err drawMenuCtx(menuCtx *ctx) {
    err erv;
    int _i, y;

    y = V_HEIGHT / 12 - 2;
    for (_i = ctx->vcount; _i > 0; _i--) {
        const int i = _i - 1;
        const int len = strlen(ctx->vopts[i]);
        const int hasMainOpt = (len > 0);
        const int isActive = (i == ctx->vpos);
        const enum textMode subMode = (isActive) ?
                                      tm_inactive :
                                      tm_nonselected;

        if (ctx->hoptsCount[i] != 0) {
            int activeIdx, j, x, sublen;
            /* Skip adding an empty line if there's no main option */
            const int yOffset = (hasMainOpt) ? -2 : 0;

            for (j = 0, x = -1; j < ctx->hoptsCount[i]; j++)
                x += strlen(ctx->hopts[i][j]) + 1;

            if (x < V_WIDTH / 8) {
                x = (V_WIDTH / 8 - x) / 2;
                activeIdx = ctx->hpos[i];

                erv =  drawSuboptions(ctx, ctx->hopts[i], ctx->hoptsCount[i], x
                    , y, activeIdx, yOffset, subMode);
                ASSERT(erv == ERR_OK, erv);
            }
            else {
                int count;

                /* If there are too many options (or if they are too long), only
                 * draw a sub-set of the options and some '...' where needed.
                 * This may lead to a few possibilities:
                 *   - "cur/first next ..."
                 *   - "first cur next ..."
                 *   - "... prev cur next ..."
                 *   - "... prev cur last"
                 *   - "... prev last/cur"
                 */
                activeIdx = 1;
                count = 1;
                j = ctx->hpos[i];
                sublen = strlen(ctx->hopts[i][j]);
                if (j > 0) {
                    sublen += strlen(ctx->hopts[i][j - 1]) + 1;
                    count++;
                }
                else
                    activeIdx = 0;
                if (j < ctx->hoptsCount[i] - 1) {
                    sublen += strlen(ctx->hopts[i][j + 1]) + 1;
                    count++;
                }
                else if (ctx->hoptsCount[i] < 3)
                    activeIdx = ctx->hoptsCount[i] - 1;

                x = (V_WIDTH / 8 - sublen) / 2;

                if (j > 1) {
                    erv = drawStrAt(ctx, "...", (x - 4) * 8
                            , y * 12 + yOffset, subMode);
                    ASSERT(erv == ERR_OK, erv);
                }
                if (j < ctx->hoptsCount[i] - 2) {
                    erv = drawStrAt(ctx, "...", (x + sublen + 1) * 8
                            , y * 12 + yOffset, subMode);
                    ASSERT(erv == ERR_OK, erv);
                }

                if (j > 0)
                    j--;
                erv =  drawSuboptions(ctx, ctx->hopts[i] + j, count, x, y
                        , activeIdx, yOffset, subMode);
                ASSERT(erv == ERR_OK, erv);
            } /* else if (x >= V_WIDTH / 8) */
            y--;
        } /* if (ctx->hoptsCount[i] != 0) */
        if (len > 0) {
            erv = drawText(ctx, ctx->vopts[i], len, y, subMode + 1);
            ASSERT(erv == ERR_OK, erv);
            y--;
        } /* if (len > 0) */
    } /* for (_i = ctx->vcount; _i > 0; _i--) */

    return ERR_OK;
}
