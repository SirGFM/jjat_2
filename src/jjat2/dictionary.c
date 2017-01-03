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
    X("spike", T_SPIKE)

/** Names (i.e., labels) that identifies types. Label position must match type
 * position in pDictTypes. */
char *pDictNames[] = {
#define X(label, type) label,
    DICTIONARY_LIST
#undef X
};

/** Types available when parsing a tilemap. Its position must match its lable in
 * pDictNames. */
int pDictTypes[] = {
#define X(label, type) type,
    DICTIONARY_LIST
#undef X
};

/** Numbers of entries on the dictionary */
int dictLen = sizeof(pDictTypes) / sizeof(int);

