/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "boolean_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include <assert.h>

SIValue AR_AND(SIValue *argv, int argc) {
	// false AND null evaluates to jalse ; all other null comparisons evaluate to null
	bool res = true;
	int null_count = 0;

	for(int i = 0; i < argc; i++) {
		SIValue v = argv[i];
		switch(SI_TYPE(v)) {
		case T_NULL:
			res &= false;
			null_count ++;
			break;
		case T_BOOL:
		case T_INT64:
			res &= v.longval;
			break;
		default:
			assert(false);
		}
	}

	if(null_count == 2) return SI_NullVal();

	return SI_BoolVal(res);
}

SIValue AR_OR(SIValue *argv, int argc) {
	// true OR null evaluates to true; all other null comparisons evaluate to null
	bool res = false;
	int null_count = 0;

	for(int i = 0; i < argc; i++) {
		SIValue v = argv[i];
		switch(SI_TYPE(v)) {
		case T_NULL:
			res |= false;
			null_count ++;
			break;
		case T_BOOL:
		case T_INT64:
			res |= v.longval;
			break;
		default:
			assert(false);
		}
	}

	// If both arguments are NULL or one argument is NULL and the other is false,
	// OR evaluates to NULL.
	if(null_count == 2 || (null_count == 1 && res == false)) return SI_NullVal();

	return SI_BoolVal(res);
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

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) > 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) > SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_GE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) >= 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) >= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LT(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) < 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) < SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_LE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) <= 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) <= SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_EQ(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) == 0);
	case T_INT64:
	case T_DOUBLE:
		return SI_BoolVal(SI_GET_NUMERIC(a) == SI_GET_NUMERIC(b));
	default:
		assert(false);
	}
}

SIValue AR_NE(SIValue *argv, int argc) {
	SIValue a = argv[0];
	SIValue b = argv[1];

	if(SIValue_IsNull(a) || SIValue_IsNull(b)) return SI_NullVal();

	assert(SI_TYPE(a) == SI_TYPE(b));
	//Type mismatch: expected Float, Integer, Point, String, Date, Time, LocalTime, LocalDateTime or DateTime
	// but was Node (line 1, column 22 (offset: 21)) "match (n),(m) return n > m" ^

	switch(SI_TYPE(a)) {
	case T_STRING:
		return SI_BoolVal(SIValue_Compare(a, b) != 0);
	case T_INT64:
	case T_DOUBLE:
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
	func_desc = AR_FuncDescNew("and", AR_AND, VAR_ARG_LEN, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("or", AR_OR, VAR_ARG_LEN, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("xor", AR_XOR, VAR_ARG_LEN, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_BOOL | T_NULL);
	func_desc = AR_FuncDescNew("not", AR_NOT, VAR_ARG_LEN, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("gt", AR_GT, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("ge", AR_GE, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("lt", AR_LT, 2, types);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, SI_ALL);
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
