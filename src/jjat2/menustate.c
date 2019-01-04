#include <base/error.h>
#include <base/game.h>
#include <base/gfx.h>
#include <base/input.h>
#include <conf/game.h>
#include <jjat2/menu_input.h>
#include <jjat2/menustate.h>
#include <jjat2/util.h>
#include <GFraMe/gfmDebug.h>
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
#define MAX_WIDTH      (V_WIDTH / 8)
#define ACTIVE_TILE    0
#define INACTIVE_TILE    4032

/* NOTE: typeof is a GCC extension... */
#define getOptsSize(__OPTS__) (sizeof(__OPTS__) / sizeof(typeof(__OPTS__[0])))

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
        , 0/*delay*/, 0/*bindToWorld*/, gfx.pSset8x8, ACTIVE_TILE);

    return ERR_OK;
}

/** If the menustate has been initialized, properly free it up. */
void freeMenustate() {
    if (menustate.pText != 0)
        gfmText_free(&menustate.pText);
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

    menustate.dir = 0;
    menustate.delay = 0;
    menustate.vpos = 0;

    return erv;
}

static void updateCursorPosition() {
    int vpos = menustate.vpos;

    switch (menustate.dir) {
    case md_left:
        menustate.hpos[vpos]--;
        break;
    case md_right:
        menustate.hpos[vpos]++;
        break;
    case md_up:
        menustate.vpos--;
        break;
    case md_down:
        menustate.vpos++;
        break;
    }

    if (menustate.vpos < 0)
        menustate.vpos = menustate.vcount - 1;
    else if (menustate.vpos >= menustate.vcount)
        menustate.vpos = 0;
    vpos = menustate.vpos;

    if (menustate.hoptsCount[vpos] == 0)
        ; /* Do nothing */
    else if (menustate.hpos[vpos] < 0)
        menustate.hpos[vpos] = menustate.hoptsCount[vpos] - 1;
    else if (menustate.hpos[vpos] >= menustate.hoptsCount[vpos])
        menustate.hpos[vpos] = 0;
}

static void handleCursorMovement() {
#define CHECK_CURSOR(__DIR__) \
    do { \
        if (DID_JUST_PRESS_MENU(__DIR__)) { \
            menustate.dir = md_ ## __DIR__; \
            menustate.delay = PRESS_DELAY_MS; \
            updateCursorPosition(); \
        } \
    } while (0)

    CHECK_CURSOR(up);
    CHECK_CURSOR(down);
    CHECK_CURSOR(left);
    CHECK_CURSOR(right);

#undef CHECK_CURSOR

    if (menustate.dir != md_none && menustate.delay > 0)
        menustate.delay -= game.elapsed;
    else if (menustate.dir != md_none && menustate.delay <= 0) {
        menustate.delay = HOLD_DELAY_MS;
        updateCursorPosition();
    }
}

/** Update the menustate */
err updateMenustate() {
    gfmRV rv;

    if (IS_MENU_RELEASED(up) && IS_MENU_RELEASED(down) &&
            IS_MENU_RELEASED(left) && IS_MENU_RELEASED(right))
        menustate.dir = md_none;

    handleCursorMovement();

    if (DID_JUST_PRESS_MENU(accept)) {
        err erv;

        // TODO Play accept SFX

        switch (menustate.vpos) {
        case OPT_NEWGAME:
            game.nextState = ST_PLAYSTATE;
            switch (menustate.hpos[OPT_NEWGAME]) {
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
    }

    return ERR_OK;
}

static err drawTextAt(char *text, int len, int x, int y, int active) {
    int tile;
    gfmRV rv;

    if (active)
        tile = ACTIVE_TILE;
    else
        tile = INACTIVE_TILE;

    rv = gfmText_setPosition(menustate.pText, x * 8, y * 8);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setSpriteset(menustate.pText, gfx.pSset8x8, tile);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_setText(menustate.pText, text, len, 0/*doCopy*/);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_forceFinish(menustate.pText);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    rv = gfmText_draw(menustate.pText, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    return ERR_OK;
}

static err drawText(char *text, int len, int y, int active) {
    int x;

    x = (V_WIDTH / 8 - len) / 2;

    return drawTextAt(text, len, x, y, active);
}

/** Draw the menustate */
err drawMenustate() {
    err erv;
    int i, y;

    gfmDebug_printf(game.pCtx, 0, 128, "  DIR: %i", menustate.dir);
    gfmDebug_printf(game.pCtx, 0, 136, "DELAY: %i", menustate.delay);
    gfmDebug_printf(game.pCtx, 0, 144, " VPOS: %i", menustate.vpos);
    gfmDebug_printf(game.pCtx, 0, 152, " HPOS: %i", menustate.hpos);

    y = V_HEIGHT / 8 - menustate.vcount - 2;
    /* Add another row for each horizontal option */
    for (i = 0; i < menustate.vcount; i++)
        if (menustate.hoptsCount[i] != 0)
            y--;

    for (i = 0; i < menustate.vcount; i++) {
        int active = (i == menustate.vpos);
        int len = strlen(menustate.vopts[i]);

        erv = drawText(menustate.vopts[i], len, y + i, active);
        ASSERT(erv == ERR_OK, erv);
        if (menustate.hoptsCount[i] != 0) {
            int j, x;

            for (j = 0, x = -1; j < menustate.hoptsCount[i]; j++)
                x += strlen(menustate.hopts[i][j]) + 1;
            x = (V_WIDTH / 8 - x) / 2;

            y++;
            for (j = 0; j < menustate.hoptsCount[i]; j++) {
                char *text = menustate.hopts[i][j];

                active = (j == menustate.hpos[i]);
                len = strlen(text);

                erv = drawTextAt(text, len, x, y + i, active);
                ASSERT(erv == ERR_OK, erv);
                x += len + 1;
            }
        }
    }

    return ERR_OK;
}
