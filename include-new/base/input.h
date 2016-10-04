/**
 * @file include/base/input.h
 *
 * Define the list of inputs and its handling functions
 */
#ifndef __BASE_INPUT_H__
#define __BASE_INPUT_H__

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
#define X(name, ...) button name;
    X_BUTTON_LIST
#undef X
};
typedef struct stInputCtx inputCtx;

extern inputCtx input;

#endif /* __BASE_INPUT_H__ */

