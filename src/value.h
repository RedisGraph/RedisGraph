/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __SECONDARY_VALUE_H__
#define __SECONDARY_VALUE_H__
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Type defines the supported types by the indexing system. The types are powers
 * of 2 so they can be used in bitmasks of matching types.
 *
 * The order of these values is significant, as the delta between values of
 * differing types is used to maintain the Cypher-defined global sort order
 * in the SIValue_Order routine. */
typedef enum {
  T_NULL = 0,
  T_STRING = 0x001,
  T_INT32 = 0x002,
  T_INT64 = 0x004,
  T_UINT = 0x008,
  T_BOOL = 0x010,
  T_FLOAT = 0x020,
  T_DOUBLE = 0x040,
  T_PTR = 0x080,
  T_CONSTSTRING = 0x100,

  // special types for +inf and -inf on all types:
  T_INF = 0x200,
  T_NEGINF = 0x400,

} SIType;

#define SI_STRING (T_STRING | T_CONSTSTRING)
#define SI_NUMERIC (T_INT32 | T_INT64 | T_UINT | T_FLOAT | T_DOUBLE)
#define SI_TYPE(value) (value).type

/* Returns true if aVal and bVal are of the same type, are both string types, or are both numeric types. */
#define SI_COMPARABLE(aVal, bVal) ((aVal).type == (bVal).type || \
		(((aVal).type & SI_NUMERIC) && (bVal).type & SI_NUMERIC) || \
		(((aVal).type & SI_STRING) && (bVal).type & SI_STRING))

/* Returns 1 if argument is positive, -1 if argument is negative,
 * and 0 if argument is zero (matching the return style of the strcmp family).
 * This is necessary to construct safe integer returns when the delta between
 * two double values is < 1.0 (and would thus be rounded to 0). */
#define COMPARE_RETVAL(a) ((a) > 0) - ((a) < 0)

#define DISJOINT INT_MAX

typedef struct {
  union {
    int32_t intval;
    int64_t longval;
    u_int64_t uintval;
    float floatval;
    double doubleval;
    int boolval;
    char *stringval;
    void* ptrval;
  };
  SIType type;
} SIValue;

/* Functions to construct an SIValue from a specific input type. */
SIValue SI_IntVal(int i);
SIValue SI_LongVal(int64_t i);
SIValue SI_UintVal(u_int64_t i);
SIValue SI_FloatVal(float f);
SIValue SI_DoubleVal(double d);
SIValue SI_NullVal(void);
SIValue SI_BoolVal(int b);
SIValue SI_PtrVal(void* v);
SIValue SI_DuplicateStringVal(const char *s); // Duplicate and ultimately free the input string
SIValue SI_ConstStringVal(char *s);           // Neither duplicate nor assume ownership of input string
SIValue SI_TransferStringVal(char *s);        // Don't duplicate input string, but assume ownership

/* Functions to copy an SIValue. */
SIValue SI_Clone(SIValue v);               // If input is a string type, duplicate and assume ownership
SIValue SI_ShallowCopy(SIValue v);         // Don't duplicate any inputs

int SIValue_IsNull(SIValue v);
int SIValue_IsNullPtr(SIValue *v);

/* Try to parse a value by string. The value's type should be set to
* anything other than T_NULL, to force strict parsing. */
int SI_ParseValue(SIValue *v, char *str);

int SIValue_ToString(SIValue v, char *buf, size_t len);

int SIValue_ToDouble(SIValue *v, double *d);

/* Try to parse a value by string. */
SIValue SIValue_FromString(const char *s);

/* Determines number of bytes required to concat strings. */
size_t SIValue_StringConcatLen(SIValue* strings, unsigned int string_count);

/* Concats strings as a comma separated string. */
size_t SIValue_StringConcat(SIValue* strings, unsigned int string_count, char *buf, size_t buf_len);

/* Compares two SIValues and returns a value similar to strcmp, or
 * the macro DISJOINT if the values were not of comparable types. */
int SIValue_Compare(SIValue a, SIValue b);

/* Return a strcmp-style integer value indicating which value is greater according
 * to Cypher's comparability property if applicable, and its orderability property if not.
 * Under Cypher's orderability, where string < boolean < numeric < NULL. */
int SIValue_Order(const SIValue a, const SIValue b);

void SIValue_Print(FILE *outstream, SIValue *v);

/* Free an SIValue's internal property if that property is a heap allocation owned
 * by this object. This is only the case when the type is T_STRING. */
void SIValue_Free(SIValue *v);

#endif // __SECONDARY_VALUE_H__

