#include "value.h"
#include "value_cmp.h"

#include <stdio.h>
#include <sys/param.h>

// Comparators for all simple types
GENERIC_CMP_FUNC_IMPL(cmp_float, floatval);
GENERIC_CMP_FUNC_IMPL(cmp_int, intval);
GENERIC_CMP_FUNC_IMPL(cmp_long, longval);
GENERIC_CMP_FUNC_IMPL(cmp_double, doubleval);
GENERIC_CMP_FUNC_IMPL(cmp_uint, uintval);

int cmp_string(void *p1, void *p2) {
  SIValue *v1 = p1, *v2 = p2;
  if (SIValue_IsInf(v1) || SIValue_IsNegativeInf(v2))
    return 1;
  if (SIValue_IsInf(v2) || SIValue_IsNegativeInf(v1))
    return -1;


  // compare the longest length possible, which is the shortest length of the
  // two strings
  int cmp = strncasecmp(v1->stringval.str, v2->stringval.str,
                    MIN(v2->stringval.len, v1->stringval.len));

  // if the strings are equal at the common length but are not of the same
  // length, the longer string wins
  if (cmp == 0 && v1->stringval.len != v2->stringval.len) {
    return v1->stringval.len > v2->stringval.len ? 1 : -1;
  }
  // if they are not equal, or equal and same length - return the original cmp
  return cmp;
}
