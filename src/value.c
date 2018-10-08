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

SIValue SI_StringVal(const char *s) {
  return (SIValue){.stringval = strdup(s), .type = T_STRING};
}

SIValue SI_BoolVal(int b) { 
  return (SIValue) {.boolval = b, .type = T_BOOL}; 
}

SIValue SI_PtrVal(void* v) { 
  return (SIValue) {.ptrval = v, .type = T_PTR}; 
}

SIValue SI_Clone(SIValue v) {
  switch (v.type) {
  case T_STRING:
    return SI_StringVal(v.stringval);
  case T_INT32:
    return SI_IntVal(v.intval);
  case T_INT64:
    return SI_LongVal(v.longval);
  case T_UINT:
    return SI_UintVal(v.uintval);  
  case T_BOOL:
    return SI_BoolVal(v.boolval);
  case T_FLOAT:
    return SI_FloatVal(v.floatval);
  case T_DOUBLE:
    return SI_DoubleVal(v.doubleval);
  case T_PTR:
    return SI_PtrVal(v.ptrval);
  case T_INF:
    return SI_InfVal();
  case T_NEGINF:
    return SI_NegativeInfVal();
  case T_NULL:
    return SI_NullVal();
  }
}

SIValue SI_InfVal() { return (SIValue){.intval = 0, .type = T_INF}; }
SIValue SI_NegativeInfVal() { return (SIValue){.intval = 0, .type = T_NEGINF}; }

inline int SIValue_IsInf(SIValue *v) { return v && v->type == T_INF; }
inline int SIValue_IsNegativeInf(SIValue *v) {
  return v && v->type == T_NEGINF;
};

inline int SIValue_IsNull(SIValue v) { return v.type == T_NULL; }
inline int SIValue_IsNullPtr(SIValue *v) {
  return v == NULL || v->type == T_NULL;
}

void SIValue_Free(SIValue *v) {
  if (v->type == T_STRING) {
    free(v->stringval);
    v->stringval = NULL;
  }
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
    v->stringval = strdup(str);

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

inline SIValue SI_NullVal() { return (SIValue){.intval = 0, .type = T_NULL}; }

SIValueVector SI_NewValueVector(size_t cap) {
  return (SIValueVector){
      .vals = calloc(cap, sizeof(SIValue)), .cap = cap, .len = 0};
}

inline void SIValueVector_Append(SIValueVector *v, SIValue val) {
  if (v->len == v->cap) {
    v->cap = v->cap ? MIN(v->cap * 2, 1000) : v->cap + 1;
    v->vals = realloc(v->vals, v->cap * sizeof(SIValue));
  }
  v->vals[v->len++] = val;
}

void SIValueVector_Free(SIValueVector *v) { free(v->vals); }

int SI_LongVal_Cast(SIValue *v, SIType type) {

  if (v->type != T_INT64)
    return 0;

  switch (type) {
  case T_INT64: // do nothing!
    return 1;
  case T_INT32:
    v->intval = (int32_t)v->longval;
    break;
  case T_BOOL:
    v->boolval = v->longval ? 1 : 0;
    break;
  case T_UINT:
    v->uintval = (u_int64_t)v->longval;
    break;
  case T_FLOAT:
    v->floatval = (float)v->longval;
    break;
  case T_DOUBLE:
    v->doubleval = (double)v->longval;
    break;
  case T_STRING: {
    char *buf = malloc(21);
    snprintf(buf, 21, "%lld", (long long)v->longval);
    v->stringval = buf;
    break;
  }  
  default:
    // cannot convert!
    return 0;
  }
  v->type = type;
  return 1;
}
int SI_DoubleVal_Cast(SIValue *v, SIType type) {
  if (v->type != T_DOUBLE)
    return 0;

  switch (type) {
  case T_DOUBLE:
    return 1;
  case T_INT64: // do nothing!
    v->longval = (int64_t)v->doubleval;
    break;
  case T_INT32:
    v->intval = (int32_t)v->doubleval;
    break;
  case T_BOOL:
    v->boolval = v->doubleval != 0 ? 1 : 0;
    break;
  case T_UINT:
    v->uintval = (u_int64_t)v->doubleval;
    break;
  case T_FLOAT:
    v->floatval = (float)v->doubleval;
    break;
  case T_STRING: {
    char *buf = malloc(256);
    snprintf(buf, 256, "%.17f", v->doubleval);
    v->stringval = buf;
    break;
  }  
  default:
    // cannot convert!
    return 0;
  }
  v->type = type;
  return 1;
}

int SI_StringVal_Cast(SIValue *v, SIType type) {

  if (v->type != T_STRING)
    return 0;

  switch (type) {
  case T_STRING:
    return 1;
  // by default we just use the parsing function
  default: {
    SIValue tmp;
    tmp.type = type;
    if (SI_ParseValue(&tmp, v->stringval)) {
      *v = tmp;
      return 1;
    }
  }
  }

  return 0;
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
    return SI_StringVal(s);
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
    elem_len = (strings[i].type == T_STRING) ? strlen(strings[i].stringval) + 1 : 32;
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

int SIValue_ValuesAreComparable(const SIValue a, const SIValue b) {
  return (a.type == b.type) || (a.type & SI_NUMERIC && b.type & SI_NUMERIC);
}

int SIValue_Compare(SIValue a, SIValue b) {
  // Types are identical
  if (a.type == b.type) {
    if (a.type == T_DOUBLE) {
      /* TODO - inf, -inf, and  NaN do not behave canonically in this routine,
       * though they do not break anything. Low-priority bug. */
      double diff = a.doubleval - b.doubleval;
      return COMPARE_RETVAL(diff);
      // return a.doubleval - b.doubleval;
    } else if (a.type == T_STRING) {
      return strcasecmp(a.stringval, b.stringval);
    } else {
      // TODO matching unhandled types - revisit this when introducing other numerics
      return 0;
    }
  }

  // Types differ
  double tmp_a, tmp_b;
  if (SIValue_ToDouble(&a, &tmp_a) && SIValue_ToDouble(&b, &tmp_b)) {
    /* Both values are numeric, and both now have double representations.
     * TODO worry about precision and overflows. This approach will
     * not be adequate if we have high-value longs, especially. */
    double diff = tmp_a - tmp_b;
    return COMPARE_RETVAL(diff);
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
  if (a.type == T_STRING) {
    return -1;
  }
  if (b.type == T_STRING) {
    return 1;
  }

  return b.type - a.type;
}

void SIValue_Print(FILE *outstream, SIValue *v) {
  switch (v->type) {
    case T_STRING:
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
