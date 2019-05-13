#include <base/error.h>
#include <base/game.h>
#include <base/menu_ctx.h>
#include <jjat2/menus.h>
#include <jjat2/menustate.h>
#include <string.h>

enum enOptions {
    OPT_FPS = 0
  , OPT_DRAWRATE
  , OPT_SONG
  , OPT_SFX
  , OPT_BACK
  , OPT_COUNT
};

static const char *options[] = {
    [OPT_FPS] "UPDATE RATE (PHYSICS FPS)"
  , [OPT_DRAWRATE] "DRAW RATE (RENDER FPS)"
  , [OPT_SONG] "MUSIC"
  , [OPT_SFX] "SFX"
  , [OPT_BACK] ""
};

#define FPS_OPTIONS \
    X(30) \
    X(50) \
    X(60) \
    X(90) \
    X(120)
static int fpsValue[] = {
#define X(x) x,
    FPS_OPTIONS
#undef X
};
static const char *fps[] = {
#define X(x) #x,
    FPS_OPTIONS
#undef X
};

static const char *volumes[] = {
    "0%"
  , "25%"
  , "50%"
  , "75%"
  , "100%"
};

enum enBack {
    OPT_SAVE = 0
  , OPT_REVERT
};

static const char *back[] = {
    [OPT_SAVE] "SAVE & APPLY"
  , [OPT_REVERT] "REVERT & BACK"
};

static const char **subOptions[] = {
    [OPT_FPS] fps
  , [OPT_DRAWRATE] fps
  , [OPT_SONG] volumes
  , [OPT_SFX] volumes
  , [OPT_BACK] back
};

static const int subOptionsCount[] = {
    [OPT_FPS] getOptsSize(fps)
  , [OPT_DRAWRATE] getOptsSize(fps)
  , [OPT_SONG] getOptsSize(volumes)
  , [OPT_SFX] getOptsSize(volumes)
  , [OPT_BACK] getOptsSize(back)
};

static int subOptionsPosition[OPT_COUNT];

static err applyFps() {
    gfmRV rv;
    int max;
    int fps = fpsValue[subOptionsPosition[OPT_FPS]];
    int dps = fpsValue[subOptionsPosition[OPT_DRAWRATE]];

    max = (fps > dps) ? fps : dps;

    rv = gfm_setFPS(game.pCtx, max);
    if (rv == GFMRV_FPS_TOO_HIGH) {
        rv = gfm_setRawFPS(game.pCtx, max);
    }
    ASSERT(rv == GFMRV_OK, ERR_FPSTOOHIGH);

    rv = gfm_setStateFrameRate(game.pCtx, fps, dps);
    ASSERT(rv == GFMRV_OK, ERR_SETFPS);

    rv = gfm_resetFPS(game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_SETFPS);

    return ERR_OK;
}

/**
 * Apply some options.
 *
 * @param [ in]idx The first index to be applied.
 * @param [ in]count How many values should be applied.
 */
static err applyOptions(int idx, int count) {
    err erv;

    if (idx == OPT_COUNT || idx + count > OPT_COUNT)
        return ERR_TOOMANYOPTIONS;

    for (; count > 0; count--) {
        const int i = idx + count - 1;
        const int val = subOptionsPosition[i];
        switch (i) {
        case OPT_FPS:
        case OPT_DRAWRATE:
            erv = applyFps();
            ASSERT(erv == ERR_OK, erv);
            break;
        case OPT_SONG:
            break;
        case OPT_SFX:
            break;
        case OPT_BACK:
            /* Do nothing */
            break;
        default:
            return ERR_INVALIDOPTION;
        }
    }

    return ERR_OK;
}

static err moveCallback(int vpos, int hpos) {
    return applyOptions(vpos, 1 /* count */);
}

static err optionsCallback(int vpos, int hpos) {
    if (vpos == OPT_BACK) {
        /* TODO Save (or revert) the options */
        switch (hpos) {
        case OPT_SAVE:
            break;
        case OPT_REVERT:
            break;
        }

        return loadMainmenu(&menustate);
    }

    return ERR_OK;
}

err loadOptions(menuCtx *ctx) {
    ctx->vopts = (char**)options;
    ctx->vcount = getOptsSize(options);

    ctx->hopts = (char***)subOptions;
    ctx->hoptsCount = (int*)subOptionsCount;
    /* TODO Load position from file */
    memset(subOptionsPosition, 0x0, sizeof(subOptionsPosition));
    ctx->hpos = (int*)subOptionsPosition;

    ctx->acceptCb = optionsCallback;
    ctx->hposCb = moveCallback;

    ctx->dir = 0;
    ctx->delay = 0;
    ctx->vpos = 0;

    return ERR_OK;
}
