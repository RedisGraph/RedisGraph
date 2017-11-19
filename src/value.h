#ifndef __SECONDARY_VALUE_H__
#define __SECONDARY_VALUE_H__
#include <stdlib.h>
#include <string.h>
#include "./rmutil/vector.h"

typedef char *SIId;

/* Type defines the supported types by the indexing system. The types are powers
 * of 2 so they can be used in bitmasks of matching types */
typedef enum {
  T_NULL = 0,
  T_STRING = 0x001,
  T_INT32 = 0x002,
  T_INT64 = 0x004,
  T_UINT = 0x008,
  T_BOOL = 0x010,
  T_FLOAT = 0x020,
  T_DOUBLE = 0x040,

  // special types for +inf and -inf on all types:
  T_INF = 0x100,
  T_NEGINF = 0x200,

} SIType;

// binary safe strings
typedef struct {
  char *str;
  size_t len;
} SIString;

SIString SI_WrapString(const char *s);
SIString SIString_Copy(SIString s);

typedef struct {
  union {
    int32_t intval;
    int64_t longval;
    u_int64_t uintval;
    float floatval;
    double doubleval;
    int boolval;
    SIString stringval;
  };
  SIType type;
} SIValue;

typedef struct {
  SIValue *vals;
  size_t len;
  size_t cap;
} SIValueVector;

SIValueVector SI_NewValueVector(size_t cap);

/* Free an SIValue. Since we usually allocate values on the stack, this does not
 * free the actual value object, but the underlying value if needed - basically
 * when it's a string */
void SIValue_Free(SIValue *v);

void SIValueVector_Append(SIValueVector *v, SIValue val);
void SIValueVector_Free(SIValueVector *v);

SIValue SI_StringVal(SIString s);
SIValue SI_StringValC(char *s);
SIValue SI_IntVal(int i);
SIValue SI_LongVal(int64_t i);
SIValue SI_UintVal(u_int64_t i);
SIValue SI_FloatVal(float f);
SIValue SI_DoubleVal(double d);
SIValue SI_NullVal();
SIValue SI_BoolVal(int b);
SIValue SI_Clone(SIValue v);

int SIValue_IsNull(SIValue v);
int SIValue_IsNullPtr(SIValue *v);

SIValue SI_InfVal();
SIValue SI_NegativeInfVal();
int SIValue_IsInf(SIValue *v);
int SIValue_IsNegativeInf(SIValue *v);

/*
 * Convesion functions used to make sure a comparison value in a query is of
 * the right type
 */
int SI_LongVal_Cast(SIValue *v, SIType type);
int SI_DoubleVal_Cast(SIValue *v, SIType type);
int SI_StringVal_Cast(SIValue *v, SIType type);

/* Try to parse a value by string. The value's type should be set to
* anything other than T_NULL, to force strict parsing. */
int SI_ParseValue(SIValue *v, char *str, size_t len);

int SIValue_ToString(SIValue v, char *buf, size_t len);

int SIValue_ToDouble(SIValue *v, double *d);

/* Try to parse a value by string. */
void SIValue_FromString(SIValue *v, char *s, size_t s_len);

/* Concats strings as a comma seperated string. */
size_t SIValue_StringConcat(SIValue* strings, unsigned int string_count, char** concat);

#endif
