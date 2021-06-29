/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "numeric_funcs.h"
#include "RG.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

#include <math.h>
#include <errno.h>

/* Returns 1 if the operand is a numeric type, and 0 otherwise.
 * This also rejects NULL values. */
static inline int _validate_numeric(const SIValue v) {
	return SI_TYPE(v) & SI_NUMERIC;
}

/* The '+' operator is overloaded to perform string concatenation
 * as well as arithmetic addition. */
SIValue AR_ADD(SIValue *argv, int argc) {
	return SIValue_Add(argv[0], argv[1]);
}

/* returns the subtracting given values. */
SIValue AR_SUB(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();
	return SIValue_Subtract(argv[0], argv[1]);
}

/* returns the multiplication of given values. */
SIValue AR_MUL(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();
	return SIValue_Multiply(argv[0], argv[1]);
}

/* returns the division of given values. */
SIValue AR_DIV(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();
	return SIValue_Divide(argv[0], argv[1]);
}

SIValue AR_MODULO(SIValue *argv, int argc) {
	if(SIValue_IsNull(argv[0]) || SIValue_IsNull(argv[1])) return SI_NullVal();
	return SIValue_Modulo(argv[0], argv[1]);
}

/* TODO All AR_* functions need to emit appropriate failures when provided
 * with arguments of invalid types and handle multiple arguments. */
/* returns the absolute value of the given number. */
SIValue AR_ABS(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	if(SI_GET_NUMERIC(argv[0]) < 0) return SIValue_Multiply(argv[0], SI_LongVal(-1));
	return argv[0];
}

/* returns the smallest floating point number that is greater than or equal to the given number and equal to a mathematical integer. */
SIValue AR_CEIL(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = ceil(result.doubleval);

	return result;
}

/* returns the largest floating point number that is less than or equal to the given number and equal to a mathematical integer. */
SIValue AR_FLOOR(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = floor(result.doubleval);

	return result;
}

/* returns a random floating point number in the range from 0 to 1; i.e. [0,1]. The numbers returned follow an approximate uniform distribution. */
SIValue AR_RAND(SIValue *argv, int argc) {
	return SI_DoubleVal((double)rand() / (double)RAND_MAX);
}

/* returns the value of the given number rounded to the nearest integer. */
SIValue AR_ROUND(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();
	// No modification is required for non-decimal values
	if(SI_TYPE(result) == T_DOUBLE) result.doubleval = round(result.doubleval);

	return result;
}

/* returns the signum of the given number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number. */
SIValue AR_SIGN(SIValue *argv, int argc) {
	if(!_validate_numeric(argv[0])) return SI_NullVal();
	int64_t sign = SIGN(SI_GET_NUMERIC(argv[0]));
	return SI_LongVal(sign);
}

SIValue AR_TOINTEGER(SIValue *argv, int argc) {
	SIValue arg = argv[0];
	char *sEnd = NULL;

	switch(SI_TYPE(arg)) {
	case T_NULL:
		return SI_NullVal();
	case T_INT64:
		return arg;
	case T_DOUBLE:
		// Remove floating point.
		return SI_LongVal(floor(arg.doubleval));
	case T_STRING:
		errno = 0;
		double parsedval = strtod(arg.stringval, &sEnd);
		/* The input was not a complete number or represented a number that
		 * cannot be represented as a double. */
		if(sEnd[0] != '\0' || errno == ERANGE) return SI_NullVal();
		// Remove floating point.
		return SI_LongVal(floor(parsedval));
	default:
		ASSERT(false);
		return SI_NullVal();
	}
}

// returns the square root of a number
SIValue AR_SQRT(SIValue *argv, int argc) {
	SIValue arg = argv[0];
	// return NULL if input is none numeric
	if(!_validate_numeric(arg)) return SI_NullVal();
	// return sqrt of input
	return SI_DoubleVal(sqrt(SI_GET_NUMERIC(arg)));
}

void Register_NumericFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_STRING | T_ARRAY | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("add", AR_ADD, 2, 2, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("sub", AR_SUB, 2, 2, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("mul", AR_MUL, 2, 2, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("div", AR_DIV, 2, 2, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("mod", AR_MODULO, 2, 2, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("abs", AR_ABS, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("ceil", AR_CEIL, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("floor", AR_FLOOR, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("rand", AR_RAND, 0, 0, types, false, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("round", AR_ROUND, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("sign", AR_SIGN, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_STRING | T_NULL));
	func_desc = AR_FuncDescNew("tointeger", AR_TOINTEGER, 1, 1, types, true, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("sqrt", AR_SQRT, 1, 1, types, true, false);
	AR_RegFunc(func_desc);
}

