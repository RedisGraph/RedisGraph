/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./ops/op.h"
#include "../../../graph/graph.h"
#include "../../execution_plan.h"
#include "../../../resultset/resultset.h"
#include "../../../filter_tree/filter_tree.h"
#include "../../../util/object_pool/object_pool.h"

typedef struct RT_ExecutionPlan {
	RT_OpBase *root;                    // Root operation of overall ExecutionPlan.
	rax *record_map;                    // Mapping between identifiers and record indices.
	ObjectPool *record_pool;
	bool prepared;                      // Indicates if the execution plan is ready for execute.
	const ExecutionPlan *plan_desc;     // Plan description
} RT_ExecutionPlan;

// Allocate a new ExecutionPlan segment.
RT_ExecutionPlan *RT_ExecutionPlan_NewEmptyExecutionPlan(void);

// Creates a new execution plan from AST
RT_ExecutionPlan *RT_NewExecutionPlan(const ExecutionPlan *plan);

/* Prepare an execution plan for execution: optimize, initialize result set schema. */
void RT_ExecutionPlan_PreparePlan(RT_ExecutionPlan *plan);

// Re position filter op
void RT_ExecutionPlan_RePositionFilterOp(RT_ExecutionPlan *plan, RT_OpBase *lower_bound,
									  const RT_OpBase *upper_bound, RT_OpBase *filter);

// Retrieve the map of aliases to Record offsets in this ExecutionPlan segment
rax *RT_ExecutionPlan_GetMappings(const RT_ExecutionPlan *plan);

// Retrieves a Record from the ExecutionPlan's Record pool
Record RT_ExecutionPlan_BorrowRecord(RT_ExecutionPlan *plan);

// Free Record contents and return it to the Record pool
void RT_ExecutionPlan_ReturnRecord(RT_ExecutionPlan *plan, Record r);

// Prints execution plan
void RT_ExecutionPlan_Print(const RT_ExecutionPlan *plan, RedisModuleCtx *ctx);

// Initialize all operations in an ExecutionPlan
void RT_ExecutionPlan_Init(RT_ExecutionPlan *plan);

// Executes plan
ResultSet *RT_ExecutionPlan_Execute(RT_ExecutionPlan *plan);

// Checks if execution plan been drained
bool RT_ExecutionPlan_Drained(RT_ExecutionPlan *plan);

// Drains execution plan
void RT_ExecutionPlan_Drain(RT_ExecutionPlan *plan);

// Profile executes plan
ResultSet *RT_ExecutionPlan_Profile(RT_ExecutionPlan *plan);

// Prints execution plan
void RT_ExecutionPlan_Print(const RT_ExecutionPlan *plan, RedisModuleCtx *ctx);

// Free execution plan
void RT_ExecutionPlan_Free(RT_ExecutionPlan *plan);
