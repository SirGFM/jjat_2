/**
 * @file include/jjat2/events/common.h
 */
#ifndef __EVENTS_COMMON_H__
#define __EVENTS_COMMON_H__

#include <stdint.h>

enum enLocalVar {
    EV_LOCAL_A = 0x00000001
  , EV_LOCAL_B = 0x00000002
  , EV_LOCAL_C = 0x00000004
  , EV_LOCAL_D = 0x00000008
};

/** Local variables that events may modify/check */
extern uint32_t _localVars;

/**
 * Convert a local variable into its value
 *
 * @param  [ in]pVal The value
 * @return The variable's value or 0, on failure
 */
uint8_t _getLocalVar(char *pVal);

#endif /* __EVENTS_COMMON_H__ */

