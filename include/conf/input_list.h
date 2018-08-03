/**
 * @file include/conf/input_list.h
 *
 * List of available buttons and their default keys.
 *
 * To make the most of this header, two macros must be defined:
 *  X_KEY(name, key) - retrieve the name (i.e., variable) of a virtual button
 *                     and its virtual button
 *  X_GPAD(name, button, port) - retrieve the name (i.e., variable) of a virtual
 *                               button, its gamepad button and its port
 *
 * All virtual keys are created based on the 'X_KEY' macro.
 */
#ifndef __INPUT_LIST_H__
#define __INPUT_LIST_H__

/** Create a list of system buttons and their default mapping. */
#define X_SYSTEM_BUTTON_LIST \
  X_KEY(fullscreen \
        , gfmKey_f12) \
  X_KEY(dummy \
       , gfmKey_f1)

/** Create a list of buttons and their default mapping. */
#define X_RELEASE_BUTTON_LIST \
  X_KEY(swordyLeft \
        , gfmKey_h) \
  X_GPAD(swordyLeft \
        , gfmController_laxis_left \
        , 0/*port*/) \
  X_KEY(swordyRight \
        , gfmKey_j) \
  X_GPAD(swordyRight \
        , gfmController_laxis_right \
        , 0/*port*/) \
  X_KEY(swordyJump \
        , gfmKey_s) \
  X_GPAD(swordyJump \
        , gfmController_l1 \
        , 0/*port*/) \
  X_KEY(swordyAtk \
        , gfmKey_d) \
  X_GPAD(swordyAtk \
        , gfmController_l2 \
        , 0/*port*/) \
  X_KEY(gunnyLeft \
        , gfmKey_k) \
  X_GPAD(gunnyLeft \
        , gfmController_raxis_left \
        , 0/*port*/) \
  X_KEY(gunnyRight \
        , gfmKey_l) \
  X_GPAD(gunnyRight \
        , gfmController_raxis_right \
        , 0/*port*/) \
  X_KEY(gunnyJump \
        , gfmKey_f) \
  X_GPAD(gunnyJump \
        , gfmController_r1 \
        , 0/*port*/) \
  X_KEY(gunnyAtk \
        , gfmKey_g) \
  X_GPAD(gunnyAtk \
        , gfmController_r2 \
        , 0/*port*/) \
  X_KEY(pause \
        , gfmKey_esc) \
  X_GPAD(pause \
        , gfmController_start \
        , 0/*port*/) \
  X_KEY(switchChar \
        , gfmKey_tab) \
  X_GPAD(switchChar \
        , gfmController_x \
        , 0/*port*/)

/** Add default alternate mappings for buttons */
#define X_ALTERNATE_BUTTON_MAPPING \
  X_KEY(swordyLeft \
        , gfmKey_left) \
  X_GPAD(swordyLeft \
        , gfmController_left \
        , 0/*port*/) \
  X_KEY(swordyRight \
        , gfmKey_right) \
  X_GPAD(swordyRight \
        , gfmController_right \
        , 0/*port*/) \
  X_KEY(swordyJump \
        , gfmKey_x) \
  X_GPAD(swordyJump \
        , gfmController_a \
        , 0/*port*/) \
  X_KEY(swordyAtk \
        , gfmKey_c) \
  X_GPAD(swordyAtk \
        , gfmController_b \
        , 0/*port*/) \

/** Create a list of debug buttons */
#if defined(DEBUG)
#  define X_DEBUG_BUTTON_LIST \
     X_KEY(qt \
        , gfmKey_f11) \
     X_KEY(gif \
        , gfmKey_f10) \
     X_KEY(dbgResetFps \
        , gfmKey_f7) \
     X_KEY(dbgStep \
        , gfmKey_f6) \
     X_KEY(dbgPause \
        , gfmKey_f5) \
     X_KEY(dbgSetPos \
        , gfmPointer_button)
#else
#  define X_DEBUG_BUTTON_LIST
#endif

/** Concatenate both lists */
#define X_BUTTON_LIST \
  X_RELEASE_BUTTON_LIST \
  X_SYSTEM_BUTTON_LIST \
  X_DEBUG_BUTTON_LIST

#endif /* __INPUT_LIST_H__ */

