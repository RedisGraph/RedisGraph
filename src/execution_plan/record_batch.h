/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "record.h"

typedef Record* RecordBatch;

// create a new record batch with given capacity
RecordBatch RecordBatch_New
(
	uint capacity
);

// adds record to batch
RecordBatch RecordBatch_Add
(
	RecordBatch *batch,
	Record r
);

uint RecordBatch_Len
(
	RecordBatch batch 
);

// clear batch
void RecordBatch_Clear
(
	RecordBatch batch
);

// return true if given batch is empty
bool RecordBatch_Empty
(
	RecordBatch batch
);

// free batch
void RecordBatch_Free
(
	RecordBatch batch
);

