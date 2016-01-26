/**
 * @file include/jjat_2/type.h
 *
 * Define all in-game types
 */
#ifndef __TYPE_H__
#define __TYPE_H__

#include <GFraMe/gfmTypes.h>

/**
 * Insteresting feature about types and the quadtree:
 *
 * If a type 'T' is greater or equal to gfmType_max, it will be displayed with
 * the same color as the type '(T % gfmType_max) + 1'.
 *
 * So, it's possible to have to different enemies be displayed with the same
 * color, even though they are handled differently by the code during collisiong
 */

/* Mask that returns only the entity, ignoring its base type */
#define T_MASK 0xffffffe0
/* Number of bits for the types */
#define T_NBITS 5

enum enType {
    /* Base type for collideables that doesn't hurt nor move (e.g., floor) */
    T_TERRAIN = gfmType_reserved_4,
    /* Base type for hitbox objects (i.e., bullets, slashes, etc) */
    T_HITBOX = gfmType_reserved_7,
    T_SLASH  = (1 << T_NBITS) | T_HITBOX,
    T_BULLET = (2 << T_NBITS) | T_HITBOX,
};
typedef enum enType type;

#endif /* __TYPE_H__ */

