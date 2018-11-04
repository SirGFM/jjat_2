/**
 * @file src/jjat2/dictionary.c
 *
 * Types dictionary, used when loading a map.
 */
#include <conf/type.h>

#include <jjat2/dictionary.h>

/** List that associates every label with its type. The label may be retrieved
 * on the first parameter of a X macro, while the type may be retrieved on its
 * second parameter */
#define DICTIONARY_LIST \
    X("floor", T_FLOOR) \
    X("floor_notp", T_FLOOR_NOTP) \
    Y("spike", T_SPIKE) \
    X("blue_platform", T_BLUE_PLATFORM)

/** Names (i.e., labels) that identifies types. Label position must match type
 * position in pDictTypes. */
char *pDictNames[] = {
#define X(label, type) label,
#define Y(label, type) label,
    DICTIONARY_LIST
#undef Y
#undef X
};

/** Types available when parsing a tilemap. Its position must match its lable in
 * pDictNames. */
int pDictTypes[] = {
#define X(label, type) type,
#define Y(label, type) type,
    DICTIONARY_LIST
#undef Y
#undef X
};

/** Numbers of entries on the dictionary */
int dictLen = sizeof(pDictTypes) / sizeof(int);

/** Types that must be converted into sides of polygons. */
int pSidedTypes[] = {
#define X(label, type) type,
#define Y(label, type)
    DICTIONARY_LIST
#undef Y
#undef X
};

/** Numbers of entries in pSidedTypes */
int sidedLen = sizeof(pSidedTypes) / sizeof(int);
