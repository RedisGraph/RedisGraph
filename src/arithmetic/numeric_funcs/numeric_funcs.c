/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "numeric_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

#include <math.h>

/* Returns 1 if the operand is a numeric type, and 0 otherwise.
 * This also rejects NULL values. */
static inline int _validate_numeric(const SIValue v) {
	return SI_TYPE(v) & SI_NUMERIC;
}

/* The '+' operator is overloaded to perform string concatenation
 * as well as arithmetic addition. */
SIValue AR_ADD(SIValue *argv, int argc) {
	// Don't modify input.
	SIValue result = SI_CloneValue(argv[0]);
	char buffer[512];
	char *string_arg = NULL;

	if(SIValue_IsNull(argv[0])) return SI_NullVal();
	for(int i = 1; i < argc; i++) {
		result = SIValue_Add(result, argv[i]);
	}
	return result;
}

/* returns the subtracting given values. */
SIValue AR_SUB(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Subtract(result, argv[i]);
	}
	return result;
}

/* returns the multiplication of given values. */
SIValue AR_MUL(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Multiply(result, argv[i]);
	}
	return result;
}

/* returns the division of given values. */
SIValue AR_DIV(SIValue *argv, int argc) {
	SIValue result = argv[0];
	if(!_validate_numeric(result)) return SI_NullVal();

	for(int i = 1; i < argc; i++) {
		if(!_validate_numeric(argv[i])) return SI_NullVal();
		result = SIValue_Divide(result, argv[i]);
	}
	return result;
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

void Register_NumericFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_STRING | T_ARRAY | T_BOOL | T_NULL));
	func_desc = AR_FuncDescNew("add", AR_ADD, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("sub", AR_SUB, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("mul", AR_MUL, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("div", AR_DIV, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("abs", AR_ABS, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("ceil", AR_CEIL, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("floor", AR_FLOOR, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 0);
	func_desc = AR_FuncDescNew("rand", AR_RAND, 0, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("round", AR_ROUND, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, (SI_NUMERIC | T_NULL));
	func_desc = AR_FuncDescNew("sign", AR_SIGN, 1, types, true);
	AR_RegFunc(func_desc);
}

