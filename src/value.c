/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "value.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <assert.h>
#include "util/rmalloc.h"

SIValue SI_IntVal(int i) { return (SIValue){.intval = i, .type = T_INT32}; }

SIValue SI_LongVal(int64_t i) {
  return (SIValue){.longval = i, .type = T_INT64};
}

SIValue SI_UintVal(u_int64_t i) {
  return (SIValue){.uintval = i, .type = T_UINT};
}

SIValue SI_FloatVal(float f) {
  return (SIValue){.floatval = f, .type = T_FLOAT};
}

SIValue SI_DoubleVal(double d) {
  return (SIValue){.doubleval = d, .type = T_DOUBLE};
}

SIValue SI_NullVal(void) {
  return (SIValue){.intval = 0, .type = T_NULL};
}

SIValue SI_BoolVal(int b) {
  return (SIValue) {.boolval = b, .type = T_BOOL};
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

int _parseInt(SIValue *v, char *str) {

  errno = 0; /* To distinguish success/failure after call */
  char *endptr = NULL;
  long long int val = strtoll(str, &endptr, 10);

  if (errno == ERANGE || (errno != 0 && val == 0)) {
    perror("strtoll");
    return 0;
  }

  if (endptr == str) {
    fprintf(stderr, "No digits were found\n");
    return 0;
  }

  switch (v->type) {
  case T_INT32:
    v->intval = val;
    break;
  case T_INT64:
    v->longval = val;
    break;
  case T_UINT:
    v->uintval = (u_int64_t)val;
    break;
  default:
    return 0;
  }
  return 1;
}

int _parseBool(SIValue *v, char *str) {
  int len = strlen(str);
  if ((len == 1 && !strcmp("1", str)) ||
      (len == 4 && !strcasecmp("true", str))) {
    v->boolval = 1;
    return 1;
  }

  if ((len == 1 && !strcmp("0", str)) ||
      (len == 5 && !strcasecmp("false", str))) {
    v->boolval = 0;
    return 1;
  }
  return 0;
}

int _parseFloat(SIValue *v, char *str) {
  errno = 0; /* To distinguish success/failure after call */
  char *endptr = NULL;
  double val = strtod(str, &endptr);

  /* Check for various possible errors */
  if (errno != 0 || (endptr == str && val == 0)) {
    return 0;
  }

  switch (v->type) {
  case T_FLOAT:
    v->floatval = (float)val;
    break;
  case T_DOUBLE:
    v->doubleval = val;
    break;
  default:
    return 0;
    break;
  }

  return 1;
}

int SI_ParseValue(SIValue *v, char *str) {
  switch (v->type) {

  case T_STRING:
    v->stringval = rm_strdup(str);
    break;
  case T_CONSTSTRING:
    v->stringval = str;
    break;
  case T_INT32:
  case T_INT64:
  case T_UINT:
    return _parseInt(v, str);

  case T_BOOL:
    return _parseBool(v, str);

  case T_FLOAT:
  case T_DOUBLE:
    return _parseFloat(v, str);

  case T_NULL:
  default:
    return 0;
  }

  return 1;
}

int SIValue_ToString(SIValue v, char *buf, size_t len) {
  int bytes_written = 0;

  switch (v.type) {
  case T_STRING:
  case T_CONSTSTRING:
    strncpy(buf, v.stringval, len);
    bytes_written = strlen(buf);
    break;
  case T_INT32:
    bytes_written = snprintf(buf, len, "%d", v.intval);
    break;
  case T_INT64:
    bytes_written = snprintf(buf, len, "%lld", (long long)v.longval);
    break;
  case T_UINT:
    bytes_written = snprintf(buf, len, "%llu", (long long)v.uintval);
    break;
  case T_BOOL:
    bytes_written = snprintf(buf, len, "%s", v.boolval ? "true" : "false");
    break;

  case T_FLOAT:
    bytes_written = snprintf(buf, len, "%f", v.floatval);
    break;
  case T_DOUBLE:
    bytes_written = snprintf(buf, len, "%f", v.doubleval);
    break;
  case T_INF:
    bytes_written = snprintf(buf, len, "+inf");
    break;
  case T_NEGINF:
    bytes_written = snprintf(buf, len, "-inf");
    break;
  case T_NULL:
  default:
    bytes_written = snprintf(buf, len, "NULL");
  }

  return bytes_written;
}

int SIValue_ToDouble(SIValue *v, double *d) {
  switch (v->type) {
  case T_DOUBLE:
    *d = v->doubleval;
    return 1;
  case T_INT64: // do nothing!
    *d = (double)v->longval;
    return 1;
  case T_INT32:
    *d = (double)v->intval;
    return 1;
  case T_UINT:
    *d = (double)v->uintval;
    return 1;
  case T_FLOAT:
    *d = (double)v->floatval;
    return 1;
  case T_BOOL:
    *d = (double)v->boolval;
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

int SIValue_Compare(SIValue a, SIValue b) {
  // Types are directly comparable
  if (SI_COMPARABLE(a, b)) {
    // Use strcmp if values are string types
    if (a.type & SI_STRING) return strcmp(a.stringval, b.stringval);

    // Attempt to cast both values to doubles
    double tmp_a, tmp_b;
    if (SIValue_ToDouble(&a, &tmp_a) && SIValue_ToDouble(&b, &tmp_b)) {
      /* Both values are numeric, and both now have double representations.
       * TODO worry about precision and overflows. This approach will
       * not be adequate if we have high-value longs, especially.
       * TODO Special values like inf, -inf, and NaN will compare properly
       * here, but may not satisfy the prescribed openCypher sort order. */
      double diff = tmp_a - tmp_b;
      return COMPARE_RETVAL(diff);
    }

    // We can reach this point if receiving two SIValue pointers, which
    // should not be possible right now.
    assert(0);
  }

  /* Types differ and are not comparable, so we will fall back to satisfying
   * ordering constraints.
   * TODO Later, we may want to separate ordering logic from comparison logic -
   * the below is useful for ORDER BY clauses, but could be misleading in WHERE clauses.
   *
   * Cypher specifies the following ascending order of disjoint types:
   * - String
   * - Number
   * - NULL
   */
  if (a.type & SI_STRING) {
    return 1;
  } else if (b.type & SI_STRING) {
    return -1;
  }

  return b.type - a.type;
}

void SIValue_Print(FILE *outstream, SIValue *v) {
  switch (v->type) {
    case T_STRING:
    case T_CONSTSTRING:
      fprintf(outstream, "%s", v->stringval);
      break;
    case T_INT32:
      fprintf(outstream, "%d", v->intval);
      break;
    case T_INT64:
      fprintf(outstream, "%lld", (long long)v->longval);
      break;
    case T_UINT:
      fprintf(outstream, "%u", v->intval);
      break;
    case T_FLOAT:
      fprintf(outstream, "%lf", v->floatval);
      break;
    case T_DOUBLE:
      fprintf(outstream, "%lf", v->doubleval);
      break;
    default:
      break;
  }
}

void SIValue_Free(SIValue *v) {
	// Only values of type T_STRING are heap allocations owned by the SIValue
	// that need to be freed.
	if (v->type == T_STRING) {
		rm_free(v->stringval);
		v->stringval = NULL;
	}
}

