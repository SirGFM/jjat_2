/**
 * @file include/base/error.h
 */
#ifndef __ERROR_H__
#define __ERROR_H__

enum enErr {
    ERR_OK,
    ERR_ARGUMENTBAD,
    ERR_MISSINGPARAM,
    ERR_MAX
};
typedef enum enErr err;

#endif /* __ERROR_H__ */

