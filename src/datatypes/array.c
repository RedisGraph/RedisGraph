#include "array.h"
#include "../util/arr.h"
#include <limits.h>

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
	if(index < 0 || index >= SIArray_Length(siarray)) return SI_NullVal();
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

int SIArray_ToString(SIValue list, char *buf, size_t len) {
	// minimum length buffer for "[...]\0"
	if(len < 6) return 0;
	// open array with "["
	int bytes_written = snprintf(buf, len, "[");
	// len now holds the actual amount of bytes allowed to be wrriten
	len -= bytes_written;
	uint arrayLen = SIArray_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// if there no more space left in the buffer
		if(len < 2) break;

		// write the next value
		int currentWriteLength = SIValue_ToString(SIArray_Get(list, i), buf + bytes_written, len);
		bytes_written += currentWriteLength;
		len -= currentWriteLength;

		// if there no more space left in the buffer or it is the last element
		if(len < 2 || i == arrayLen - 1) break;

		currentWriteLength = snprintf(buf + bytes_written, len, ", ");
		bytes_written += currentWriteLength;
		len -= currentWriteLength;
	}

	// if there is still room in the buffer, close the array
	if(len >= 2) {
		bytes_written += snprintf(buf + bytes_written, len, "]");
		return bytes_written;
	} else {
		// last write exeeded buffer length, replace with "...]\0"
		snprintf(buf + strlen(buf) - 5, 5, "...]");
		return strlen(buf);
	}
}

void SIArray_Free(SIValue siarray) {
	uint arrayLen = SIArray_Length(siarray);
	for(uint i = 0; i < arrayLen; i++) {
		SIValue value = siarray.array[i];
		SIValue_Free(&value);
	}
	array_free(siarray.array);
}
