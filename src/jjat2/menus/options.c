#include <base/error.h>
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

static const char *fps[] = {
    "30"
  , "50"
  , "60"
  , "90"
  , "120"
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

static err moveCallback(int vpos, int hpos) {
    return ERR_OK;
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
