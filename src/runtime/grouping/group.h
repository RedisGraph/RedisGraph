/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../storage/datatypes/value.h"
#include "../../IR/arithmetic_expression/arithmetic_expression.h"

typedef struct {
	SIValue *keys;                       // SIValues that form the key associated with each group
	AR_ExpNode **aggregationFunctions;   // Nodes containing aggregate functions to be evaluated
	uint key_count;                      // Number of SIValues in the key
	uint func_count;                     // Number of aggregation function values
	Record r;                            // Representative record for all aggregated records in group
} Group;

// creates a new group
Group *NewGroup
(
	SIValue *keys,
	uint key_count,
	AR_ExpNode **funcs,
	uint func_count,
	Record r
);

void FreeGroup
(
	Group *group
);

