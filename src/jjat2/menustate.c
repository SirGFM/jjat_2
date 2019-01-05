#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <base/menu_ctx.h>
#include <conf/game.h>
#include <jjat2/menu_input.h>
#include <jjat2/menustate.h>
#include <jjat2/util.h>
#include <GFraMe/gfmDebug.h>
#include <GFraMe/gfmText.h>
#include <string.h>

enum enOptions {
    OPT_NEWGAME = 0
  , OPT_EXIT
  , OPT_COUNT
};

static const char *options[] = {
    [OPT_NEWGAME] "NEW GAME"
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
    [OPT_NEWGAME] (const char**)newGameSub
  , [OPT_EXIT] 0
};

static const int subOptionsCount[] = {
    [OPT_NEWGAME] getOptsSize(newGameSub)
  , [OPT_EXIT] 0
};

static int subOptionsPosition[OPT_COUNT];

/** Initialize the menustate. */
err initMenustate() {
    gfmRV rv;

    rv = gfmText_getNew(&menustate.pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_init(menustate.pText, 0/*x*/, 0/*y*/, MAX_WIDTH, 1/*maxLines*/
        , 0/*delay*/, 0/*bindToWorld*/, gfx.pSset8x8, DEF_ACTIVE_TILE);
    menustate.activeOffset = DEF_ACTIVE_TILE;
    menustate.inactiveOffset = DEF_INACTIVE_TILE;

    return ERR_OK;
}

/** If the menustate has been initialized, properly free it up. */
void freeMenustate() {
    if (menustate.pText != 0)
        gfmText_free(&menustate.pText);
}


static err menustateCallback(int vpos, int hpos) {
    gfmRV rv;
    err erv;

    switch (vpos) {
    case OPT_NEWGAME:
        game.nextState = ST_PLAYSTATE;
        switch (hpos) {
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
    case OPT_EXIT:
        rv = gfm_setQuitFlag(game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    return ERR_OK;
}

/** Setup the loadstate so it may start to be executed */
err loadMenustate() {
    err erv;

    erv = setDefaultMenuInput();
    ASSERT(erv == ERR_OK, erv);

    menustate.vopts = (char**)options;
    menustate.vcount = getOptsSize(options);

    menustate.hopts = (char***)subOptions;
    menustate.hoptsCount = (int*)subOptionsCount;
    memset(subOptionsPosition, 0x0, sizeof(subOptionsPosition));
    menustate.hpos = (int*)subOptionsPosition;

    menustate.callback = menustateCallback;

    menustate.dir = 0;
    menustate.delay = 0;
    menustate.vpos = 0;

    return erv;
}

err updateMenustate() {
    return updateMenuCtx(&menustate);
}

/** Draw the menustate */
err drawMenustate() {
    return drawMenuCtx(&menustate);
}
