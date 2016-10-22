/**
 * @file include/jjat2/swordy.h
 */
#ifndef __JJAT2_SWORDY_H__
#define __JJAT2_SWORDY_H__

#include <jjat2/entity.h>

struct stSwordyCtx {
    /** Base object */
    entityCtx entity;
    /** Whether swordy has already double jumped */
    int didDoubleJump;
};
typedef struct stSwordyCtx swordyCtx;

#endif /* __JJAT2_SWORDY_H__ */

