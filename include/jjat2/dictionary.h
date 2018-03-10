/**
 * @file include/jjat2/dictionary.h
 *
 * Types dictionary, used when loading a map.
 */
#ifndef __JJAT2_DICTIONARY_H__
#define __JJAT2_DICTIONARY_H__

/** Names (i.e., labels) that identifies types. Label position must match type
 * position in pDictTypes. */
extern char *pDictNames[];

/** Types available when parsing a tilemap. Its position must match its lable in
 * pDictNames. */
extern int pDictTypes[];

/** Numbers of entries on the dictionary */
extern int dictLen;

/** Types that must be converted into sides of polygons. */
extern int pSidedTypes[];

/** Numbers of entries in pSidedTypes */
extern int sidedLen;

#endif /* __JJAT2_DICTIONARY_H__ */

