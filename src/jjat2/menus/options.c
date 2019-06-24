#include <base/error.h>
#include <base/game.h>
#include <base/menu_ctx.h>
#include <base/sfx.h>
#include <conf/config.h>
#include <jjat2/menus.h>
#include <jjat2/menustate.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <GFraMe/gframe.h>
#include <GFraMe/core/gfmAudio_bkend.h>

extern void *displayList;
extern configCtx glConfig;

/* List of every option possible:
 *
 * Display:
 *   + Windowed Resolution (x1, x2, 3x, ...)
 *   + Fullscreen mode (640x480@60FPS, ...)
 *   + Fullscreen?
 *   + Advanced
 *       - Backend
 *       - VSync
 *       - Back
 *   + Back
 * Audio:
 *   + Song volume
 *   + SFX volume
 *   + Advanced
 *       - Quality (mono@11kHz, stereo@22kHz, stereo@44kHz)
 *       - Loading mode (preload, background, when needed)
 *       - Back
 *   + Back
 * Game:
 *   + Show timer
 *   + Teleport on Press / Teleport on Release
 *   + Hold to remove target
 *   + Advanced
 *       - Physics FPS
 *       - Display FPS
 *       - Debug mode (allow stepping)
 *       - Back
 *   + Back
 * Custom controls:
 *   + Active profile (p1, p2, p3, ...)
 *   + Configure profile 1
 *   + Configure profile 2
 *   + ...
 *   + Back
 */

#define GETOPTSSIZE(VAL, NAME, SUBOPT) \
    [VAL] = getOptsSize(SUBOPT),
#define FIRST_ENUM(VAL, ...) \
    VAL = 0,
#define OTHER_ENUMS(VAL, ...) \
    VAL,
#define MACRO2ZERO(VAL, ...) \
    [VAL] = 0,
#define MACRO2DICT(VAL, NAME, ...) \
    [VAL] = NAME,
#define OPT_NOOP(...)
#define MACRO2SUBOPT(VAL, NAME, SUBOPT) \
    [VAL] = SUBOPT,
#define MACRO2VAL(VAL, NAME, ...) \
    VAL

#define CREATE_SUBOPTS(type, LIST) \
    enum type ## _enum { \
        LIST(FIRST_ENUM, FIRST_ENUM, OTHER_ENUMS, OTHER_ENUMS, OTHER_ENUMS) \
    }; \
    static const char *type[] = { \
        LIST(MACRO2DICT, MACRO2DICT, MACRO2DICT, MACRO2DICT, OPT_NOOP) \
    }

#define CREATE_OPTS(type, LIST, CONST) \
    CREATE_SUBOPTS(type, LIST); \
    static CONST char ** type ## _subopts[] = { \
        LIST(MACRO2SUBOPT, MACRO2ZERO, MACRO2SUBOPT, MACRO2ZERO, OPT_NOOP) \
    }; \
    static CONST int type ## _count[] = { \
        LIST(GETOPTSSIZE, MACRO2ZERO, GETOPTSSIZE, MACRO2ZERO, OPT_NOOP) \
    }; \
    static int type ## _pos[LIST(OPT_NOOP, OPT_NOOP, OPT_NOOP, OPT_NOOP, MACRO2VAL)]; \
    static int type ## _bkup[LIST(OPT_NOOP, OPT_NOOP, OPT_NOOP, OPT_NOOP, MACRO2VAL)]

#define LOAD_OPTS(type) \
    ctx->vopts = (char**)type; \
    ctx->vcount = getOptsSize(type); \
    ctx->hopts = (char***)type ## _subopts; \
    ctx->hoptsCount = (int*)type ## _count; \
    ctx->hpos = (int*)type ## _pos; \
    memcpy(type ## _bkup, type ## _pos, sizeof(type ## _bkup))

#define REVERT_OPTS(type) \
    memcpy(type ## _pos, type ## _bkup, sizeof(type ## _bkup)) \

#define __APPLY_OPTS(type, apply_enum, menu_enum, menu_count) \
    do { \
        if (type  ## _pos[apply_enum] == OPTS_APPLY) \
            return applyOptions(menu_enum, 0, menu_count); \
        else { \
            memcpy(type ## _bkup, type ## _pos, sizeof(type ## _bkup)); \
            return applyOptions(menu_enum, 0, menu_count); \
        } \
    } while (0)
#define APPLY_OPTS(type, MENU) \
    __APPLY_OPTS(type, OPT_ ## MENU ## _APPLY, MENU ## _OPTIONS, OPT_ ## MENU ## _COUNT)

#define YESNO_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_YES, "YES") \
    Ysimple(OPTS_NO, "NO") \
    Z(OPTS_YESNO_COUNT, "")

#define APPLY_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_APPLY, "APPLY") \
    Ysimple(OPTS_REVERT, "REVERT") \
    Z(OPTS_APPLY_COUNT, "")

#define APPLYRESTART_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_APPLYRT, "APPLY & RESTART") \
    Ysimple(OPTS_REVERTRT, "REVERT") \

#define WNDRES_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPT_WNDRES_1X, "1X") \
    Ysimple(OPT_WNDRES_2X, "2X") \
    Ysimple(OPT_WNDRES_3X, "3X") \
    Ysimple(OPT_WNDRES_4X, "4X") \
    Z(OPT_WNDRES_COUNT, "")

#define FULLSCREEN_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_FULLSCREEN_WINDOWED, "WINDOWED") \
    Ysimple(OPTS_FULLSCREEN_FULLSCREEN, "FULLSCREEN") \
    Z(OPTS_FULLSCREEN_COUNT, "")

#define BACKEND_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_BACKEND_SW, "SOFTWARE") \
    Ysimple(OPTS_BACKEND_SDL, "SDL") \
    Ysimple(OPTS_BACKEND_OPENGL, "OPENGL") \
    Z(OPTS_BACKEND_COUNT, "")

#define SFXQUALITY_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_SFXQUALITY_MONO11KHZ, "MONO@11KHZ") \
    Ysimple(OPTS_SFXQUALITY_STEREO22KHZ, "STEREO@22KHZ") \
    Ysimple(OPTS_SFXQUALITY_STEREO44KHZ, "STEREO@44KHZ") \
    Z(OPTS_SFXQUALITY_COUNT, "")

#define SFXLOADING_SUBOPTS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPTS_SFXLOADING_BACKGROUND, "BACKGROUND") \
    Ysimple(OPTS_SFXLOADING_BLOCKING, "BLOCKING") \
    Z(OPTS_SFXLOADING_COUNT, "") \

#define MAIN_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    Xsimple(OPT_DISPLAY, "DISPLAY") \
    Ysimple(OPT_AUDIO, "AUDIO") \
    Ysimple(OPT_GAME, "GAME") \
    /* Ysimple(OPT_CONTROLS, "CONTROLS") */ \
    Ysimple(OPT_BACK, "BACK") \
    Z(OPT_COUNT, "")

static const char *dummy[] = {};
#define GFX_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_GFX_WND_RES, "WINDOWED RESOLUTION", wndRes) \
    Y(OPT_GFX_FULL_MODE, "FULLSCREEN MODE", dummy) \
    Y(OPT_GFX_SET_FULLSCREEN, "", fullscreen) \
    Y(OPT_GFX_APPLY, "", apply) \
    Ysimple(OPT_GFX_ADVANCED, "ADVANCED") \
    Ysimple(OPT_GFX_BACK, "BACK") \
    Z(OPT_GFX_COUNT, "")

#define ADVGFX_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_ADVGFX_BACKEND, "BACKEND", gfxBackend) \
    Y(OPT_ADVGFX_VSYNC, "VSYNC", yesNo) \
    Y(OPT_ADVGFX_SIMPLE, "SIMPLE RENDERING", yesNo) \
    Y(OPT_ADVGFX_APPLY, "", applyRestart) \
    Ysimple(OPT_ADVGFX_BACK, "BACK") \
    Z(OPT_ADVGFX_COUNT, "")

#define SFX_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_SFX_MUSIC, "MUSIC", volumes) \
    Y(OPT_SFX_SOUND_FX, "SOUND FX", volumes) \
    Ysimple(OPT_SFX_ADVANCED, "ADVANCED") \
    Ysimple(OPT_SFX_BACK, "BACK") \
    Z(OPT_SFX_COUNT, "")

#define ADVSFX_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_ADVSFX_QUALITY, "QUALITY", sfxQuality) \
    Y(OPT_ADVSFX_LOADING, "LOADING MODE", sfxLoading) \
    Y(OPT_ADVSFX_APPLY, "", applyRestart) \
    Ysimple(OPT_ADVSFX_BACK, "BACK") \
    Z(OPT_ADVSFX_COUNT, "")

#define GAME_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_GAME_ASYNC, "2 PLAYER MODE", yesNo) \
    Y(OPT_GAME_TIMER, "TIMER", yesNo) \
    Ysimple(OPT_GAME_ADVANCED, "ADVANCED") \
    Ysimple(OPT_GAME_BACK, "BACK") \
    Z(OPT_GAME_COUNT, "")

#define ADVGAME_OPTIONS(X, Xsimple, Y, Ysimple, Z) \
    X(OPT_ADVGAME_FPS, "UPDATE RATE (PHYSICS FPS)", fps) \
    Y(OPT_ADVGAME_DRAWRATE, "DRAW RATE (RENDER FPS)", fps) \
    Y(OPT_ADVGAME_DEBUG, "DEBUG MODE", yesNo) \
    Ysimple(OPT_ADVGAME_BACK, "BACK") \
    Z(OPT_ADVGAME_COUNT, "")

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

CREATE_SUBOPTS(yesNo, YESNO_SUBOPTS);
CREATE_SUBOPTS(apply, APPLY_SUBOPTS);
CREATE_SUBOPTS(applyRestart, APPLYRESTART_SUBOPTS);
CREATE_SUBOPTS(wndRes, WNDRES_SUBOPTS);
CREATE_SUBOPTS(fullscreen, FULLSCREEN_SUBOPTS);
CREATE_SUBOPTS(gfxBackend, BACKEND_SUBOPTS);
CREATE_SUBOPTS(sfxQuality, SFXQUALITY_SUBOPTS);
CREATE_SUBOPTS(sfxLoading, SFXLOADING_SUBOPTS);
CREATE_OPTS(optionsMenu, MAIN_OPTIONS, const);
CREATE_OPTS(gfxMenu, GFX_OPTIONS, /* not const */);
CREATE_OPTS(advGfxMenu, ADVGFX_OPTIONS, const);
CREATE_OPTS(sfxMenu, SFX_OPTIONS, const);
CREATE_OPTS(advSfxMenu, ADVSFX_OPTIONS, const);
CREATE_OPTS(gameMenu, GAME_OPTIONS, const);
CREATE_OPTS(advGameMenu, ADVGAME_OPTIONS, const);

enum enCurMenu {
    MAIN_OPTIONS = 0
  , GFX_OPTIONS
  , ADVGFX_OPTIONS
  , SFX_OPTIONS
  , ADVSFX_OPTIONS
  , GAME_OPTIONS
  , ADVGAME_OPTIONS
};
static enum enCurMenu curMenu;

static err load(menuCtx *ctx, enum enCurMenu _curMenu) {
    switch (_curMenu) {
    case MAIN_OPTIONS:
        LOAD_OPTS(optionsMenu);
        break;
    case GFX_OPTIONS:
        LOAD_OPTS(gfxMenu);
        break;
    case ADVGFX_OPTIONS:
        LOAD_OPTS(advGfxMenu);
        break;
    case SFX_OPTIONS:
        LOAD_OPTS(sfxMenu);
        break;
    case ADVSFX_OPTIONS:
        LOAD_OPTS(advSfxMenu);
        break;
    case GAME_OPTIONS:
        LOAD_OPTS(gameMenu);
        break;
    case ADVGAME_OPTIONS:
        LOAD_OPTS(advGameMenu);
        break;
    default:
        return ERR_UNHANDLED_MENU;
    }
    ctx->dir = 0;
    ctx->delay = 0;
    ctx->vpos = 0;
    curMenu = _curMenu;

    return ERR_OK;
}

static err back() {
    switch (curMenu) {
    case MAIN_OPTIONS:
        return loadMainmenu(&menustate);
    case GFX_OPTIONS:
        curMenu = MAIN_OPTIONS;
        break;
    case ADVGFX_OPTIONS:
        curMenu = GFX_OPTIONS;
        break;
    case SFX_OPTIONS:
        curMenu = MAIN_OPTIONS;
        break;
    case ADVSFX_OPTIONS:
        curMenu = SFX_OPTIONS;
        break;
    case GAME_OPTIONS:
        curMenu = MAIN_OPTIONS;
        break;
    case ADVGAME_OPTIONS:
        curMenu = GAME_OPTIONS;
        break;
    default:
        return ERR_UNHANDLED_MENU;
    }
    return load(&menustate, curMenu);
}

static err applyFps() {
    gfmRV rv;
    int max;
    int fps = fpsValue[advGameMenu_pos[OPT_ADVGAME_FPS]];
    int dps = fpsValue[advGameMenu_pos[OPT_ADVGAME_DRAWRATE]];

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

static err applyGfx() {
    gfmRV rv;
    err erv;
    int zoom = 0;

    switch ((enum fullscreen_enum)gfxMenu_pos[OPT_GFX_SET_FULLSCREEN]) {
    case OPTS_FULLSCREEN_WINDOWED:
        rv = gfm_setResolution(game.pCtx, 0);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfm_setWindowed(game.pCtx);
        ASSERT(rv == GFMRV_OK || rv == GFMRV_WINDOW_MODE_UNCHANGED, ERR_GFMERR);
        switch ((enum wndRes_enum)gfxMenu_pos[OPT_GFX_WND_RES]) {
        case OPT_WNDRES_1X:
            zoom = 1;
            break;
        case OPT_WNDRES_2X:
            zoom = 2;
            break;
        case OPT_WNDRES_3X:
            zoom = 3;
            break;
        case OPT_WNDRES_4X:
            zoom = 4;
            break;
        case OPT_WNDRES_COUNT: {}
        }
        rv = gfm_setDimensions(game.pCtx, V_WIDTH * zoom, V_HEIGHT * zoom);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        glConfig.wndWidth = V_WIDTH * zoom;
        glConfig.wndHeight = V_HEIGHT * zoom;
        glConfig.flags &= ~CFG_FULLSCREEN;
        break;
    case OPTS_FULLSCREEN_FULLSCREEN:
        rv = gfm_setDimensions(game.pCtx, V_WIDTH, V_HEIGHT);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfm_setResolution(game.pCtx, gfxMenu_pos[OPT_GFX_FULL_MODE]);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfm_setFullscreen(game.pCtx);
        ASSERT(rv == GFMRV_OK || rv == GFMRV_WINDOW_MODE_UNCHANGED, ERR_GFMERR);

        glConfig.fullscreenResolution = gfxMenu_pos[OPT_GFX_FULL_MODE];
        glConfig.flags |= CFG_FULLSCREEN;
        break;
    case OPTS_FULLSCREEN_COUNT: {}
    }

    return ERR_OK;
}

static err applyAdvGfx() {
    if (advGfxMenu_pos[OPT_ADVGFX_VSYNC] == OPTS_YES)
        glConfig.flags |= CFG_VSYNC;
    else
        glConfig.flags &= ~CFG_VSYNC;

    if (advGfxMenu_pos[OPT_ADVGFX_SIMPLE] == OPTS_YES)
        glConfig.flags |= CFG_SIMPLEDRAW;
    else
        glConfig.flags &= ~CFG_SIMPLEDRAW;

    switch ((enum gfxBackend_enum)advGfxMenu_pos[OPT_ADVGFX_BACKEND]) {
    case OPTS_BACKEND_SW:
        glConfig.videoBackend = GFM_VIDEO_SWSDL2;
        break;
    case OPTS_BACKEND_SDL:
        glConfig.videoBackend = GFM_VIDEO_SDL2;
        break;
    case OPTS_BACKEND_OPENGL:
        glConfig.videoBackend = GFM_VIDEO_GL3;
        break;
    case OPTS_BACKEND_COUNT: {}
    }

    glConfig.flags |= CFG_RESTART;
    return ERR_RESTART;
}

static err applyAdvSfx() {
    switch((enum sfxQuality_enum)advSfxMenu_pos[OPT_ADVSFX_QUALITY]) {
    case OPTS_SFXQUALITY_MONO11KHZ:
        glConfig.audioSettings = gfmAudio_lowQuality;
        break;
    case OPTS_SFXQUALITY_STEREO22KHZ:
        glConfig.audioSettings = gfmAudio_medQuality;
        break;
    case OPTS_SFXQUALITY_STEREO44KHZ:
        glConfig.audioSettings = gfmAudio_defQuality;
        break;
   case OPTS_SFXQUALITY_COUNT: {}
    }

    switch((enum sfxLoading_enum)advSfxMenu_pos[OPT_ADVSFX_LOADING]) {
    case OPTS_SFXLOADING_BACKGROUND:
        glConfig.flags |= CFG_LAZYLOAD;
        break;
    case OPTS_SFXLOADING_BLOCKING:
        glConfig.flags &= ~CFG_LAZYLOAD;
        break;
    case OPTS_SFXLOADING_COUNT: {}
    }

    glConfig.flags |= CFG_RESTART;
    return ERR_RESTART;
}

/**
 * Apply some options for a given sub-menu.
 *
 * @param [ in]menu The menu to have its options applied.
 * @param [ in]idx The first index to be applied.
 * @param [ in]count How many values should be applied.
 */
static err applyOptions(enum enCurMenu menu, int idx, int count) {
    err erv;
    int menuMaxCount;

    switch (menu) {
    case MAIN_OPTIONS:
        /* Nothing to be applied. */
        return ERR_OK;
    case SFX_OPTIONS:
        menuMaxCount = OPT_SFX_COUNT;
        break;
    case GAME_OPTIONS:
        menuMaxCount = OPT_GAME_COUNT;
        break;
    case ADVGAME_OPTIONS:
        menuMaxCount = OPT_ADVGAME_COUNT;
        break;
    default:
        return ERR_UNHANDLED_MENU;
    }
    if (idx == menuMaxCount || idx + count > menuMaxCount)
        return ERR_TOOMANYOPTIONS;

    /* XXX: Pay extra attention here, as some of the menus have options which
     * must be ignored (e.g., "APPLY/REVERT"). */
    for (; count > 0; count--) {
        const int i = idx + count - 1;
        switch (menu) {
        case SFX_OPTIONS: {
            const int val = sfxMenu_pos[i];
            switch (i) {
            case OPT_SFX_MUSIC:
                erv = setSongVolume((volume)val);
                break;
            case OPT_SFX_SOUND_FX:
                erv = setSfxVolume((volume)val);
                break;
            default: {}
            }
        } break;
        case GAME_OPTIONS: {
            const int val = gameMenu_pos[i];
            switch (i) {
            case OPT_GAME_ASYNC:
                /* TODO */
            case OPT_GAME_TIMER:
                /* TODO */
            default: {}
            }
        } break;
        case ADVGAME_OPTIONS: {
            switch (i) {
            case OPT_ADVGAME_FPS:
            case OPT_ADVGAME_DRAWRATE:
                erv = applyFps();
                break;
            case OPT_ADVGAME_DEBUG:
                /* TODO */
            default: {}
            }
        } break;
        default: { /* XXX: Already handled, but ignores warnings. */ }
        } /* switch (menu) */

        ASSERT(erv == ERR_OK, erv);
    } /* for (; count > 0; count--) */

    return ERR_OK;
}

static err moveCallback(int vpos, int hpos) {
    switch (curMenu) {
    case MAIN_OPTIONS:
        /* Does nothing. */
        return ERR_OK;
    case GFX_OPTIONS:
    case ADVGFX_OPTIONS:
    case ADVSFX_OPTIONS:
        /* Does nothing, except on accept apply. */
        return ERR_OK;
    case SFX_OPTIONS:
        return applyOptions(SFX_OPTIONS, vpos, 1 /* count */);
    case GAME_OPTIONS:
        return applyOptions(GAME_OPTIONS, vpos, 1 /* count */);
    case ADVGAME_OPTIONS:
        return applyOptions(ADVGAME_OPTIONS, vpos, 1 /* count */);
    default:
        return ERR_UNHANDLED_MENU;
    }
}

static err acceptCallback(int vpos, int hpos) {
    switch (curMenu) {
    case MAIN_OPTIONS:
        switch (vpos) {
        case OPT_DISPLAY:
            return load(&menustate, GFX_OPTIONS);
        case OPT_AUDIO:
            return load(&menustate, SFX_OPTIONS);
        case OPT_GAME:
            return load(&menustate, GAME_OPTIONS);
        #if 0
        case OPT_CONTROLS:
        #endif
        case OPT_BACK:
            return back();
        }
        break;
    case GFX_OPTIONS:
        switch (vpos) {
        case OPT_GFX_APPLY:
            if (gfxMenu_pos[OPT_GFX_APPLY] == OPTS_REVERT)
                REVERT_OPTS(gfxMenu);
            return applyGfx();
        case OPT_GFX_ADVANCED:
            return load(&menustate, ADVGFX_OPTIONS);
        case OPT_GFX_BACK:
            return back();
        }
        break;
    case ADVGFX_OPTIONS:
        switch (vpos) {
        case OPT_ADVGFX_APPLY:
            if (advGfxMenu_pos[OPT_ADVGFX_APPLY] == OPTS_REVERT) {
                REVERT_OPTS(advGfxMenu);
                return ERR_OK;
            }
            return applyAdvGfx();
        case OPT_ADVGFX_BACK:
            return back();
        }
        break;
    case SFX_OPTIONS:
        switch (vpos) {
        case OPT_SFX_ADVANCED:
            return load(&menustate, ADVSFX_OPTIONS);
        case OPT_SFX_BACK:
            return back();
        }
        break;
    case ADVSFX_OPTIONS:
        switch (vpos) {
        case OPT_ADVSFX_APPLY:
            if (advSfxMenu_pos[OPT_ADVSFX_APPLY] == OPTS_REVERT) {
                REVERT_OPTS(advSfxMenu);
                return ERR_OK;
            }
            return applyAdvSfx();
        case OPT_ADVSFX_BACK:
            return back();
        }
        break;
    case GAME_OPTIONS:
        switch (vpos) {
        case OPT_GAME_ADVANCED:
            return load(&menustate, ADVGAME_OPTIONS);
        case OPT_GAME_BACK:
            return back();
        }
        break;
    case ADVGAME_OPTIONS:
        switch (vpos) {
        case OPT_ADVGAME_BACK:
            return back();
        }
        break;
    default:
        return ERR_UNHANDLED_MENU;
    }
    /* TODO: Play "noop" buzz. */
    return ERR_OK;
}

err loadOptions(menuCtx *ctx) {
    ctx->acceptCb = acceptCallback;
    ctx->hposCb = moveCallback;

    /* TODO Load positions from file */

    return load(ctx, MAIN_OPTIONS);
}

#define DISPLAY_BASE "WWWWWXHHHH@FFFHZ"
#define DISPLAY_FMT "%dX%d@%dHZ"

/**
 * Initialize the list of display mode names.
 *
 */
err initDisplayList() {
    gfmRV rv;
    int len, i;

    rv = gfm_queryResolutions(&len, game.pCtx);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    displayList = malloc(len * (sizeof(char**) + sizeof(DISPLAY_BASE)));
    ASSERT(displayList, ERR_OOM);
    for (i = 0; i < len; i++) {
        char *name;
        int w, h, fps;

        rv = gfm_getResolution(&w, &h, &fps, game.pCtx, i);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        name = (char*)displayList;
        name += i * sizeof(DISPLAY_BASE) + (len * sizeof(char**));
        snprintf(name, sizeof(DISPLAY_BASE) - 1, DISPLAY_FMT, w, h, fps);

        ((char**)displayList)[i] = name;
    }

    gfxMenu_subopts[OPT_GFX_FULL_MODE] = (char**)displayList;
    gfxMenu_count[OPT_GFX_FULL_MODE] = len;

    return ERR_OK;
}

/**
 * Release the list of display mode names.
 */
void freeDisplayList() {
    free(displayList);
    displayList = 0;
}
