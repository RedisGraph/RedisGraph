#ifndef __CMP_H__
#define __CMP_H__

#include <stdlib.h>
#include "value.h"

typedef int (*CmpFunc)(void *p1, void *p2);

#define GENERIC_CMP_FUNC_DECL(F) int F(void *p1, void *p2);

#define GENERIC_CMP_FUNC_IMPL(F, memb)                                         \
  int F(void *p1, void *p2) {												   \
    SIValue *v1 = p1, *v2 = p2;                                                \
    if (SIValue_IsInf(v1) || SIValue_IsNegativeInf(v2))                        \
      return 1;                                                                \
    if (SIValue_IsInf(v2) || SIValue_IsNegativeInf(v1))                        \
      return -1;                                                               \
    return (v1->memb < v2->memb ? -1 : (v1->memb > v2->memb ? 1 : 0));         \
  }


GENERIC_CMP_FUNC_DECL(cmp_string);
GENERIC_CMP_FUNC_DECL(cmp_float);
GENERIC_CMP_FUNC_DECL(cmp_int);
GENERIC_CMP_FUNC_DECL(cmp_long);
GENERIC_CMP_FUNC_DECL(cmp_double);
GENERIC_CMP_FUNC_DECL(cmp_uint);

#endif
