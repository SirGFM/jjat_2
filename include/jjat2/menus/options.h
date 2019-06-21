#ifndef __JJAT2_OPTIONS_H__
#define __JJAT2_OPTIONS_H__

#include <base/error.h>

/**
 * Initialize the list of display mode names.
 *
 */
err initDisplayList();

/**
 * Release the list of display mode names.
 */
void freeDisplayList();

#endif /* __JJAT2_OPTIONS_H__ */
