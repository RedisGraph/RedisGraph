/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../value.h"
#include "../arithmetic/arithmetic_expression.h"

typedef struct {
	SIValue *keys;                       // SIValues that form the key associated with each group
	AR_ExpNode **aggregationFunctions;   // nodes containing aggregate functions to be evaluated
	uint key_count;                      // number of SIValues in the key
	uint func_count;                     // number of aggregation function values
} Group;

// creates a new group
Group *NewGroup
(
	SIValue *keys,
	uint key_count,
	AR_ExpNode **funcs,
	uint func_count
);

void FreeGroup
(
	Group *group
);

