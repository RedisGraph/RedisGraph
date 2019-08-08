/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Type defines the supported types by the indexing system. The types are powers
 * of 2 so they can be used in bitmasks of matching types.
 *
 * The order of these values is significant, as the delta between values of
 * differing types is used to maintain the Cypher-defined global sort order
 * in the SIValue_Order routine. */
typedef enum
{
    T_MAP = (1 << 0),
    T_NODE = (1 << 1),
    T_EDGE = (1 << 2),
    T_ARRAY = (1 << 3),
    T_PATH = (1 << 4),
    T_DATETIME = (1 << 5),
    T_LOCALDATETIME = (1 << 6),
    T_DATE = (1 << 7),
    T_TIME = (1 << 8),
    T_LOCALTIME = (1 << 9),
    T_DURATION = (1 << 10),
    T_STRING = (1 << 11),
    T_BOOL = (1 << 12), // shares 'longval' representation in SIValue union
    T_INT64 = (1 << 13),
    T_DOUBLE = (1 << 14),
    T_NULL = (1 << 15),
    T_PTR = (1 << 16),
    T_ERROR = (1 << 17), // Represents an error, error message is held inside .stringval
} SIType;

typedef enum
{
    M_NONE = 0,       // SIValue is not heap-allocated
    M_SELF = 0x1,     // SIValue is responsible for freeing its reference
    M_VOLATILE = 0x2, // SIValue does not own its reference and may go out of scope
    M_CONST = 0x4     // SIValue does not own its allocation, but its access is safe
} SIAllocation;

#define SI_TYPE(value) (value).type
#define SI_NUMERIC (T_INT64 | T_DOUBLE)
#define SI_GRAPHENTITY (T_NODE | T_EDGE)
#define SI_ALL (T_MAP | T_NODE | T_EDGE | T_ARRAY | T_PATH | T_DATETIME | T_LOCALDATETIME | T_DATE | T_TIME | T_LOCALTIME | T_DURATION | T_STRING | T_BOOL | T_INT64 | T_DOUBLE | T_NULL | T_PTR)

/* Retrieve the numeric associated with an SIValue without explicitly
 * assigning it a type. */
#define SI_GET_NUMERIC(v) ((v).type == T_DOUBLE ? (v).doubleval : (v).longval)

/* Build an integer return value for a comparison routine in the style of strcmp.
 * This is necessary to construct safe returns when the delta between
 * two values is < 1.0 (and would thus be rounded to 0). */
#define SAFE_COMPARISON_RESULT(a) SIGN(a)

/* Returns 1 if argument is positive, -1 if argument is negative,
 * and 0 if argument is zero.*/
#define SIGN(a) ((a) > 0) - ((a) < 0)

#define DISJOINT INT_MAX
#define COMPARED_NULL INT_MIN

typedef struct SIValue
{
    union {
        int64_t longval;
        double doubleval;
        char *stringval;
        void *ptrval;
        struct SIValue *array;
    };
    SIType type;
    SIAllocation allocation;
} SIValue;

/* Functions to construct an SIValue from a specific input type. */
SIValue SI_LongVal(int64_t i);
SIValue SI_DoubleVal(double d);
SIValue SI_NullVal(void);
SIValue SI_BoolVal(int b);
SIValue SI_PtrVal(void *v);
SIValue SI_Node(void *n);
SIValue SI_Edge(void *e);
SIValue SI_Array(SIValue *array);
SIValue SI_EmptyArray();
SIValue SI_DuplicateStringVal(const char *s); // Duplicate and ultimately free the input string
SIValue SI_ConstStringVal(char
                              *s);     // Neither duplicate nor assume ownership of input string
SIValue SI_TransferStringVal(char *s); // Don't duplicate input string, but assume ownership

/* Functions for copying and guaranteeing memory safety for SIValues. */
// SI_ShareValue creates an SIValue that shares all of the original's allocations.
SIValue SI_ShareValue(const SIValue v);

// SI_CloneValue creates an SIValue that duplicates all of the original's allocations.
SIValue SI_CloneValue(const SIValue v);

// SI_ConstValue creates an SIValue that shares the original's allocations, but does not need to persist them.
SIValue SI_ConstValue(const SIValue v);

/* Functions for copying and guaranteeing memory safety for SIValues. */
// SI_ShareValue creates an SIValue that shares all of the original's allocations.
SIValue SI_ShareValue(const SIValue v);

// SI_CloneValue creates an SIValue that duplicates all of the original's allocations.
SIValue SI_CloneValue(const SIValue v);

// SI_ConstValue creates an SIValue that shares the original's allocations, but does not need to persist them.
SIValue SI_ConstValue(const SIValue v);

// SIValue_MakeVolatile updates an SIValue to mark that its allocations are shared rather than self-owned.
void SIValue_MakeVolatile(SIValue *v);
/* Functions to copy an SIValue. */
SIValue SI_Clone(SIValue v); // If input is a string type, duplicate and assume ownership
SIValue SI_ConstStringVal(char
                              *s);     // Neither duplicate nor assume ownership of input string
SIValue SI_TransferStringVal(char *s); // Don't duplicate input string, but assume ownership

// SIValue_Persist updates an SIValue to duplicate any allocations that may go out of scope in the lifetime of this query.
void SIValue_Persist(SIValue *v);

bool SIValue_IsNull(SIValue v);
bool SIValue_IsNullPtr(SIValue *v);

const char *SIType_ToString(SIType t);
int SIValue_ToString(SIValue v, char *buf, size_t len);

/* Try to read a value as a double.
 * TODO Only used by agg_funcs, consider refactoring. */
int SIValue_ToDouble(const SIValue *v, double *d);

/* Try to parse a value by string. */
SIValue SIValue_FromString(const char *s);

/* Determines number of bytes required to concat strings. */
size_t SIValue_StringConcatLen(SIValue *strings, unsigned int string_count);

/* Concats strings as a comma separated string. */
size_t SIValue_StringConcat(SIValue *strings, unsigned int string_count, char *buf, size_t buf_len);

/* Arithmetic operators for numeric SIValues.
 * The caller is responsible for ensuring that the arguments
 * are numeric types.
 * If both arguments are integer types, the result is as well,
 * otherwise a double is returned. */
SIValue SIValue_Add(const SIValue a, const SIValue b);
SIValue SIValue_Subtract(const SIValue a, const SIValue b);
SIValue SIValue_Multiply(const SIValue a, const SIValue b);
/* SIValue_Divide always returns a double value. */
SIValue SIValue_Divide(const SIValue a, const SIValue b);

/* Compares two SIValues and returns a value similar to strcmp, or
 * the macro DISJOINT if the values were not of comparable types. */
int SIValue_Compare(const SIValue a, const SIValue b);

/* Return a strcmp-style integer value indicating which value is greater according
 * to Cypher's comparability property if applicable, and its orderability property if not.
 * Under Cypher's orderability, where string < boolean < numeric < NULL. */
int SIValue_Order(const SIValue a, const SIValue b);

/* Free an SIValue's internal property if that property is a heap allocation owned
 * by this object. */
void SIValue_Free(SIValue *v);
