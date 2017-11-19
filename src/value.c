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


inline SIString SI_WrapString(const char *s) {
  return (SIString){(char *)s, strlen(s)};
}

SIValue SI_StringVal(SIString s) {
  return (SIValue){.stringval = s, .type = T_STRING};
}
SIValue SI_StringValC(char *s) {
  return (SIValue){.stringval = SI_WrapString(s), .type = T_STRING};
}

SIValue SI_BoolVal(int b) { return (SIValue){.boolval = b, .type = T_BOOL}; }

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
  case T_INF:
    return SI_InfVal();
  case T_NEGINF:
    return SI_NegativeInfVal();
  case T_NULL:
    return SI_NullVal();
  }
}

SIString SIString_Copy(SIString s) {
  char *b = malloc(s.len + 1);
  memcpy(b, s.str, s.len);

  b[s.len] = 0;
  return (SIString){.str = b, .len = s.len};
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
    free(v->stringval.str);
    v->stringval.str = NULL;
    v->stringval.len = 0;
  }
}

int _parseInt(SIValue *v, char *str, size_t len) {

  errno = 0; /* To distinguish success/failure after call */
  char *endptr = str + len;
  long long int val = strtoll(str, &endptr, 10);

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
      (errno != 0 && val == 0)) {
    perror("strtol");
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
int _parseBool(SIValue *v, char *str, size_t len) {

  if ((len == 1 && !strncmp("1", str, len)) ||
      (len == 4 && !strncasecmp("true", str, len))) {
    v->boolval = 1;
    return 1;
  }

  if ((len == 1 && !strncmp("0", str, len)) ||
      (len == 5 && !strncasecmp("false", str, len))) {
    v->boolval = 0;
    return 1;
  }
  return 0;
}

int _parseFloat(SIValue *v, char *str, size_t len) {
  errno = 0; /* To distinguish success/failure after call */
  char *endptr = str + len;
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

int SI_ParseValue(SIValue *v, char *str, size_t len) {
  switch (v->type) {

  case T_STRING:
    v->stringval.str = str;
    v->stringval.len = len;

    break;
  case T_INT32:
  case T_INT64:
  case T_UINT:
    return _parseInt(v, str, len);

  case T_BOOL:
    return _parseBool(v, str, len);

  case T_FLOAT:
  case T_DOUBLE:
    return _parseFloat(v, str, len);

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
    // bytes_written = snprintf(buf, len, "\"%.*s\"", (int)v.stringval.len, v.stringval.str);
    bytes_written = snprintf(buf, len, "%.*s", (int)v.stringval.len, v.stringval.str);
    break;
  case T_INT32:
    bytes_written = snprintf(buf, len, "%d", v.intval);
    break;
  case T_INT64:
    bytes_written = snprintf(buf, len, "%lld", (long long)v.longval);
    break;
  case T_UINT:
    bytes_written = snprintf(buf, len, "%zd", v.uintval);
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
    v->stringval = SI_StringValC(buf).stringval;
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
    v->stringval = SI_StringValC(buf).stringval;
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
    if (SI_ParseValue(&tmp, v->stringval.str, v->stringval.len)) {
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

void SIValue_FromString(SIValue *v, char *s, size_t s_len) {
  int numeric = 1;
  int i;
  char c;

  /* Scan string, see if we can find any none numeric characters int it. */
  for(i = 0; i < s_len; i++) {
    c = s[i];
    
    if(!isdigit(c) && c != '.' && c != '-') {
      numeric = 0;
      v->type = T_STRING;
      break;
    }
  }

  if(numeric) {
    v->type = T_DOUBLE;
  }

  SI_ParseValue(v, s, s_len);
}

size_t SIValue_StringConcat(SIValue* strings, unsigned int string_count, char** concat) {
  int i;
  size_t length = 0;
  size_t offset = 0;
  /* Compute length. */
  for(i = 0; i < string_count; i++) {
    /* Element string representation bytes size, strings are 
    * srounded by double quotes,
    * for all other SIValue types 32 bytes should be enough. */
    size_t len = (strings[i].type == T_STRING) ? strings[i].stringval.len + 2 : 32;
    length += len;
  }

  /* Account for delimiters and NULL terminating byte. */
  length += string_count + 1;
  *concat = calloc(length, sizeof(char));

  for(i = 0; i < string_count; i++) {
    offset += SIValue_ToString(strings[i], (*concat) + offset, length - offset);
    (*concat)[offset++] = ',';
  }
  /* Backtrack once. */
  offset--;

  /* Discard last delimiter. */
  (*concat)[offset] = 0;
  return offset;
}
