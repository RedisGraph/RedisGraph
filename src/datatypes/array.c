#include "array.h"
#include "../util/arr.h"

SIValue Array_New(u_int64_t initialCapacity) {

	SIValue siarray;
	siarray.array = array_new(SIValue, initialCapacity);
	siarray.type = T_ARRAY;
	siarray.allocation = M_SELF;
	// add a ref counter hidden from the user
	siarray = Array_Append(siarray, SI_LongVal(1));
	return siarray;
}

SIValue Array_Append(SIValue siarray, SIValue value) {
	// clone and persist incase of pointer values
	SIValue clone = SI_Clone(value);
	SIValue_Persist(&clone);
	// append
	siarray.array = array_append(siarray.array, clone);
	return siarray;
}

SIValue Array_Get(SIValue siarray, u_int64_t index) {
	// check index
	if(index < 0 || index >= Array_Length(siarray))
		return SI_NullVal();
	// return value, offset from the ref counter
	return SI_Clone(siarray.array[index + 1]);
}

u_int32_t Array_Length(SIValue siarray) {
	// return the length without the ref counter
	return array_len(siarray.array) - 1 ;
}

SIValue Array_Clone(SIValue siarray) {
	// increase ref counter
	SIValue refCounter = siarray.array[0];
	siarray.array[0] = SI_LongVal(refCounter.longval + 1);
	return siarray;
}

int Array_ToString(SIValue list, char *buf, size_t len) {
	// minimum length buffer for "[...]\0"
	if(len < 6) return 0;
	// open array with "["
	int bytes_written = snprintf(buf, len, "[");
	// len now holds the actual amount of bytes allowed to be wrriten
	len -= bytes_written;
	uint arrayLen = Array_Length(list);
	for(uint i = 0; i < arrayLen; i ++) {
		// if there no more space left in the buffer
		if(len < 2) break;

		// write the next value
		int currentWriteLength = SIValue_ToString(Array_Get(list, i), buf + bytes_written, len);
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

void Array_Free(SIValue siarray) {
	// decrease ref counter
	SIValue refCounter = siarray.array[0];
	siarray.array[0] = SI_LongVal(refCounter.longval - 1);
	// if last one - free all members and underlying array
	if(siarray.array[0].longval == 0) {
		uint arrayLen = Array_Length(siarray);
		for(uint i = 0; i < arrayLen; i++) {
			SIValue value = Array_Get(siarray, i);
			SIValue_Free(&value);
		}
		array_free(siarray.array);
	}
}