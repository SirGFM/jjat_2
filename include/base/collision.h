/**
 * @file include/base/collision.h
 */
#ifndef __BASE_COLLISION_H__
#define __BASE_COLLISION_H__

#include <base/error.h>

#include <GFraMe/gfmQuadtree.h>

enum enCollisionFlags {
    /** Whether pending collisions (for the current object) should be skipped */
    CF_SKIP        = 0x01
    /** Quadtree's visibility (only available on debug mode) */
  , CF_VISIBLE     = 0x02
#if  defined(JJATENGINE)
    /** Fix passing through platforms when teleporting */
  , CF_FIXTELEPORT = 0x04
#endif  /* JJATENGINE */
};
typedef enum enCollisionFlags collisionFlags;

struct stCollisionCtx {
    /** Quadtree's root */
    gfmQuadtreeRoot *pQt;
    /** Static quadtree's root */
    gfmQuadtreeRoot *pStaticQt;
    /** Controls the collision */
    collisionFlags flags;
};
typedef struct stCollisionCtx collisionCtx;

/** Collision context. Declared on src/base/static.c. */
extern collisionCtx collision;

/** Setup the collision context */
err setupCollision();

/** Release all memory used by the collision context */
void cleanCollision();

/**
 * Continue handling collision.
 * 
 * Different from the other functions on this module, this one is declared on
 * src/collision.c (instead of src/base/collision.c). This decision was made
 * because this function shall be modified for each game.
 *
 * @param  [ in]pQt The current quadtree
 */
err doCollide(gfmQuadtreeRoot *pQt);

/** Skip any pending collision for the current object */
#define skipCollision() do { collision.skip = 1; } while (0)

/** Checks whether the quadtree should be rendered */
#if defined(DEBUG)
#  define IS_QUADTREE_VISIBLE() (collision.flags & CF_VISIBLE)
#else
#  define IS_QUADTREE_VISIBLE() (0)
#endif

#endif /* __BASE_COLLISION_H__ */

