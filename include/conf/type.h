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
/** Mask that returns the type's base type */
#define T_BASE_MASK  0x0000001f

/** Retrieve an object's type (mask out all non-type bits) */
#define TYPE(type) \
    (type & T_MASK)

enum enType {
      T_HAZARD    = gfmType_reserved_2  /* ( 5) pink */
    , T_PLAYER    = gfmType_reserved_3  /* ( 6) light blue */
    , T_EVENT     = gfmType_reserved_4  /* ( 7) green */
    , T_FLOOR     = gfmType_reserved_5  /* ( 8) purple */
    , T_ENEMY     = gfmType_reserved_7  /* (10) light red */
    , T_FX        = gfmType_reserved_10 /* (13) dirty yellow/green */

    , T_FLOOR_NOTP      = (1 << T_BASE_NBITS) | T_FLOOR
    , T_FLOOR_SKIP_TP   = (2 << T_BASE_NBITS) | T_FLOOR
    , T_LOADZONE        = (3 << T_BASE_NBITS) | T_FLOOR
    , T_BLUE_PLATFORM   = (4 << T_BASE_NBITS) | T_FLOOR
    , T_SWORDY          = (1 << T_BASE_NBITS) | T_PLAYER
    , T_GUNNY           = (2 << T_BASE_NBITS) | T_PLAYER
    , T_ATK_SWORD       = (3 << T_BASE_NBITS) | T_PLAYER
    , T_TEL_BULLET      = (4 << T_BASE_NBITS) | T_PLAYER
    , T_CHECKPOINT      = (5 << T_BASE_NBITS) | T_PLAYER
    , T_DUMMY_SWORDY    = (6 << T_BASE_NBITS) | T_PLAYER
    , T_DUMMY_GUNNY     = (7 << T_BASE_NBITS) | T_PLAYER
    , T_SPIKE           = (1 << T_BASE_NBITS) | T_HAZARD
    , T_EN_WALKY        = (1 << T_BASE_NBITS) | T_ENEMY
    , T_EN_G_WALKY      = (2 << T_BASE_NBITS) | T_ENEMY
    , T_EN_G_WALKY_ATK  = (3 << T_BASE_NBITS) | T_ENEMY
    , T_EN_G_WALKY_VIEW = (4 << T_BASE_NBITS) | T_ENEMY
    , T_EN_SPIKY        = (5 << T_BASE_NBITS) | T_ENEMY
    , T_EN_TURRET       = (6 << T_BASE_NBITS) | T_ENEMY
    , T_SWORD_FX        = (1 << T_BASE_NBITS) | T_FX
    , T_DOOR            = (1 << T_BASE_NBITS) | T_EVENT
    , T_PRESSURE_PAD    = (2 << T_BASE_NBITS) | T_EVENT
    , T_HDOOR           = (3 << T_BASE_NBITS) | T_EVENT
};
typedef enum enType type;

/**
 * NOTES:
 *   - T_HDOOR is only used by the sprite itself! It appears as a T_DOOR within
 *     the quadtree.
 */

#endif /* __CONF_TYPE_H__ */

