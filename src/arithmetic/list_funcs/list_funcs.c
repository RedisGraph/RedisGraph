/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "list_funcs.h"
#include "../func_desc.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include"../../query_ctx.h"
#include "../../util/strutil.h"
#include "../../datatypes/array.h"
#include "../../util/rax_extensions.h"
#include "../string_funcs/string_funcs.h"
#include "../boolean_funcs/boolean_funcs.h"
#include "../numeric_funcs/numeric_funcs.h"

//------------------------------------------------------------------------------
// reduce context
//------------------------------------------------------------------------------

// routine for freeing a reduction function private data
void ListReduceCtx_Free
(
	void *ctx_ptr
) {
	ListReduceCtx *ctx = ctx_ptr;

	if(ctx->exp) {
		AR_EXP_Free(ctx->exp);
	}

	if(ctx->record) {
		rax *mapping = ctx->record->mapping;
		Record_Free(ctx->record);
		raxFree(mapping);
	}

	rm_free(ctx);
}

// Routine for cloning a comprehension function's private data.
void *ListReduceCtx_Clone
(
	void *orig
) {
	ListReduceCtx *ctx = orig;
	// allocate space for the clone
	ListReduceCtx *clone = rm_malloc(sizeof(ListReduceCtx));

	// clone the variadic node
	clone->record           =  NULL;
	clone->variable_idx     =  ctx->variable_idx;
	clone->variable         =  ctx->variable;
	clone->accumulator_idx  =  ctx->accumulator_idx;
	clone->accumulator      =  ctx->accumulator;

	// clone the eval routine
	clone->exp = AR_EXP_Clone(ctx->exp);

	return clone;
}

static void _PopulateReduceCtx
(
	ListReduceCtx *ctx,
	Record outer_record
) {
	rax *record_map = raxClone(outer_record->mapping);

	//--------------------------------------------------------------------------
	// map variable name
	//--------------------------------------------------------------------------

	intptr_t id = raxSize(record_map);
	raxTryInsert(record_map, (unsigned char *)ctx->variable,
				 strlen(ctx->variable), (void *)id, NULL);

	//--------------------------------------------------------------------------
	// map accumulator name
	//--------------------------------------------------------------------------

	id++;
	raxTryInsert(record_map, (unsigned char *)ctx->accumulator,
				 strlen(ctx->accumulator), (void *)id, NULL);

	ctx->record = Record_New(record_map);

	// this could just be assigned to 'id'
	// but for safety we'll use a Record lookup
	ctx->variable_idx = Record_GetEntryIdx(ctx->record, ctx->variable);
	ctx->accumulator_idx = Record_GetEntryIdx(ctx->record, ctx->accumulator);
	ASSERT(ctx->variable_idx != INVALID_INDEX);
	ASSERT(ctx->accumulator_idx != INVALID_INDEX);
}

// forward declaration of property function
SIValue AR_PROPERTY(SIValue *argv, int argc, void *private_data);

// create a list from a given squence of values
// "RETURN [1, '2', True, null]"
SIValue AR_TOLIST(SIValue *argv, int argc, void *private_data) {
	SIValue array = SI_Array(argc);
	for(int i = 0; i < argc; i++) {
		SIArray_Append(&array, argv[i]);
	}
	return array;
}

/* Convert a list of values to a list of new type values.
   Uses the function *converter_ptr to convert each value in input list
*/
static SIValue _AR_TOTYPELIST
(
    SIValue *list,                    				// list of elements to convert
    SIValue (*converter_ptr)(SIValue*, int, void*) 	// convert function e.g. AR_TOFLOAT
) {
	// get array length
	uint32_t arrayLen = SIArray_Length(*list);

	SIValue array = SI_Array(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue v = SIArray_Get(*list, i);
		SIArray_Append(&array, converter_ptr(&v, 1, NULL));
		SIValue_Free(v);
	}
	return array;
}

/* Convert a list of values to a list of boolean values.
   The conversion of each item in list is done using toBooleanOrNull */
SIValue AR_TOBOOLEANLIST(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	if(SI_TYPE(argv[0]) == T_NULL) {
		return SI_NullVal();
	}
	ASSERT(SI_TYPE(argv[0]) == T_ARRAY);
	SIValue originalArray = argv[0];

	return _AR_TOTYPELIST(&originalArray, AR_TO_BOOLEAN);
}

/* Convert a list of values to a list of float values.
   The conversion of each item in list is done using toFloatOrNull */
SIValue AR_TOFLOATLIST(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	if(SI_TYPE(argv[0]) == T_NULL) {
		return SI_NullVal();
	}
	ASSERT(SI_TYPE(argv[0]) == T_ARRAY);
	SIValue originalArray = argv[0];

	return _AR_TOTYPELIST(&originalArray, AR_TOFLOAT);
}

/* Convert a list of values to a list of integer values.
   The conversion of each item in list is done using toIntegerOrNull */
SIValue AR_TOINTEGERLIST(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	if(SI_TYPE(argv[0]) == T_NULL) {
		return SI_NullVal();
	}
	ASSERT(SI_TYPE(argv[0]) == T_ARRAY);
	SIValue originalArray = argv[0];

	return _AR_TOTYPELIST(&originalArray, AR_TOINTEGER);
}

/* Convert a list of values to a list of string values.
   The conversion of each item in list is done using toStringOrNull */
SIValue AR_TOSTRINGLIST(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	if(SI_TYPE(argv[0]) == T_NULL) {
		return SI_NullVal();
	}
	ASSERT(SI_TYPE(argv[0]) == T_ARRAY);
	SIValue originalArray = argv[0];

	// get array length
	uint32_t arrayLen = SIArray_Length(originalArray);

	SIValue array = SI_Array(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue v = SIArray_Get(originalArray, i);
		SIValue vstr = AR_TOSTRING(&v, 1, NULL);
		SIArray_Append(&array, vstr);
		SIValue_Free(vstr);
		SIValue_Free(v);
	}
	return array;
}

/* If given an array, returns a value in a specific index in an array.
   Valid index range is [-arrayLen, arrayLen).
   Invalid index will return null.
   "RETURN [1, 2, 3][0]" will yield 1.

   If given a map or graph entity, returns the property value associated
   with the given key string. */
SIValue AR_SUBSCRIPT(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 2);
	if(SI_TYPE(argv[0]) == T_NULL || SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();
	if(SI_TYPE(argv[0]) & (T_MAP | SI_GRAPHENTITY)) {
		if(SI_TYPE(argv[1]) != T_STRING) {
			Error_SITypeMismatch(argv[1], T_STRING);
			return SI_NullVal();
		}
		/* If the first argument is a map or graph entity, this is a property lookup of a form like:
		 * WITH {val: 5} AS a return a['val']
		 * MATCH (a) RETURN a['val']
		 * Pass the arguments to the AR_PROPERTY function. */
		SIValue property_args[3] = {argv[0], argv[1], SI_LongVal(ATTRIBUTE_ID_NONE)};
		return AR_PROPERTY(property_args, 3, NULL);
	}

	if(SI_TYPE(argv[1]) == T_STRING) {
		// String indexes are only permitted on maps, not arrays.
		Error_SITypeMismatch(argv[1], T_INT64);
		return SI_NullVal();
	}

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
SIValue AR_SLICE(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 3);
	if(SI_TYPE(argv[0]) == T_NULL ||
	   SI_TYPE(argv[1]) == T_NULL ||
	   SI_TYPE(argv[2]) == T_NULL) {
		return SI_NullVal();
	}
	ASSERT(SI_TYPE(argv[0]) == T_ARRAY && SI_TYPE(argv[1]) == T_INT64 && SI_TYPE(argv[2]) == T_INT64);
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
SIValue AR_RANGE(SIValue *argv, int argc, void *private_data) {
	int64_t start = argv[0].longval;
	int64_t end = argv[1].longval;
	int64_t interval = 1;
	if(argc == 3) {
		ASSERT(SI_TYPE(argv[2]) == T_INT64);
		interval = argv[2].longval;
		if(interval == 0) {
			ErrorCtx_RaiseRuntimeException("ArgumentError: step argument to range() can't be 0");
			// Incase expection handler wasn't set, return NULL.
			return SI_NullVal();
		}
	}

	uint64_t size = 0;
	if((end >= start && interval > 0) || (end <= start && interval < 0)) {
		size = 1 + (end - start) / interval;
	}

	SIValue array = SI_Array(size);
	for(uint64_t i = 0; i < size; i++) {
		SIArray_Append(&array, SI_LongVal(start));
		start += interval;
	}
	return array;
}

/* Checks if a value is in a given list.
   "RETURN 3 IN [1, 2, 3]" will return true */
SIValue AR_IN(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 2);
	if(SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();
	ASSERT(SI_TYPE(argv[1]) == T_ARRAY);
	SIValue lookupValue = argv[0];
	SIValue lookupList = argv[1];
	// indicate if there was a null comparison during the array scan
	bool comparedNull = false;
	if(SIArray_ContainsValue(lookupList, lookupValue, &comparedNull)) {
		return SI_BoolVal(true);
	}
	// if there was a null comparison return null, other wise return false as the lookup item did not found
	return comparedNull ? SI_NullVal() : SI_BoolVal(false);
}

/* Return a list/string/map/path size.
   "RETURN size([1, 2, 3])" will return 3
   TODO: when map and path are implemented, add their functionality */
SIValue AR_SIZE(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	SIValue value = argv[0];
	switch(SI_TYPE(value)) {
		case T_ARRAY:
			return SI_LongVal(SIArray_Length(value));
		case T_STRING:
			return SI_LongVal(str_length(value.stringval));
		case T_NULL:
			return SI_NullVal();
		default:
			ASSERT(false);
			return SI_NullVal();
	}
}

/* Return the first member of a list.
   "RETURN head([1, 2, 3])" will return 1 */
SIValue AR_HEAD(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	SIValue value = argv[0];
	if(SI_TYPE(value) == T_NULL) return SI_NullVal();
	ASSERT(SI_TYPE(value) == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	if(arrayLen == 0) return SI_NullVal();
	SIValue retval = SIArray_Get(value, 0);
	SIValue_Persist(&retval);
	return retval;
}

/* Return the last member of a list.
   "RETURN last([1, 2, 3])" will return 3 */
SIValue AR_LAST(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	SIValue value = argv[0];
	if(SI_TYPE(value) == T_NULL) return SI_NullVal();
	ASSERT(SI_TYPE(value) == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	if(arrayLen == 0) return SI_NullVal();
	SIValue retval = SIArray_Get(value, arrayLen-1);
	SIValue_Persist(&retval);
	return retval;
}

/* Return a sublist of a list, which contains all the values withiout the first value.
   "RETURN tail([1, 2, 3])" will return [2, 3] */
SIValue AR_TAIL(SIValue *argv, int argc, void *private_data) {
	ASSERT(argc == 1);
	SIValue value = argv[0];
	if(SI_TYPE(value) == T_NULL) return SI_NullVal();
	ASSERT(SI_TYPE(value) == T_ARRAY);
	uint arrayLen = SIArray_Length(value);
	SIValue array = SI_Array(arrayLen);
	if(arrayLen < 2) return array;
	for(uint i = 1; i < arrayLen; i++) {
		SIArray_Append(&array, SIArray_Get(value, i));
	}
	return array;
}

SIValue AR_REDUCE
(
	SIValue *argv,
	int argc,
	void *private_data
) {
	// reduce(sum = 0, n IN [1,2,3] | sum + n)
	// argv[0] - accumulator initial value
	// argv[1] - array
	// argv[2] - input record

	// return NULL if expected array is NULL
	if(SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();

	// set arguments
	SIValue        accum  =  SI_ShareValue(argv[0]);
	SIValue        list   =  argv[1];
	Record         rec    =  argv[2].ptrval;
	ListReduceCtx  *ctx   =  private_data;

	// on first invocation build the internal record
	if(ctx->record == NULL) _PopulateReduceCtx(ctx, rec);
	Record r = ctx->record;

	// populate record with the contents of the input record
	Record_Clone(rec, r);

	// init accumulator within internal record
	Record_AddScalar(r, ctx->accumulator_idx, accum);

	// evaluate expression for each list element
	// e.g. for `n` in `list`, compute: sum = sum + n
	uint len = SIArray_Length(list);
	for(uint i = 0; i < len; i++) {
		// retrieve the current element
		SIValue elem = SIArray_Get(list, i);

		// set current element to the record
		Record_AddScalar(r, ctx->variable_idx, elem);
		// compute sum = sum + i
		SIValue new_accum = AR_EXP_Evaluate_NoThrow(ctx->exp, r);
		SIValue_Free(accum);
		accum = new_accum;
		// update accumulator within internal record
		Record_AddScalar(r, ctx->accumulator_idx, accum);
	}

	// clear internal record
	Record_Remove(r, ctx->variable_idx);
	Record_Remove(r, ctx->accumulator_idx);

	SIValue_Persist(&accum);
	return accum;
}

void Register_ListFuncs() {
	SIType *types;
	SIType ret_type;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 1);
	array_append(types, SI_ALL);
	ret_type = T_ARRAY;
	func_desc = AR_FuncDescNew("tolist", AR_TOLIST, 0, VAR_ARG_LEN, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("toBooleanList", AR_TOBOOLEANLIST, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("toFloatList", AR_TOFLOATLIST, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("toIntegerList", AR_TOINTEGERLIST, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("toStringList", AR_TOSTRINGLIST, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, T_ARRAY | T_MAP | SI_GRAPHENTITY | T_NULL);
	array_append(types, T_INT64 | T_STRING | T_NULL);
	ret_type = SI_ALL;
	func_desc = AR_FuncDescNew("subscript", AR_SUBSCRIPT, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, T_ARRAY | T_NULL);
	array_append(types, T_INT64 | T_NULL);
	array_append(types, T_INT64 | T_NULL);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("slice", AR_SLICE, 3, 3, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, T_INT64);
	array_append(types, T_INT64);
	array_append(types, T_INT64);
	ret_type = T_ARRAY | T_NULL;
	func_desc = AR_FuncDescNew("range", AR_RANGE, 2, 3, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = T_NULL | T_BOOL;
	func_desc = AR_FuncDescNew("in", AR_IN, 2, 2, types, ret_type, true, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_STRING | T_ARRAY | T_NULL);
	ret_type = T_NULL | T_INT64;
	func_desc = AR_FuncDescNew("size", AR_SIZE, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = SI_ALL;
	func_desc = AR_FuncDescNew("head", AR_HEAD, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = SI_ALL;
	func_desc = AR_FuncDescNew("last", AR_LAST, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	array_append(types, T_ARRAY | T_NULL);
	ret_type = SI_ALL;
	func_desc = AR_FuncDescNew("tail", AR_TAIL, 1, 1, types, ret_type, false, true);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 4);
	array_append(types, SI_ALL);            // accumulator initial value
	array_append(types, T_ARRAY | T_NULL);  // array to iterate over
	array_append(types, T_PTR);             // input record
	ret_type = SI_ALL;
	func_desc = AR_FuncDescNew("reduce", AR_REDUCE, 3, 3, types, ret_type, true, true);
	AR_SetPrivateDataRoutines(func_desc, ListReduceCtx_Free,
							  ListReduceCtx_Clone);
	AR_RegFunc(func_desc);
}

