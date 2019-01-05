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

static err drawTextAt(menuCtx *ctx, char *text, int len, int x, int y
        , int active) {
    int tile;
    gfmRV rv;

    if (active)
        tile = ctx->activeOffset;
    else
        tile = ctx->inactiveOffset;

    rv = gfmText_setPosition(ctx->pText, x * 8, y * 8);
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

static err drawText(menuCtx *ctx, char *text, int len, int y, int active) {
    int x;

    x = (V_WIDTH / 8 - len) / 2;

    return drawTextAt(ctx, text, len, x, y, active);
}

err drawMenuCtx(menuCtx *ctx) {
    err erv;
    int i, y;

    y = V_HEIGHT / 8 - ctx->vcount - 2;
    /* Add another row for each horizontal option */
    for (i = 0; i < ctx->vcount; i++)
        if (ctx->hoptsCount[i] != 0)
            y--;

    for (i = 0; i < ctx->vcount; i++) {
        int active = (i == ctx->vpos);
        int len = strlen(ctx->vopts[i]);

        erv = drawText(ctx, ctx->vopts[i], len, y + i, active);
        ASSERT(erv == ERR_OK, erv);
        if (ctx->hoptsCount[i] != 0) {
            int j, x;

            for (j = 0, x = -1; j < ctx->hoptsCount[i]; j++)
                x += strlen(ctx->hopts[i][j]) + 1;
            x = (V_WIDTH / 8 - x) / 2;

            y++;
            for (j = 0; j < ctx->hoptsCount[i]; j++) {
                char *text = ctx->hopts[i][j];

                active = (j == ctx->hpos[i]);
                len = strlen(text);

                erv = drawTextAt(ctx, text, len, x, y + i, active);
                ASSERT(erv == ERR_OK, erv);
                x += len + 1;
            }
        }
    }

    return ERR_OK;
}
