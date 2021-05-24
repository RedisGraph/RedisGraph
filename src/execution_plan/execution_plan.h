/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./ops/op.h"
#include "../graph/graph.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"
#include "../util/object_pool/object_pool.h"

// execution plan abort reasons
typedef enum {
	EXEC_PLAN_ABORT_NONE = 0,        // execution wasn't aborted
	EXEC_PLAN_ABORT_TIMEOUT,         // execution timedout
	EXEC_PLAN_ABORT_OPTIMISTIC_READ  // optimistic read failed
} ExecutionPlan_AbortReason;

typedef struct ExecutionPlan ExecutionPlan;

struct ExecutionPlan {
	OpBase *root;                            // Root operation of overall ExecutionPlan.
	AST *ast_segment;                        // The segment which the current ExecutionPlan segment is built from.
	rax *record_map;                         // Mapping between identifiers and record indices.
	QueryGraph *query_graph;                 // QueryGraph representing all graph entities in this segment.
	QueryGraph **connected_components;       // Array of all connected components in this segment.
	ObjectPool *record_pool;
	bool prepared;                           // Indicates if the execution plan is ready for execute.
	int ref_count;                           // Number of active references.
	ExecutionPlan_AbortReason abort_reason;  // Reason why execution been aborted.
};

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(void);

/* Prepare an execution plan for execution: optimize, initialize result set schema. */
void ExecutionPlan_PreparePlan(ExecutionPlan *plan);

/* Allocate a new ExecutionPlan segment. */
ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void);

/* Build a tree of operations that performs all the work required by the clauses of the current AST. */
void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan);

/* Re position filter op. */
void ExecutionPlan_RePositionFilterOp(ExecutionPlan *plan, OpBase *lower_bound,
									  const OpBase *upper_bound, OpBase *filter);

/* Retrieve the map of aliases to Record offsets in this ExecutionPlan segment. */
rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan);

/* Retrieves a Record from the ExecutionPlan's Record pool. */
Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan);

/* Free Record contents and return it to the Record pool. */
void ExecutionPlan_ReturnRecord(ExecutionPlan *plan, Record r);

/* Prints execution plan. */
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx);

/* Initialize all operations in an ExecutionPlan. */
void ExecutionPlan_Init(ExecutionPlan *plan);

/* Executes plan */
ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan);

/* Checks if execution plan been aborted, in case plan been aborted reason is set */
bool ExecutionPlan_Aborted(const ExecutionPlan *plan, ExecutionPlan_AbortReason *reason);

/* Abort execution, drains all operations */
void ExecutionPlan_Abort(OpBase *root, ExecutionPlan_AbortReason reason);

/* Profile executes plan */
ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan);

/* Increase execution plan reference count */
void ExecutionPlan_IncreaseRefCount(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlan_Free(ExecutionPlan *plan);

