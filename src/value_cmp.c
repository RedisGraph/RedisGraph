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
  return strcasecmp(((SIValue*)p1)->stringval, ((SIValue*)p2)->stringval);
}
