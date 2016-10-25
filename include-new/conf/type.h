/**
 * @file include/conf/type.h
 *
 * Define all in-game types.
 */
#ifndef __CONF_TYPE_H__
#define __CONF_TYPE_H__

#include <GFraMe/gfmTypes.h>

/** Mask that return the proper 16 bit type */
#define T_MASK 0x0000ffff
/** Number of bits per type */
#define T_BITS 16
/**
 * How many bits there are for any given "base type". Different types that share
 * the same base one will be rendered within the quadtree with the same color.
 */
#define T_BASE_NBITS 5

/** Retrieve an object's type (mask out all non-type bits) */
#define TYPE(type) \
    (type & T_MASK)

enum enType {
    T_FLOOR  = gfmType_reserved_5, /* purple */
    T_PLAYER = gfmType_reserved_7, /* light red */

    T_SWORDY = (1 << T_BASE_NBITS) | T_PLAYER,
    T_GUNNY  = (2 << T_BASE_NBITS) | T_PLAYER,
};
typedef enum enType type;

#endif /* __CONF_TYPE_H__ */

