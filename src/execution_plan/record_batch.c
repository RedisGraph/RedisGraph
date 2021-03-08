/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "record_batch.h"
#include "../util/arr.h"

RecordBatch RecordBatch_New
(
	uint capacity
) {
	return array_new(Record, capacity);
}

RecordBatch RecordBatch_Add
(
	RecordBatch *batch,
	Record r
){
	ASSERT(batch != NULL);

	*batch = array_append(*batch, r);
	return *batch;
}

uint RecordBatch_Len
(
	RecordBatch batch 
) {
	ASSERT(batch != NULL);
	return array_len(batch);
}

// clear batch
void RecordBatch_Clear
(
	RecordBatch batch
){
	ASSERT(batch != NULL);
	array_clear(batch);
}

// return true if given batch is empty
bool RecordBatch_Empty
(
	RecordBatch batch
){
	ASSERT(batch != NULL);
	return array_len(batch) == 0;
}

// free batch
RecordBatch_Free
(
	RecordBatch batch
){
	ASSERT(batch);
	array_free(batch);
}

