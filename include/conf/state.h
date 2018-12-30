/**
 * @file include/conf/state.h
 *
 * List of available states
 */
#ifndef __CONF_STATE_H__
#define __CONF_STATE_H__

enum enState {
    ST_NONE = 0,
    ST_PLAYSTATE,
    ST_LEVELTRANSITION,
    ST_LOADSTATE,
    ST_MENUSTATE,
    ST_MAX
};
typedef enum enState state;

#endif /* __CONF_STATE_H__ */

