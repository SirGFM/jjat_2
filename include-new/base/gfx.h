/**
 * @file include/base/gfx.h
 *
 * Define the list of available textures and its respective spritesets.
 */
#ifndef __BASE_GFX_H__
#define __BASE_GFX_H__

#include <conf/gfx_list.h>

#include <GFraMe/gframe.h>
#include <GFraMe/gfmSpriteset.h>

struct stGfxCtx {
#define X(name, ...) \
    gfmTexture *name;
    TEXTURE_LIST
#undef X

#define X(name, ...) \
    gfmSpriteset *name;
    SPRITESET_LIST
#undef X
};
typedef struct stGfxCtx gfxCtx;

/** Global graphics context (declared on src/base/static.c) */
extern gfxCtx gfx;

#endif /* __BASE_GFX_H__ */

