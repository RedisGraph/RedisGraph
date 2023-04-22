/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "array.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include <limits.h>
#include "xxhash.h"

SIValue SIArray_New(uint32_t initialCapacity) {
	SIValue siarray;
	siarray.array = array_new(SIValue, initialCapacity);
	siarray.type = T_ARRAY;
	siarray.allocation = M_SELF;
	return siarray;
}

void SIArray_Append(SIValue *siarray, SIValue value) {
	// clone and persist incase of pointer values
	SIValue clone = SI_CloneValue(value);
	// append
	array_append(siarray->array, clone);
}

SIValue SIArray_Get(SIValue siarray, uint32_t index) {
	// check index
	if(index >= SIArray_Length(siarray)) return SI_NullVal();
	return SI_ShareValue(siarray.array[index]);
}

uint32_t SIArray_Length(SIValue siarray) {
	return array_len(siarray.array);
}

/**
  * @brief  Returns true if any of the types in 't' are contained in the array
            or its nested array children, if any
  * @param  siarray: array
  * @param  t: bitmap of types to search for
  * @retval a boolean indicating whether any types were matched
  */
bool SIArray_ContainsType(SIValue siarray, SIType t) {
	uint array_len = SIArray_Length(siarray);
	for(uint i = 0; i < array_len; i++) {
		SIValue elem = siarray.array[i];
		if(SI_TYPE(elem) & t) return true;

		// recursively check nested arrays
		if(SI_TYPE(elem) == T_ARRAY) {
			bool type_is_nested = SIArray_ContainsType(elem, t);
			if(type_is_nested) return true;
		}
	}
	return false;
}

/**
  * @brief  Returns true if the array contains an element equals to 'value'
  * @param  siarray: array
  * @param  value: value to search for
  * @param  comparedNull: indicate if there was a null comparison during the array scan
  * @retval a boolean indicating whether value was found in siarray
  */
bool SIArray_ContainsValue(SIValue siarray, SIValue value, bool *comparedNull) {
	// indicate if there was a null comparison during the array scan
	if(comparedNull) *comparedNull = false;
	uint array_len = SIArray_Length(siarray);
	for(uint i = 0; i < array_len; i++) {
		int disjointOrNull = 0;
		SIValue elem = siarray.array[i];
		int compareValue = SIValue_Compare(elem, value, &disjointOrNull);
		if(disjointOrNull == COMPARED_NULL) {
			if(comparedNull) *comparedNull = true;
			continue;
		}
		if(compareValue == 0) return true;
	}
	return false;
}

bool SIArray_AllOfType(SIValue siarray, SIType t) {
	uint array_len = SIArray_Length(siarray);
	for(uint i = 0; i < array_len; i++) {
		SIValue elem = siarray.array[i];
		if((SI_TYPE(elem) & t) == 0) return false;
	}

	return true;
}

// compare two SIValues, wrt ascending order
static int _siarray_compare_func_asc
(
	const void *a,
	const void *b,
	void *data
) {
	return SIValue_Compare(*(SIValue*)a, *(SIValue*)b, NULL);
}

// compare two SIValues, wrt ascending order
static int _siarray_compare_func_desc
(
	const void *a,
	const void *b,
	void *data
) {
	return SIValue_Compare(*(SIValue*)b, *(SIValue*)a, NULL);
}

// sorts the array in place in ascending\descending order
void SIArray_Sort
(
	SIValue siarray,
	bool ascending
) {
	uint32_t arrayLen = SIArray_Length(siarray);

	if(ascending) {
		sort_r(siarray.array, arrayLen, sizeof(SIValue),
				_siarray_compare_func_asc, (void *)&ascending);
	} else {
		sort_r(siarray.array, arrayLen, sizeof(SIValue),
				_siarray_compare_func_desc, (void *)&ascending);
	}
}

SIValue SIArray_Clone(SIValue siarray) {
	uint arrayLen = SIArray_Length(siarray);
	SIValue newArray = SIArray_New(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIArray_Append(&newArray, siarray.array[i]);
	}
	return newArray;
}

void SIArray_ToString(SIValue list, char **buf, size_t *bufferLen, size_t *bytesWritten) {
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// open array with "["
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "[");
	uint arrayLen = SIArray_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// write the next value
		SIValue_ToString(list.array[i], buf, bufferLen, bytesWritten);
		// if it is not the last element, add ", "
		if(i != arrayLen - 1) {
			if(*bufferLen - *bytesWritten < 64) {
				*bufferLen += 64;
				*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
			}
			*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
		}
	}
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// close array with "]"
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "]");
}

// this method referenced by Java ArrayList.hashCode() method, which takes
// into account the hasing of nested values
XXH64_hash_t SIArray_HashCode(SIValue siarray) {
	SIType t = T_ARRAY;
	XXH64_hash_t hashCode = XXH64(&t, sizeof(t), 0);

	uint arrayLen = SIArray_Length(siarray);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue value = siarray.array[i];
		hashCode = 31 * hashCode + SIValue_HashCode(value);
	}

	return hashCode;
}

// returns the number of bytes required for a binary representation of `arr`
size_t SIArray_BinarySize
(
	const SIValue *arr  // array
) {
	// format:
	// number of elements
	// elements

	size_t    n         = sizeof(uint32_t);
	SIValue   *elements = arr->array;
	uint32_t  len       = array_len(elements);

	for (uint32_t i = 0; i < len; i++) {
		n += SIValue_BinarySize(elements + i);
	}

	return n;
}

// writes a binary representation of `v` into `stream`
void SIArray_ToBinary
(
	FILE *stream,       // stream to populate
	const SIValue *arr  // array
) {
	// format:
	// number of elements
	// elements

	SIValue *elements = arr->array;
	uint32_t len = array_len(elements);

	// write number of elements
	fwrite_assert(&len, sizeof(uint32_t), stream);

	// write each element
	for (uint32_t i = 0; i < len; i++) {
		SIValue_ToBinary(stream, elements + i);
	}
}

// creates an array from its binary representation
// this is the reverse of SIArray_ToBinary
// x = SIArray_FromBinary(SIArray_ToBinary(y));
// x == y
SIValue SIArray_FromBinary
(
	FILE *stream  // stream containing binary representation of an array
) {
	// read number of elements
	uint32_t n;
	fread_assert(&n, sizeof(uint32_t), stream);

	SIValue arr = SIArray_New(n);

	for(uint32_t i = 0; i < n; i++) {
		array_append(arr.array, SIValue_FromBinary(stream));
	}

	return arr;
}

void SIArray_Free(SIValue siarray) {
	uint arrayLen = SIArray_Length(siarray);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue value = siarray.array[i];
		SIValue_Free(value);
	}
	array_free(siarray.array);
}

