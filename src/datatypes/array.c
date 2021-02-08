/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "array.h"
#include "../util/arr.h"
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
	siarray->array = array_append(siarray->array, clone);

}

SIValue SIArray_Get(SIValue siarray, uint32_t index) {
	// check index
	if(index >= SIArray_Length(siarray)) return SI_NullVal();
	return SI_ShareValue(siarray.array[index]);
}

uint32_t SIArray_Length(SIValue siarray) {
	return array_len(siarray.array);
}

SIValue SIArray_Clone(SIValue siarray) {
	uint arrayLen = SIArray_Length(siarray);
	SIValue newArray = SIArray_New(arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		SIArray_Append(&newArray, SIArray_Get(siarray, i));
	}
	return newArray;
}

void SIArray_ToString(SIValue list, char **buf, size_t *bufferLen, size_t *bytesWritten,
					  bool json) {
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// open array with "["
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "[");
	uint arrayLen = SIArray_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// write the next value
		SIValue_ToString(SIArray_Get(list, i), buf, bufferLen, bytesWritten, json);
		// if it is the last element, add ", "
		if(i != arrayLen - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}
	// close array with "]"
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "]");
}

/* This method referenced by Java ArrayList.hashCode() method, which takes
 * into account the hasing of nested values.*/
XXH64_hash_t SIArray_HashCode(SIValue siarray) {
	SIType t = SI_TYPE(siarray);
	XXH64_hash_t hashCode = XXH64(&t, sizeof(t), 0);
	uint arrayLen = SIArray_Length(siarray);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue value = siarray.array[i];
		hashCode = 31 * hashCode + SIValue_HashCode(value);
	}
	return hashCode;
}

void SIArray_Free(SIValue siarray) {
	uint arrayLen = SIArray_Length(siarray);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue value = siarray.array[i];
		SIValue_Free(value);
	}
	array_free(siarray.array);
}

