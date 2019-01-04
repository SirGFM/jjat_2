/**
 * @file src/base/input.c
 */
#include <base/collision.h>
#include <base/error.h>
#include <base/game.h>
#include <base/input.h>
#include <conf/input_list.h>

#include <GFraMe/gfmError.h>

#include <string.h>

/* Current list of buttons in use */
static button *inputList;
/* Number of buttons in the list */
static int listCount;

enum enInputNames {
#define X_GPAD(...)
#define X_KEY(name, ...) enInput_##name,
    X_RELEASE_BUTTON_LIST
    X_SYSTEM_BUTTON_LIST
    enInput_count,
    X_DEBUG_BUTTON_LIST
#undef X_KEY
#undef X_GPAD
};
typedef enum enInputNames inputNames;

/**
 * Handle every input that require an immediate action (i.e, those that are more
 * like flags, instead of being interpreted during the game loop).
 */
void handleInput() {
    if (inputList != (button*)&input) {
        return;
    }

    if (DID_JUST_PRESS(pause)) {
        /* TODO Pause the game */
    }

    if (DID_JUST_RELEASE(fullscreen)) {
        gfmRV rv;

        /* TODO Refactor this keeping the current state */
        rv = gfm_setWindowed(game.pCtx);
        if (rv == GFMRV_WINDOW_MODE_UNCHANGED) {
            gfm_setFullscreen(game.pCtx);
        }
    }

#if defined(JJATENGINE)
    if ((game.flags & AC_BOTH) != AC_BOTH && DID_JUST_PRESS(switchChar)) {
        /* Invert the currently active player */
        game.flags ^= AC_BOTH;

#define SWITCH_VKEY(new, old, vkey) \
    input.new ## vkey.handle = input.old ## vkey.handle; \
    input.old ## vkey.handle = input.dummy.handle
#define SWITCH_CHAR(new, old) \
    SWITCH_VKEY(new, old, Left); \
    SWITCH_VKEY(new, old, Right); \
    SWITCH_VKEY(new, old, Jump); \
    SWITCH_VKEY(new, old, Atk)

        switch (game.flags & AC_BOTH) {
            case AC_SWORDY: {
                /* Swordy just became active */
                SWITCH_CHAR(swordy, gunny);
            } break;
            case AC_GUNNY: {
                /* Gunny just became active */
                SWITCH_CHAR(gunny, swordy);
            } break;
        }

#undef SWITCH_VKEY
#undef SWITCH_CHAR

    }
#endif /* JJATENGINE */
}

#if defined(DEBUG)
/**
 * Handle the debug controls of the game's simulation. These allow the update
 * loop to be paused/resumed or even stepped.
 */
void handleDebugInput() {
    if (inputList != (button*)&input) {
        return;
    }

    if (DID_JUST_RELEASE(dbgResetFps)) {
        gfm_resetFPS(game.pCtx);
    }

    if (DID_JUST_RELEASE(dbgPause)) {
        /* Toggle pause/resume update loop */
        if (game.debugRunState == DBG_PAUSED) {
            game.debugRunState = DBG_RUNNING;
        }
        else {
            game.debugRunState = DBG_PAUSED;
        }
    }

    if (DID_JUST_RELEASE(dbgStep)) {
        /* Single step & pause update loop */
        game.debugRunState = DBG_STEP;
    }

    if (DID_JUST_RELEASE(qt)) {
        /* Toggle quadtree visibility */
        collision.flags ^= CF_VISIBLE;
    }

    if (DID_JUST_RELEASE(gif)) {
        gfmRV rv;

        rv = gfm_didExportGif(game.pCtx);
        if (rv == GFMRV_TRUE || rv == GFMRV_GIF_OPERATION_NOT_ACTIVE) {
            rv = gfm_recordGif(game.pCtx, 10000 /* ms */, "anim.gif", 8, 0);
        }
    }
}
#endif

/** Retrieve the state of every button */
err updateInput() {
    /** List of buttons, used to easily iterate through all virtual buttons */
    inputNames i;

    i = 0;
    /* Iterate through all buttons and update their state */
    while (i < listCount) {
        gfmRV rv;

        rv = gfm_getKeyState(&inputList[i].state, &inputList[i].numPressed
                , game.pCtx, inputList[i].handle);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);

        i++;
    }

    return ERR_OK;
}

/** Forcefully update every debug button */
err updateDebugInput() {
    gfmInput *pInput;
    gfmRV rv;

    if (inputList != (button*)&input) {
        return ERR_OK;
    }

    rv = gfm_getInput(&pInput, game.pCtx);
    ASSERT(rv == GFMRV_OK, rv);

#define X_GPAD(...)
#define X_KEY(name, ...) \
    rv = gfmInput_updateVKey(pInput, input.name.handle); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR); \
    rv = gfm_getKeyState(&input.name.state, &input.name.numPressed, game.pCtx \
                , input.name.handle); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_DEBUG_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    return ERR_OK;
}

static err configureGameButtons() {
    gfmInput *pInput;
    gfmRV rv;

    rv = gfm_getInput(&pInput, game.pCtx);
    ASSERT(rv == GFMRV_OK, rv);

    rv = gfmInput_reset(pInput);
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);

    /* Create virtual keys for every input */
#define X_GPAD(...)
#define X_KEY(name, ...) \
    rv = gfm_addVirtualKey(&input.name.handle, game.pCtx); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    setButtonList((button*)&input, enInput_count);

    return ERR_OK;
}

/** Initialize every button with their default mapping */
err initInput() {
    gfmRV rv;

    ASSERT(configureGameButtons() == ERR_OK, ERR_GFMERR);

    /* Bind every key */
#define X_GPAD(...)
#define X_KEY(name, key, ...) \
    rv = gfm_bindInput(game.pCtx, input.name.handle, key); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
    X_ALTERNATE_BUTTON_MAPPING
#undef X_KEY
#undef X_GPAD

#define X_KEY(...)
#define X_GPAD(name, button, port, ...) \
    rv = gfm_bindGamepadInput(game.pCtx, input.name.handle, button, port); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
    X_ALTERNATE_BUTTON_MAPPING
#undef X_GPAD
#undef X_KEY

    return ERR_OK;
}

err initInputFromStr(char *pMap, int len) {
    ASSERT(configureGameButtons() == ERR_OK, ERR_GFMERR);
    ASSERT(configureInput(pMap, len) == ERR_OK, ERR_GFMERR);

    return ERR_OK;
}

/** Set the list of buttons to be updated */
void setButtonList(button *list, int count) {
    inputList = list;
    listCount = count;
}

#if defined(JJATENGINE)
/** Convert a single char to its hexadecimal representation */
static inline char _convertChar2Hexa(char c) {
    if (c >= 'a' && c <= 'f') {
        return (char)(c - 'a' + 0xa);
    }
    else if (c >= 'A' && c <= 'F') {
        return (char)(c - 'A' + 0xa);
    }
    else {
        return (char)(c - '0');
    }
}

/**
 * Configure the (remappable) inputs.
 *
 * @param  [ in]pMap The key map
 * @param  [ in]len  Length of the map
 */
err configureInput(char *pMap, int len) {
    err erv;
    int i;

    erv = beginInputRemapping(0/*non-interactive*/);
    ASSERT(erv == ERR_OK, erv);

    i = 0;
    while (i < len) {
        int j, handle;

        /* Retrieve the current key identifier */
        j = 0;
        while (i + j < len && pMap[i + j] != ':') {
            j++;
        }
        ASSERT(i + j < len, ERR_INVALIDINPUTMAP);

        if (memcmp(pMap + i, "SL:", 3) == 0) {
            handle = input.swordyLeft.handle;
        }
        else if (memcmp(pMap + i, "SR:", 3) == 0) {
            handle = input.swordyRight.handle;
        }
        else if (memcmp(pMap + i, "SJ:", 3) == 0) {
            handle = input.swordyJump.handle;
        }
        else if (memcmp(pMap + i, "SA:", 3) == 0) {
            handle = input.swordyAtk.handle;
        }
        else if (memcmp(pMap + i, "GL:", 3) == 0) {
            handle = input.gunnyLeft.handle;
        }
        else if (memcmp(pMap + i, "GR:", 3) == 0) {
            handle = input.gunnyRight.handle;
        }
        else if (memcmp(pMap + i, "GJ:", 3) == 0) {
            handle = input.gunnyJump.handle;
        }
        else if (memcmp(pMap + i, "GA:", 3) == 0) {
            handle = input.gunnyAtk.handle;
        }
        else if (memcmp(pMap + i, "P:", 2) == 0) {
            handle = input.pause.handle;
        }
        else if (memcmp(pMap + i, "SW:", 3) == 0) {
            handle = input.switchChar.handle;
        }
        else {
            ASSERT(0, ERR_INVALIDKEYID);
        }
        i += j + 1;

        /* Read each physical key (guaranteed to be 2 characters long) */
        while (i < len && pMap[i] != ';') {
            gfmInputIface iface;
            int port;

            ASSERT(i + 2 <= len, ERR_INVALIDINPUTMAP);
            iface = (_convertChar2Hexa(pMap[i]) & 0xf) << 4;
            iface |= _convertChar2Hexa(pMap[i + 1]) & 0xf;
            ASSERT(iface > gfmIface_none && iface < gfmIface_max
                    , ERR_INVALIDKEYVALUE);
            i += 2;

            if (iface >= gfmController_left) {
                ASSERT(i + 1 <= len, ERR_INVALIDINPUTMAP);
                port = (int)(_convertChar2Hexa(pMap[i]) & 0xf);
                i++;
            }

            /* Dumb copiler says that port may be used unitialized here. That
             * would only happen if iface were less than gfmController_left.
             * However, port is only used within updateKeyMapping if iface is
             * greater or equal gfmController_left, case which it's also set on
             * this call...
             * Therefore, ignore that warning! */
            erv = updateKeyMapping(handle, iface, port);
            ASSERT(erv == ERR_OK, erv);
        } /* while (i < len && pMap[i] != ';') */
        i++;
    } /* while (i < len) */

    erv = endInputRemapping();
    ASSERT(erv == ERR_OK, erv);

    return ERR_OK;
}

/**
 * Starts remapping the inputs used by the game.
 *
 * Every input is reset (except by a few system inputs) and should be later
 * rebound by calling updateKeyMapping.
 *
 * @param  [ in]interactive Whether the remap will be run interactivelly or not
 */
err beginInputRemapping(int interactive) {
    gfmRV rv;

    /* Reset and re-create every virtual key */
    gfm_resetInput(game.pCtx);
#define X_GPAD(...)
#define X_KEY(name, ...) \
    rv = gfm_addVirtualKey(&input.name.handle, game.pCtx); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    if (interactive) {
        int port;

        gfm_cancelGetLastPressed(game.pCtx);
        rv = gfm_getLastPort(&port, game.pCtx);
        ASSERT(rv == GFMRV_WAITING, ERR_GFMERR);
    }

    return ERR_OK;
}

/**
 * Updates the mapping for a given virtual key.
 *
 * If running interactively, iface must be gfmIface_none.
 *
 * @param  [ in]handle Identifier of the current virtual key
 * @param  [ in]iface  Physical key that will be bound to the virtual key
 * @param  [ in]port   Port of the gamepad being bound (only required for
 *                     gfmController_*)
 */
err updateKeyMapping(int handle, gfmInputIface iface, int port) {
    gfmRV rv;

    if (iface == gfmIface_none) {
        /** Running interactivelly, so get it from the framework */
        rv = gfm_getLastPort(&port, game.pCtx);
        if (rv == GFMRV_WAITING) {
            return ERR_NOKEYPRESSED;
        }
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
        rv = gfm_getLastPressed(&iface, game.pCtx);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
    ASSERT(iface > gfmIface_none && iface < gfmIface_max, ERR_ARGUMENTBAD);

    if (iface >= gfmController_left) {
        rv = gfm_bindGamepadInput(game.pCtx, handle, iface, port);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }
    else {
        rv = gfm_bindInput(game.pCtx, handle, iface);
        ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    }

    return ERR_OK;
}

/** Finishes remapping the game's inputs. */
err endInputRemapping() {
    gfmRV rv;

    gfm_cancelGetLastPressed(game.pCtx);

    /* Bind every system key */
#define X_GPAD(name, button, port, ...) \
    rv = gfm_bindGamepadInput(game.pCtx, input.name.handle, button, port); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
#define X_KEY(name, key, ...) \
    rv = gfm_bindInput(game.pCtx, input.name.handle, key); \
    ASSERT(rv == GFMRV_OK, ERR_GFMERR);
    X_SYSTEM_BUTTON_LIST
    X_DEBUG_BUTTON_LIST
#undef X_KEY
#undef X_GPAD

    return ERR_OK;
}
#endif /* JJATENGINE */

