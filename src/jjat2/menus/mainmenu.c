#include <base/error.h>
#include <base/game.h>
#include <base/menu_ctx.h>
#include <jjat2/util.h>
#include <jjat2/menus.h>
#include <jjat2/menustate.h>
#include <string.h>

enum enOptions {
    OPT_GAMEMODE = 0
  , OPT_NEWGAME
  , OPT_OPTIONS
  , OPT_EXIT
  , OPT_COUNT
};

static const char *options[] = {
    [OPT_NEWGAME] "NEW GAME"
  , [OPT_GAMEMODE] = "GAME MODE"
  , [OPT_OPTIONS] = "OPTIONS"
  , [OPT_EXIT] "EXIT"
};

enum enNewGameOpts {
    NGOPT_1P2C
  , NGOPT_2P
  , NGOPT_1P1C
};

static const char *newGameSub[] = {
    [NGOPT_1P2C] "1P2C"
  , [NGOPT_2P] "2P"
  , [NGOPT_1P1C] "1P1C"
};

static const char **subOptions[] = {
    [OPT_GAMEMODE] (const char**)newGameSub
  , [OPT_NEWGAME] 0
  , [OPT_OPTIONS] 0
  , [OPT_EXIT] 0
};

static const int subOptionsCount[] = {
    [OPT_GAMEMODE] getOptsSize(newGameSub)
  , [OPT_NEWGAME] 0
  , [OPT_OPTIONS] 0
  , [OPT_EXIT] 0
};

static int subOptionsPosition[OPT_COUNT];

static err mainmenuCallback(int vpos, int hpos) {
    gfmRV rv;
    err erv;
    int gamemode = subOptionsPosition[OPT_GAMEMODE];

    switch (vpos) {
    case OPT_NEWGAME:
        game.nextState = ST_PLAYSTATE;
        switch (gamemode) {
        case NGOPT_1P2C:
            game.flags |= AC_BOTH;
            erv = staticSetInputStr("SL:6005200b01;SR:6105300d;SJ:5a00214;SA:5b00c;GL:64047;GR:65048;GJ:5d049;GA:5e04a;P:68042;SW:6904344;");
            break;
        case NGOPT_2P:
            game.flags |= AC_BOTH;
            erv = staticSetInputStr("SL:600520;SR:610530;SJ:560;SA:570580;GL:60152147;GR:61153148;GJ:56115;GA:57158116;P:68068142;SW:5905914344;");
            break;
        case NGOPT_1P1C:
            game.flags |= AC_SWORDY;
            game.flags &= ~AC_GUNNY;
            erv = staticSetInputStr("SL:60052047;SR:61053048;SJ:56015;SA:57058016;P:68042;SW:5904344;");
            break;
        }
        ASSERT(erv == ERR_OK, erv);
        break;
    case OPT_OPTIONS:
        return loadOptions(&menustate);
        break;
    case OPT_EXIT:
        rv = gfm_setQuitFlag(game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        break;
    }

    return ERR_OK;
}

err loadMainmenu(menuCtx *ctx) {
    ctx->vopts = (char**)options;
    ctx->vcount = getOptsSize(options);

    ctx->hopts = (char***)subOptions;
    ctx->hoptsCount = (int*)subOptionsCount;
    memset(subOptionsPosition, 0x0, sizeof(subOptionsPosition));
    ctx->hpos = (int*)subOptionsPosition;

    ctx->acceptCb = mainmenuCallback;
    ctx->hposCb = 0;

    ctx->dir = 0;
    ctx->delay = 0;
    ctx->vpos = 0;

    return ERR_OK;
}
