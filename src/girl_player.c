/**
 * @file src/girl_player.c
 *
 * Messy implementation (read: ugly and not generic) of the girl. This is mostly
 * a starting point so I can get the feel for what is and isn't need when coding
 * all mobs properly.
 *
 * Things I must have answered by the end of this:
 *  - How will everything update?
 *  - How will everything collide?
 *  - How do I handle collision against environment and 'moving platforms'?
 *  - How do I properly handle animations through various different sprites?
 *  - ...
 */
#include <base/game_const.h>
#include <base/game_ctx.h>

#include <GFraMe/gfmAssert.h>
#include <GFraMe/gfmError.h>
#include <GFraMe/gfmParser.h>
#include <GFraMe/gfmQuadtree.h>
#include <GFraMe/gfmSprite.h>

#include <stdlib.h>
#include <string.h>


