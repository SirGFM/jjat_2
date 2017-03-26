/**
 * @file include/conf/gfx_list.h
 *
 * Define the list of available textures and its respective spritesets.
 */
#ifndef __CONF_GFX_LIST_H__
#define __CONF_GFX_LIST_H__

/**
 * List of textures. When defining the 'X macro' for use, the first parameter is
 * the name of the attribute, the second is the file and the last is the alpha
 * color key (in RRGGBB format).
 */
#define TEXTURE_LIST \
  X(atlas, "gfx/atlas.bmp", 0xFF00FF)

/**
 * List of spritesets. When defining the 'X macro' for use, the first parameter
 * is the name of the attribute, the second is the width of each tile, the third
 * is its height and the last one is the name of the texture associated with
 * this spriteset.
 */
#define SPRITESET_LIST \
  X(pSset8x8, 8/*width*/, 8/*height*/, atlas) \
  X(pSset16x16, 16/*width*/, 16/*height*/, atlas)

#endif /* __CONF_GFX_LIST_H__ */

