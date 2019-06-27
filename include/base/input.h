/**
 * @file include/base/input.h
 *
 * Define the list of inputs and its handling functions
 */
#ifndef __BASE_INPUT_H__
#define __BASE_INPUT_H__

#include <base/error.h>
#include <conf/input_list.h>

#include <GFraMe/gfmInput.h>

/** Define a single button */
struct stButton {
    /** Button handle, internal to the framework */
    int handle;
    /** Number of times the button was consecutivelly pressed */
    int numPressed;
    /** Current state of the button */
    gfmInputState state;
    /** Ignore; Used to expand the struct to 32 bytes */
    int padding;
};
typedef struct stButton button;

/** Encapsulate every button */
struct stInputCtx {
#define X_GPAD(...)
#define X_KEY(name, ...) button name;
    X_BUTTON_LIST
#undef X_KEY
#undef X_GPAD
};
typedef struct stInputCtx inputCtx;

/** Encapsulate every button */
struct stMenuInputCtx {
#define X_GPAD(...)
#define X_KEY(name, ...) button name;
    X_MENU_BUTTON_LIST
#undef X_KEY
#undef X_GPAD
};
typedef struct stMenuInputCtx menuInputCtx;

/** Global input (declared on src/base/static.c) */
extern inputCtx input;

/** Global menu input (declared on src/jjat2/static.c) */
extern menuInputCtx menuInput;

/**
 * Handle every input that require an immediate action (i.e, those that are more
 * like flags, instead of being interpreted during the game loop).
 */
void handleInput();

/**
 * Handle the debug controls of the game's simulation. These allow the update
 * loop to be paused/resumed or even stepped.
 */
void handleDebugInput();

/** Retrieve the state of every button */
err updateInput();

/** Forcefully update every debug button */
err updateDebugInput();

/** Initialize every button with their default mapping */
err initInput();

/** Initialize every button with a custom mapping */
err initInputFromStr(char *pMap, int len);

/** Set the list of buttons to be updated */
void setButtonList(button *list, int count);

#if defined(JJATENGINE)
/**
 * Configure the (remappable) inputs.
 *
 * @param  [ in]pMap The key map
 * @param  [ in]len  Length of the map
 */
err configureInput(char *pMap, int len);

/**
 * Starts remapping the inputs used by the game.
 *
 * Every input is reset (except by a few system inputs) and should be later
 * rebound by calling updateKeyMapping.
 *
 * @param  [ in]interactive Whether the remap will be run interactivelly or not
 */
err beginInputRemapping(int interactive);

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
err updateKeyMapping(int handle, gfmInputIface iface, int port);

/** Finishes remapping the game's inputs. */
err endInputRemapping();
#endif /* JJATENGINE */

/** Whether a given button is currently released */
#define IS_RELEASED(bt) \
    (input.bt.state & gfmInput_released)

/** Whether a given button is currently pressed */
#define IS_PRESSED(bt) \
    (input.bt.state & gfmInput_pressed)

/** Whether a state is just pressed */
#define IS_STATE_JUSTPRESSED(st) \
    ((st & gfmInput_justPressed) == gfmInput_justPressed)

/** Whether a state is just released */
#define IS_STATE_JUSTRELEASED(st) \
    ((st & gfmInput_justReleased) == gfmInput_justReleased)

/** Whether a given button was just pressed */
#define DID_JUST_PRESS(bt) \
    (IS_STATE_JUSTPRESSED(input.bt.state))

/** Whether a given button was just released */
#define DID_JUST_RELEASE(bt) \
    (IS_STATE_JUSTRELEASED(input.bt.state))

/** Whether a given menu button is currently released */
#define IS_MENU_RELEASED(bt) \
    (menuInput.bt.state & gfmInput_released)

/** Whether a given menu button is currently pressed */
#define IS_MENU_PRESSED(bt) \
    (menuInput.bt.state & gfmInput_pressed)

/** Whether a given menu button was just pressed */
#define DID_JUST_PRESS_MENU(bt) \
    (IS_STATE_JUSTPRESSED(menuInput.bt.state))

/** Whether a given menu button was just released */
#define DID_JUST_RELEASE_MENU(bt) \
    (IS_STATE_JUSTRELEASED(menuInput.bt.state))

#endif /* __BASE_INPUT_H__ */

