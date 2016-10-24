/**
 * @file include/conf/input_list.h
 *
 * List of available buttons and their default keys.
 *
 * To make the most of this header, a 'X' macro must be defined, which will do
 * something with each parameter. The first will is the name of the virtual
 * button (TBD whether an enum within an array or a member within a struct), the
 * second is the default keyboard mapping and the last (if available) is the
 * default gamepad mapping.
 */
#ifndef __INPUT_LIST_H__
#define __INPUT_LIST_H__

/** Create a list of buttons and their default mapping. */
#define X_RELEASE_BUTTON_LIST \
  X(swordyLeft    , gfmKey_h  , gfmController_laxis_left) \
  X(swordyRight   , gfmKey_j  , gfmController_laxis_right) \
  X(swordyJump    , gfmKey_s  , gfmController_l1) \
  X(swordyAtk     , gfmKey_d  , gfmController_l2) \
  X(gunnyLeft    , gfmKey_k  , gfmController_raxis_left) \
  X(gunnyRight   , gfmKey_l  , gfmController_raxis_right) \
  X(gunnyJump    , gfmKey_f  , gfmController_r1) \
  X(gunnyAtk     , gfmKey_g  , gfmController_r2) \
  X(pause      , gfmKey_esc, gfmController_start) \
  X(fullscreen , gfmKey_f12) \

/** Create a list of debug buttons */
#if defined(DEBUG)
#  define X_DEBUG_BUTTON_LIST \
     X(qt         , gfmKey_f11) \
     X(gif        , gfmKey_f10) \
     X(dbgStep    , gfmKey_f6) \
     X(dbgPause   , gfmKey_f5)
#else
#  define X_DEBUG_BUTTON_LIST
#endif

/** Concatenate both lists */
#define X_BUTTON_LIST \
  X_RELEASE_BUTTON_LIST \
  X_DEBUG_BUTTON_LIST

#endif /* __INPUT_LIST_H__ */

