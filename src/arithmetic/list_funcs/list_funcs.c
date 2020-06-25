/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "list_funcs.h"
#include "../func_desc.h"
#include "../../datatypes/array.h"
#include "../../util/arr.h"
#include"../../query_ctx.h"

/* Create a list from a given squence of values.
   "RETURN [1, '2', True, null]" */
SIValue AR_TOLIST(SIValue *argv, int argc) {
	SIValue array = SI_Array(argc);
	for(int i = 0; i < argc; i++) {
		SIArray_Append(&array, argv[i]);
	}
	return array;
}

/* Returns a value in a specific index in an array.
   Valid index range is [-arrayLen, arrayLen).
   Invalid index will return null.
   "RETURN [1, 2, 3][0]" will yield 1. */
SIValue AR_SUBSCRIPT(SIValue *argv, int argc) {
	assert(argc == 2);
	if(SI_TYPE(argv[0]) == T_NULL || SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();
	assert(SI_TYPE(argv[0]) == T_ARRAY && SI_TYPE(argv[1]) == T_INT64);
	SIValue list = argv[0];
	int32_t index = (int32_t)argv[1].longval;
	uint32_t arrayLen = SIArray_Length(list);
	// given a negativ index, the accses is calculated as arrayLen+index
	uint32_t absIndex = abs(index);
	// index range can be [-arrayLen, arrayLen) (lower bound inclusive, upper exclusive)
	// this is because 0 = arrayLen+(-arrayLen)
	if((index < 0 && absIndex > arrayLen) || (index > 0 && absIndex >= arrayLen)) return SI_NullVal();
	index = index >= 0 ? index : arrayLen - absIndex;
	SIValue res = SIArray_Get(list, index);
	// clone is in case for nested heap allocated values returned from the array
	return SI_CloneValue(res);
}

/* Return a sub array from an array given a range of indices.
   Valid indices ragne is [-arrayLen, arrayLen).
   If range start value is bigger then range end value an empty list will be returnd.
   If indices are still integers but not in the valid range, only values within the valid range
   will be returned.
   If one of the indices is null, null will be returnd.
   "RETURN [1, 2, 3][0..1]" will yield [1, 2] */
SIValue AR_SLICE(SIValue *argv, int argc) {
	assert(argc == 3);
	if(SI_TYPE(argv[0]) == T_NULL ||
	   SI_TYPE(argv[1]) == T_NULL ||
	   SI_TYPE(argv[2]) == T_NULL) {
		return SI_NullVal();
	}
	assert(SI_TYPE(argv[0]) == T_ARRAY && SI_TYPE(argv[1]) == T_INT64 && SI_TYPE(argv[2]) == T_INT64);
	SIValue array = argv[0];

	// get array length
	uint32_t arrayLen = SIArray_Length(array);

	// get start and end index
	SIValue start = argv[1];
	int32_t startIndex = (int32_t)start.longval;
	SIValue end = argv[2];
	int32_t endIndex = (int32_t)end.longval;

	// if negative index, calculate offset from end
	if(startIndex < 0) startIndex = arrayLen - abs(startIndex);
	// if offset from the end is out of bound, start at 0
	if(startIndex < 0) startIndex = 0;

	// if negative index, calculate offset from end
	if(endIndex < 0) endIndex = arrayLen - abs(endIndex);
	// if index out of bound, end at arrayLen
	if(((int32_t)arrayLen) < endIndex) endIndex = arrayLen;
	// cant go in reverse
	if(endIndex <= startIndex) {
		return SI_EmptyArray();
	}

	SIValue subArray = SI_Array(endIndex - startIndex);
	for(uint i = startIndex; i < endIndex; i++) {
		SIArray_Append(&subArray, SIArray_Get(array, i));
	}
	return subArray;
}

/* Create a new list of integers in the range of [start, end]. If a step was given
   the step between two consecutive list members will be this step.
   If step was not suppllied, it will be default as 1
   "RETURN range(3,8,2)" will yield [3, 5, 7] */
SIValue AR_RANGE(SIValue *argv, int argc) {
	int64_t start = argv[0].longval;
	int64_t end = argv[1].longval;
	int64_t interval = 1;
	if(argc == 3) {
		assert(SI_TYPE(argv[2]) == T_INT64);
		interval = argv[2].longval;
		if(interval < 1) {
			QueryCtx_SetError("ArgumentError: step argument to range() must be >= 1");
			QueryCtx_RaiseRuntimeException();
			// Incase expection handler wasn't set, return NULL.
			return SI_NullVal();
		}
	}

	SIValue array = SI_Array(1 + (end - start) / interval);
	for(; start <= end; start += interval) {
		SIArray_Append(&array, SI_LongVal(start));
	}
	return array;
}

/* Checks if a value is in a given list.
   "RETURN 3 IN [1, 2, 3]" will return true */
SIValue AR_IN(SIValue *argv, int argc) {
	assert(argc == 2);
	if(SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();
	assert(SI_TYPE(argv[1]) == T_ARRAY);
	SIValue lookupValue = argv[0];
	SIValue lookupList = argv[1];
	// indicate if there was a null comparison during the array scan
	bool comparedNull = false;
	uint arrayLen = SIArray_Length(lookupList);
	for(uint i = 0; i < arrayLen; i++) {
		int disjointOrNull = 0;
		int compareValue = SIValue_Compare(lookupValue, SIArray_Get(lookupList, i), &disjointOrNull);
		if(disjointOrNull == COMPARED_NULL) {
			comparedNull = true;
			continue;
		}
		if(compareValue == 0) return SI_BoolVal(true);
	}
	// if there was a null comparison return null, other wise return false as the lookup item did not found
	return comparedNull ? SI_NullVal() : SI_BoolVal(false);
}

/* Return a list/string/map/path size.
   "RETURN size([1, 2, 3])" will return 3
   TODO: when map and path are implemented, add their functionality */
SIValue AR_SIZE(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	switch(SI_TYPE(value)) {
	case T_ARRAY:
		return SI_LongVal(SIArray_Length(value));
	case T_STRING:
		return SI_LongVal(strlen(value.stringval));
	case T_NULL:
		return SI_NullVal();
	default:
		assert(false);
	}
}

/* Return the first member of a list.
   "RETURN head([1, 2, 3])" will return 1 */
SIValue AR_HEAD(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	if(SI_TYPE(value) == T_NULL) return SI_NullVal();
	assert(SI_TYPE(value) == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	if(arrayLen == 0) return SI_NullVal();
	return SIArray_Get(value, 0);
}

/* Return a sublist of a list, which contains all the values withiout the first value.
   "RETURN tail([1, 2, 3])" will return [2, 3] */
SIValue AR_TAIL(SIValue *argv, int argc) {
	assert(argc == 1);
	SIValue value = argv[0];
	if(SI_TYPE(value) == T_NULL) return SI_NullVal();
	assert(SI_TYPE(value) == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	SIValue array = SI_Array(arrayLen);
	if(arrayLen < 2) return array;
	for(uint i = 1; i < arrayLen; i++) {
		SIArray_Append(&array, SIArray_Get(value, i));
	}
	return array;
}

void Register_ListFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	types = array_append(types, SI_ALL);
	func_desc = AR_FuncDescNew("tolist", AR_TOLIST, 0, VAR_ARG_LEN, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, T_ARRAY | T_NULL);
	types = array_append(types, T_INT64 | T_NULL);
	func_desc = AR_FuncDescNew("subscript", AR_SUBSCRIPT, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	types = array_append(types, T_ARRAY | T_NULL);
	types = array_append(types, T_INT64 | T_NULL);
	types = array_append(types, T_INT64 | T_NULL);
	func_desc = AR_FuncDescNew("slice", AR_SLICE, 3, 3, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	types = array_append(types, T_INT64);
	types = array_append(types, T_INT64);
	types = array_append(types, T_INT64);
	func_desc = AR_FuncDescNew("range", AR_RANGE, 2, 3, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	types = array_append(types, SI_ALL);
	types = array_append(types, T_ARRAY | T_NULL);
	func_desc = AR_FuncDescNew("in", AR_IN, 2, 2, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_ARRAY | T_NULL);
	func_desc = AR_FuncDescNew("size", AR_SIZE, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_ARRAY | T_NULL);
	func_desc = AR_FuncDescNew("head", AR_HEAD, 1, 1, types, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_ARRAY | T_NULL);
	func_desc = AR_FuncDescNew("tail", AR_TAIL, 1, 1, types, true);
	AR_RegFunc(func_desc);
}

