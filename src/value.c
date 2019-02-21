/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "value.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <assert.h>
#include "util/rmalloc.h"

SIValue SI_LongVal(int64_t i) {
  return (SIValue){.longval = i, .type = T_INT64};
}

SIValue SI_DoubleVal(double d) {
  return (SIValue){.doubleval = d, .type = T_DOUBLE};
}

SIValue SI_NullVal(void) {
  return (SIValue){.longval = 0, .type = T_NULL};
}

SIValue SI_BoolVal(int b) {
  return (SIValue) {.longval = b, .type = T_BOOL};
}

SIValue SI_PtrVal(void* v) {
  return (SIValue) {.ptrval = v, .type = T_PTR};
}

SIValue SI_DuplicateStringVal(const char *s) {
  return (SIValue){.stringval = rm_strdup(s), .type = T_STRING};
}

SIValue SI_ConstStringVal(char *s) {
  return (SIValue){.stringval = s, .type = T_CONSTSTRING};
}

SIValue SI_TransferStringVal(char *s) {
  return (SIValue){.stringval = s, .type = T_STRING};
}

SIValue SI_Clone(SIValue v) {
  if (v.type & SI_STRING) {
    // Allocate a new copy of the input's string value
    return SI_DuplicateStringVal(v.stringval);
  }
  SIValue dup;
  memcpy(&dup, &v, sizeof(SIValue));
  return dup;
}

SIValue SI_ShallowCopy(SIValue v) {
  if (v.type & SI_STRING) {
    // Re-use the pointer of the input's string value
    return SI_ConstStringVal(v.stringval);
  }
  SIValue dup;
  memcpy(&dup, &v, sizeof(SIValue));
  return dup;
}

inline int SIValue_IsNull(SIValue v) { return v.type == T_NULL; }
inline int SIValue_IsNullPtr(SIValue *v) {
  return v == NULL || v->type == T_NULL;
}

int SIValue_ToString(SIValue v, char *buf, size_t len) {
  int bytes_written = 0;

  switch (v.type) {
  case T_STRING:
  case T_CONSTSTRING:
    strncpy(buf, v.stringval, len);
    bytes_written = strlen(buf);
    break;
  case T_INT64:
    bytes_written = snprintf(buf, len, "%lld", (long long)v.longval);
    break;
  case T_BOOL:
    bytes_written = snprintf(buf, len, "%s", v.longval ? "true" : "false");
    break;
  case T_DOUBLE:
    bytes_written = snprintf(buf, len, "%f", v.doubleval);
    break;
  case T_NULL:
  default:
    bytes_written = snprintf(buf, len, "NULL");
  }

  return bytes_written;
}

int SIValue_ToDouble(const SIValue *v, double *d) {
  switch (v->type) {
  case T_DOUBLE:
    *d = v->doubleval;
    return 1;
  case T_INT64:
  case T_BOOL:
    *d = (double)v->longval;
    return 1;

  default:
    // cannot convert!
    return 0;
  }
}

SIValue SIValue_FromString(const char *s) {
  char *sEnd = NULL;

  errno = 0;
  double parsedval = strtod(s, &sEnd);
  /* The input was not a complete number or represented a number that
   * cannot be represented as a double.
   * Create a string SIValue. */
  if (sEnd[0] != '\0' || errno == ERANGE) {
    return SI_DuplicateStringVal(s);
  }

  // The input was fully converted; create a double SIValue.
  return SI_DoubleVal(parsedval);
}

size_t SIValue_StringConcatLen(SIValue* strings, unsigned int string_count) {
  size_t length = 0;
  size_t elem_len;

  /* Compute length. */
  for(int i = 0; i < string_count; i ++) {
    /* String elements representing bytes size strings,
     * for all other SIValue types 32 bytes should be enough. */
    elem_len = (strings[i].type & SI_STRING) ? strlen(strings[i].stringval) + 1 : 32;
    length += elem_len;
  }

  /* Account for NULL terminating byte. */
  length += string_count + 1;
  return length;
}

size_t SIValue_StringConcat(SIValue* strings, unsigned int string_count, char *buf, size_t buf_len) {
  size_t offset = 0;

  for (int i = 0; i < string_count; i ++) {
    offset += SIValue_ToString(strings[i], buf + offset, buf_len - offset - 1);
    buf[offset++] = ',';
  }
  /* Backtrack once and discard last delimiter. */
  buf[--offset] = '\0';

  return offset;
}

SIValue SIValue_Add(const SIValue a, const SIValue b) {
  /* Only construct an integer return if both operands are integers. */
  if (a.type & b.type & T_INT64) {
    return SI_LongVal(a.longval + b.longval);
  }
  /* Return a double representation. */
  return SI_DoubleVal(SI_GET_NUMERIC(a) + SI_GET_NUMERIC(b));
}

SIValue SIValue_Subtract(const SIValue a, const SIValue b) {
  /* Only construct an integer return if both operands are integers. */
  if (a.type & b.type & T_INT64) {
    return SI_LongVal(a.longval - b.longval);
  }
  /* Return a double representation. */
  return SI_DoubleVal(SI_GET_NUMERIC(a) - SI_GET_NUMERIC(b));
}

SIValue SIValue_Multiply(const SIValue a, const SIValue b) {
  /* Only construct an integer return if both operands are integers. */
  if (a.type & b.type & T_INT64) {
    return SI_LongVal(a.longval * b.longval);
  }
  /* Return a double representation. */
  return SI_DoubleVal(SI_GET_NUMERIC(a) * SI_GET_NUMERIC(b));
}

SIValue SIValue_Divide(const SIValue a, const SIValue b) {
  /* Always perform floating-point division. */
  return SI_DoubleVal(SI_GET_NUMERIC(a) / (double)SI_GET_NUMERIC(b));
}

int SIValue_Compare(const SIValue a, const SIValue b) {
  /* In order to be comparable, both SIValues must be strings,
   * booleans, or numerics. */
  if (a.type == b.type) {
    switch (a.type) {
      case T_INT64:
      case T_BOOL:
        return a.longval - b.longval;
      case T_DOUBLE:
        return SAFE_COMPARISON_RESULT(a.doubleval - b.doubleval);
      case T_STRING:
      case T_CONSTSTRING:
        // Both inputs are strings of the same SIType
        return strcmp(a.stringval, b.stringval);
      default:
        // Both pointers were of an incomparable type, like a pointer
        return DISJOINT;
    }
  }

  // The inputs have different SITypes - compare them if they
  // are both numerics or both strings of differing types
  if (SI_TYPE(a) & SI_NUMERIC && SI_TYPE(b) & SI_NUMERIC) {
    double diff = SI_GET_NUMERIC(a) - SI_GET_NUMERIC(b);
    return SAFE_COMPARISON_RESULT(diff);
  } else if (SI_TYPE(a) & SI_STRING && SI_TYPE(b) & SI_STRING) {
    return strcmp(a.stringval, b.stringval);
  }

  // Inputs are not comparable
  return DISJOINT;
}

// Return a strcmp-like value wherein -1 indicates that the value
// on the left-hand side is lesser, and 1 indicates that is greater.
int SIValue_Order(const SIValue a, const SIValue b) {
  // If the values are directly comparable, return the comparison result
  int cmp = SIValue_Compare(a, b);
  if (cmp != DISJOINT) return cmp;

  // Cypher's orderability property defines string < boolean < numeric < NULL.
  if (a.type & SI_STRING) {
    return -1;
  } else if (b.type & SI_STRING) {
    return 1;
  } else if (a.type == T_BOOL) {
    return -1;
  } else if (b.type == T_BOOL) {
    return 1;
  } else if (a.type & SI_NUMERIC) {
    return -1;
  } else if (b.type & SI_NUMERIC) {
    return 1;
  }

  // We can reach here if both values are NULL, in which case no order is imposed.
  return 0;
}

void SIValue_Free(SIValue *v) {
	// Only values of type T_STRING are heap allocations owned by the SIValue
	// that need to be freed.
	if (v->type == T_STRING) {
		rm_free(v->stringval);
		v->stringval = NULL;
	}
}

