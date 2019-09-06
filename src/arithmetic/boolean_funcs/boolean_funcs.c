/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "boolean_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include <assert.h>

#define CONTAINS_NULL 2 // Macro used for efficiently evaluating 3-valued truth table

SIValue AR_AND(SIValue *argv, int argc) {
	// false AND null evaluates to false ; all other null comparisons evaluate to null

	SIValue a = argv[0];
	SIValue b = argv[1];

	int a_val = SIValue_IsNull(a) ? CONTAINS_NULL : a.longval;
	int b_val = SIValue_IsNull(b) ? CONTAINS_NULL : b.longval;
	bool has_nulls = (a_val | b_val) & CONTAINS_NULL;
	int val = a_val & b_val;

	// If one argument is null and the other is false, returns false.
	if(has_nulls && !val) return SI_BoolVal(false);
	if(has_nulls) return SI_NullVal(); // Else if an argument is null, returns null.
	return SI_BoolVal(val); // Else return the logical AND.
}

SIValue AR_OR(SIValue *argv, int argc) {
	// true OR null evaluates to true; all other null comparisons evaluate to null

	SIValue a = argv[0];
	SIValue b = argv[1];

	int a_val = SIValue_IsNull(a) ? CONTAINS_NULL : a.longval;
	int b_val = SIValue_IsNull(b) ? CONTAINS_NULL : b.longval;
	int val = a_val | b_val;

	if(val & 1) return SI_BoolVal(true);         // If at least one argument is true, returns true.
	if(val & CONTAINS_NULL) return SI_NullVal(); // Else if an argument is null, returns null.
	return SI_BoolVal(false);                    // If both arguments are false, returns false
}

SIValue AR_XOR(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a)) return SI_NullVal();
	if(SIValue_IsNull(b)) return SI_NullVal();

	bool res = a.longval != b.longval;
	return SI_BoolVal(res);
}

SIValue AR_NOT(SIValue *argv, int argc) {
	SIValue a = argv[0];
	if(SIValue_IsNull(a)) return SI_NullVal();

	if(SI_TYPE(a) & (SI_NUMERIC | T_BOOL)) return SI_BoolVal(!SI_GET_NUMERIC(a));
	// String, Node, Edge, Ptr all evaluate to true.
	return SI_BoolVal(false);
}

SIValue AR_GT(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Emit error when attempting to compare invalid types
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) {
		char *error;
		asprintf(&error, "Type mismatch: expected %s but was %s", SIType_ToString(SI_TYPE(a)),
				 SIType_ToString(SI_TYPE(b)));
		QueryCtx_SetError(error);
		return SI_NullVal(); // The return doesn't matter, as the caller will check for errors.
	}

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) > 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) > SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_GE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Emit error when attempting to compare invalid types
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) {
		char *error;
		asprintf(&error, "Type mismatch: expected %s but was %s", SIType_ToString(SI_TYPE(a)),
				 SIType_ToString(SI_TYPE(b)));
		QueryCtx_SetError(error);
		return SI_NullVal(); // The return doesn't matter, as the caller will check for errors.
	}

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) >= 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) >= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LT(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Emit error when attempting to compare invalid types
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) {
		char *error;
		asprintf(&error, "Type mismatch: expected %s but was %s", SIType_ToString(SI_TYPE(a)),
				 SIType_ToString(SI_TYPE(b)));
		QueryCtx_SetError(error);
		return SI_NullVal(); // The return doesn't matter, as the caller will check for errors.
	}

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) < 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) < SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Emit error when attempting to compare invalid types
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) {
		char *error;
		asprintf(&error, "Type mismatch: expected %s but was %s", SIType_ToString(SI_TYPE(a)),
				 SIType_ToString(SI_TYPE(b)));
		QueryCtx_SetError(error);
		return SI_NullVal(); // The return doesn't matter, as the caller will check for errors.
	}

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) <= 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) <= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_EQ(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Non-comparable types cannot equal each other.
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) return SI_BoolVal(false);

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) == 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) == SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_NE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	// Comparisons with NULL values always return NULL.
	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	// Non-comparable types cannot equal each other.
	if(!SI_VALUES_ARE_COMPARABLE(a, b)) return SI_BoolVal(true);

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) != 0);
	case T_INT64:
	case T_DOUBLE:
	case T_BOOL:
		return SI_BoolVal(SI_GET_NUMERIC(a) != SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

void Register_BooleanFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	types = array_append(types, T_BOOL | T_NULL);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("and", AR_AND, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, T_BOOL | T_NULL);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("or", AR_OR, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, T_BOOL | T_NULL);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("xor", AR_XOR, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("not", AR_NOT, 1, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("gt", AR_GT, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("ge", AR_GE, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("lt", AR_LT, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	types = array_append(types, (SI_NUMERIC | T_STRING | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("le", AR_LE, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("eq", AR_EQ, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("neq", AR_NE, 2, types);
	AR_RegFunc(func_desc);
}

