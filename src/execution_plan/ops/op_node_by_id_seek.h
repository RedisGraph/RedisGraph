/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../util/range/unsigned_range.h"

#define ID_RANGE_UNBOUND -1

/* Node by ID seek locates an entity by its ID */
typedef struct {
	OpBase op;
	const char *alias;      // Alias of the node being scanned by this op.
	NodeID minId;           // Min ID to fetch.
	NodeID maxId;           // Max ID to fetch.
} NodeByIdSeek;

OpBase *NewNodeByIdSeekOp(const ExecutionPlan *plan, const char *alias, UnsignedRange *id_range);
