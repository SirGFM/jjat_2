/**
 * @file include/conf/error_list.h
 *
 * Enumerate all possible errors.
 */
#ifndef __CONF_ERROR_LIST_H__
#define __CONF_ERROR_LIST_H__

/** List of errors. Should be accessed through the 'err' enumeration */
#define ERROR_LIST \
    X(ERR_OK) \
    X(ERR_FORCEEXIT) \
    X(ERR_ARGUMENTBAD) \
    X(ERR_MISSINGPARAM) \
    X(ERR_GFMERR) \
    X(ERR_INDEXOOB) \
    X(ERR_DIDJUMP) \
    X(ERR_INVALIDINPUTMAP) \
    X(ERR_INVALIDKEYID) \
    X(ERR_INVALIDKEYVALUE) \
    X(ERR_NOKEYPRESSED) \
    X(ERR_INVALIDTYPE) \
    X(ERR_BUFFERTOOSMALL) \
    X(ERR_NOHIT) \
    X(ERR_INVALIDLEVELNAME) \
    X(ERR_PARSINGERR) \
    X(ERR_UNEXPECTEDBEHAVIOUR) \
    X(ERR_UNHANDLED_COLLISION) \
    X(ERR_NOTIMPLEMENTED) \
    X(ERR_ALREADYLOADING) \
    X(ERR_LOADINGRESOURCE) \
    X(ERR_MAX)

#endif /* __CONF_ERROR_LIST_H__ */

