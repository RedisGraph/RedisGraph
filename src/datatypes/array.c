#include "array.h"
#include "../util/arr.h"

SIValue Array_New(u_int64_t initialCapacity) {

	SIValue siarray;
	siarray.array = array_new(SIValue, initialCapacity);
	siarray.type = T_ARRAY;
	siarray.allocation = M_SELF;
	Array_Append(siarray, SI_LongVal(1));
	return siarray;
}

SIValue Array_Append(SIValue siarray, SIValue value) {
	SIValue clone = SI_Clone(value);
	SIValue_Persist(&clone);
	siarray.array = array_append(siarray.array, clone);
	return siarray;
}

SIValue Array_Get(SIValue siarray, u_int64_t index) {
	if(index < 0 || index >= Array_Length(siarray))
		return SI_NullVal();
	return siarray.array[index + 1];
}

u_int32_t Array_Length(SIValue siarray) {
	return array_len(siarray.array) - 1 ;
}

SIValue Array_Clone(SIValue siarray) {
	SIValue refCounter = siarray.array[0];
	siarray.array[0] = SI_LongVal(refCounter.longval + 1);
	return siarray;
}

void Array_Free(SIValue siarray) {
	SIValue refCounter = siarray.array[0];
	siarray.array[0] = SI_LongVal(refCounter.longval - 1);
	if(siarray.array[0].longval == 0) {
		uint arrayLen = Array_Length(siarray);
		for(uint i = 0; i < arrayLen; i++) {
			SIValue value = Array_Get(siarray, i);
			SIValue_Free(&value);
		}
		array_free(siarray.array);
	}
}