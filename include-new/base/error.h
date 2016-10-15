/**
 * @file include/base/error.h
 */
#ifndef __ERROR_H__
#define __ERROR_H__

#include <conf/error_list.h>

/**
 * Check if a condition failed, execute a statement (to set the error value) and
 * jump to a label (where clean up should be done).
 *
 * @param  [ in]stmt     Statement to be evaluated
 * @param  [ in]ret_stmt Statement to set the erro value
 * @param  [ in]label    Label where function clean up should be done
 */
#define ASSERT_TO(stmt, ret_stmt, label) \
  do { \
    if (!(stmt)) { \
      ret_stmt; \
      goto label; \
    } \
  } while (0)

/**
 * Check if a condition failed and return its error
 *
 * @param  [ in]stmt Statement to be evaluated
 * @param  [ in]ret  Returned value, on failure
 */
#define ASSERT(stmt, ret) \
  do { \
    if (!(stmt)) { \
      return ret; \
    } \
  } while (0)

enum enErr {
#define X(val) val,
    ERROR_LIST
#undef X
};
typedef enum enErr err;

#endif /* __ERROR_H__ */

