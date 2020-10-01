/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "execution_plan.h"
#include "../RG.h"
#include "./ops/ops.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "./optimizations/optimizer.h"
#include "../ast/ast_build_filter_tree.h"
#include "execution_plan_build/execution_plan_construct.h"
#include "execution_plan_build/execution_plan_modify.h"

#include <assert.h>
#include <setjmp.h>

// Allocate a new ExecutionPlan segment.
inline ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void) {
	ExecutionPlan *plan = rm_calloc(1, sizeof(ExecutionPlan));
	return plan;
}

void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Initialize the plan's record mapping if necessary.
	// It will already be set if this ExecutionPlan has been created to populate a single stream.
	if(plan->record_map == NULL) plan->record_map = raxNew();

	// Build query graph
	// Query graph is set if this ExecutionPlan has been created to populate a single stream.
	if(plan->query_graph == NULL) plan->query_graph = BuildQueryGraph(ast);

	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		// Build the appropriate operation(s) for each clause in the query.
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		ExecutionPlanSegment_ConvertClause(gc, ast, plan, clause);
	}
}

static ExecutionPlan *_ExecutionPlan_UnionPlans(AST *ast) {
	uint end_offset = 0;
	uint start_offset = 0;
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	uint *union_indices = AST_GetClauseIndices(ast, CYPHER_AST_UNION);
	union_indices = array_append(union_indices, clause_count);
	int union_count = array_len(union_indices);
	assert(union_count > 1);

	/* Placeholder for each execution plan, these all will be joined
	 * via a single UNION operation. */
	ExecutionPlan *plans[union_count];

	for(int i = 0; i < union_count; i++) {
		// Create an AST segment from which we will build an execution plan.
		end_offset = union_indices[i];
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);
		plans[i] = NewExecutionPlan();
		AST_Free(ast_segment); // Free the AST segment.

		// Next segment starts where this one ends.
		start_offset = union_indices[i] + 1;
	}

	QueryCtx_SetAST(ast); // AST segments have been freed, set master AST in QueryCtx.

	array_free(union_indices);

	/* Join streams:
	 * MATCH (a) RETURN a UNION MATCH (a) RETURN a ....
	 * left stream:     [Scan]->[Project]->[Results]
	 * right stream:    [Scan]->[Project]->[Results]
	 *
	 * Joined:
	 * left stream:     [Scan]->[Project]
	 * right stream:    [Scan]->[Project]
	 *                  [Union]->[Distinct]->[Result] */
	ExecutionPlan *plan = ExecutionPlan_NewEmptyExecutionPlan();
	plan->record_map = raxNew();

	OpBase *results_op = NewResultsOp(plan);
	OpBase *parent = results_op;
	ExecutionPlan_UpdateRoot(plan, results_op);

	// Introduce distinct only if `ALL` isn't specified.
	const cypher_astnode_t *union_clause = AST_GetClause(ast, CYPHER_AST_UNION);
	if(!cypher_ast_union_has_all(union_clause)) {
		OpBase *distinct_op = NewDistinctOp(plan);
		ExecutionPlan_AddOp(results_op, distinct_op);
		parent = distinct_op;
	}

	OpBase *join_op = NewJoinOp(plan);
	ExecutionPlan_AddOp(parent, join_op);

	// Join execution plans.
	for(int i = 0; i < union_count; i++) {
		ExecutionPlan *sub_plan = plans[i];
		assert(sub_plan->root->type == OPType_RESULTS);

		// Remove OP_Result.
		OpBase *op_result = sub_plan->root;
		ExecutionPlan_RemoveOp(sub_plan, sub_plan->root);
		OpBase_Free(op_result);

		ExecutionPlan_AddOp(join_op, sub_plan->root);
	}

	return plan;
}

static OpBase *_ExecutionPlan_FindLastWriter(OpBase *root) {
	if(OpBase_IsWriter(root)) return root;
	for(int i = root->childCount - 1; i >= 0; i--) {
		OpBase *child = root->children[i];
		OpBase *res = _ExecutionPlan_FindLastWriter(child);
		if(res) return res;
	}
	return NULL;
}

ExecutionPlan *NewExecutionPlan(void) {
	AST *ast = QueryCtx_GetAST();
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	/* Handle UNION if there are any. */
	if(AST_ContainsClause(ast, CYPHER_AST_UNION)) return _ExecutionPlan_UnionPlans(ast);

	uint start_offset = 0;
	uint end_offset = 0;

	/* Execution plans are created in 1 or more segments. Every WITH clause demarcates
	 * the beginning of a new segment, and a RETURN clause (if present) forms its own segment. */
	const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
	cypher_astnode_type_t last_clause_type = cypher_astnode_type(last_clause);
	bool query_has_return = (last_clause_type == CYPHER_AST_RETURN);

	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	uint with_clause_count = array_len(segment_indices);

	bool with_is_first_clause = false;
	// If the first clause of the query is WITH, remove its index from the segment list.
	if(array_len(segment_indices) > 0 && segment_indices[0] == 0) {
		segment_indices = array_del(segment_indices, 0);
		with_is_first_clause = true;
	}

	/* The RETURN clause is converted into an independent final segment.
	 * If the query is exclusively composed of a RETURN clause, only one segment is constructed
	 * so this step is skipped. */
	if(query_has_return && clause_count > 1) {
		segment_indices = array_append(segment_indices, clause_count - 1);
	}

	// Add the clause count as a final value so that the last segment will read to the end of the query.
	segment_indices = array_append(segment_indices, clause_count);

	uint segment_count = array_len(segment_indices);
	ExecutionPlan *segments[segment_count];
	AST *ast_segments[segment_count];
	start_offset = 0;
	for(int i = 0; i < segment_count; i++) {
		uint end_offset = segment_indices[i];
		// Slice the AST to only include the clauses in the current segment.
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);
		ast_segments[i] = ast_segment;
		// Construct a new ExecutionPlanSegment.
		ExecutionPlan *segment = ExecutionPlan_NewEmptyExecutionPlan();
		ExecutionPlan_PopulateExecutionPlan(segment);
		segment->ast_segment = ast_segment;
		segments[i] = segment;
		start_offset = end_offset;
	}

	// The first segment only requires filter ops at this point if the first clause is WITH.
	if(with_is_first_clause) {
		const cypher_astnode_t *with_clause = cypher_ast_query_get_clause(ast->root, 0);
		FT_FilterNode *ft = AST_BuildFilterTreeFromClauses(ast_segments[0], &with_clause, 1);
		if(ft) ExecutionPlan_PlaceFilterOps(segments[0], segments[0]->root, NULL, ft);
	}

	OpBase *connecting_op = NULL;
	OpBase *prev_scope_end = NULL;
	// Merge segments.
	for(int i = 1; i < segment_count; i++) {
		ExecutionPlan *prev_segment = segments[i - 1];
		ExecutionPlan *current_segment = segments[i];

		OpBase *prev_root = prev_segment->root;
		connecting_op = ExecutionPlan_LocateOpMatchingType(current_segment->root, PROJECT_OPS,
														   PROJECT_OP_COUNT);
		assert(connecting_op->childCount == 0);

		ExecutionPlan_AddOp(connecting_op, prev_root);

		// The final segment cannot culminate in a WITH clause, so has no additional filters to process.
		if(i == segment_count - 1) continue;

		// Retrieve the current projection clause to build any necessary Filter ops.
		const cypher_astnode_t *with_clause = cypher_ast_query_get_clause(ast->root,
																		  segment_indices[i - 1]);
		// Build filters required by current segment.
		FT_FilterNode *ft = AST_BuildFilterTreeFromClauses(ast_segments[i], &with_clause, 1);
		if(ft) {
			// If any of the filtered variables operate on a WITH alias, place the filter op above the projection.
			if(FilterTree_FiltersAlias(ft, with_clause)) {
				OpBase *filter_op = NewFilterOp(current_segment, ft);
				ExecutionPlan_UpdateRoot(current_segment, filter_op);
			} else {
				// None of the filtered variables are aliases; filter ops may be placed anywhere in the scope.
				ExecutionPlan_PlaceFilterOps(current_segment, current_segment->root, prev_scope_end, ft);
			}
		}

		prev_scope_end = prev_root; // Track the previous scope's end so filter placement doesn't overreach.
	}

	array_free(segment_indices);

	ExecutionPlan *plan = segments[segment_count - 1];
	// The root operation is OpResults only if the query culminates in a RETURN or CALL clause.
	if(query_has_return || last_clause_type == CYPHER_AST_CALL) {
		OpBase *results_op = NewResultsOp(plan);
		ExecutionPlan_UpdateRoot(plan, results_op);
	}

	return plan;
}

// Sets an AST segment in the execution plan.
inline void ExecutionPlan_SetAST(ExecutionPlan *plan, AST *ast) {
	plan->ast_segment = ast;
}

// Gets the AST segment from the execution plan.
inline AST *ExecutionPlan_GetAST(const ExecutionPlan *plan) {
	return plan->ast_segment;
}

void ExecutionPlan_PreparePlan(ExecutionPlan *plan) {
	// Plan should be prepared only once.
	assert(!plan->prepared);
	optimizePlan(plan);
	QueryCtx_SetLastWriter(_ExecutionPlan_FindLastWriter(plan->root));
	plan->prepared = true;
}

inline rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	assert(plan && plan->record_map);
	return plan->record_map;
}

Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan) {
	rax *mapping = ExecutionPlan_GetMappings(plan);
	assert(plan->record_pool);

	// Get a Record from the pool and set its owner and mapping.
	Record r = ObjectPool_NewItem(plan->record_pool);
	r->owner = plan;
	r->mapping = plan->record_map;
	return r;
}

void ExecutionPlan_ReturnRecord(ExecutionPlan *plan, Record r) {
	assert(plan && r);
	ObjectPool_DeleteItem(plan->record_pool, r);
}

void _ExecutionPlan_Print(const OpBase *op, RedisModuleCtx *ctx, char *buffer, int buffer_len,
						  int ident, int *op_count) {
	if(!op) return;

	*op_count += 1; // account for current operation.

	// Construct operation string representation.
	int bytes_written = snprintf(buffer, buffer_len, "%*s", ident, "");
	bytes_written += OpBase_ToString(op, buffer + bytes_written, buffer_len - bytes_written);

	RedisModule_ReplyWithStringBuffer(ctx, buffer, bytes_written);

	// Recurse over child operations.
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_Print(op->children[i], ctx, buffer, buffer_len, ident + 4, op_count);
	}
}

// Reply with a string representation of given execution plan.
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx) {
	assert(plan && ctx);

	int op_count = 0;   // Number of operations printed.
	char buffer[1024];

	// No idea how many operation are in execution plan.
	RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
	_ExecutionPlan_Print(plan->root, ctx, buffer, 1024, 0, &op_count);

	RedisModule_ReplySetArrayLength(ctx, op_count);
}

static inline void _ExecutionPlan_InitRecordPool(ExecutionPlan *plan) {
	if(plan->record_pool) return;
	/* Initialize record pool.
	 * Determine Record size to inform ObjectPool allocation. */
	uint entries_count = raxSize(plan->record_map);
	uint rec_size = sizeof(_Record) + (sizeof(Entry) * entries_count);

	// Create a data block with initial capacity of 256 records.
	plan->record_pool = ObjectPool_New(256, rec_size, (fpDestructor)Record_FreeEntries);
}

static void _ExecutionPlanInit(OpBase *root) {
	// If the ExecutionPlan associated with this op hasn't built a record pool yet, do so now.
	_ExecutionPlan_InitRecordPool((ExecutionPlan *)root->plan);

	// Initialize the operation if necessary.
	if(root->init) root->init(root);

	// Continue initializing downstream operations.
	for(int i = 0; i < root->childCount; i++) {
		_ExecutionPlanInit(root->children[i]);
	}
}

void ExecutionPlan_Init(ExecutionPlan *plan) {
	_ExecutionPlanInit(plan->root);
}

ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan) {
	assert(plan->prepared);
	/* Set an exception-handling breakpoint to capture run-time errors.
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	int encountered_error = SET_EXCEPTION_HANDLER();

	// Encountered a run-time error - return immediately.
	if(encountered_error) return QueryCtx_GetResultSet();

	ExecutionPlan_Init(plan);

	Record r = NULL;
	// Execute the root operation and free the processed Record until the data stream is depleted.
	while((r = OpBase_Consume(plan->root)) != NULL) ExecutionPlan_ReturnRecord(r->owner, r);

	return QueryCtx_GetResultSet();
}

// NOP operation consume routine for immediately terminating execution.
static Record deplete_consume(struct OpBase *op) {
	return NULL;
}

// return true if execution plan been drained
// false otherwise
bool ExecutionPlan_Drained(ExecutionPlan *plan) {
	ASSERT(plan != NULL);
	ASSERT(plan->root != NULL);
	return (plan->root->consume == deplete_consume);
}

static void _ExecutionPlan_Drain(OpBase *root) {
	root->consume = deplete_consume;
	for(int i = 0; i < root->childCount; i++) {
		_ExecutionPlan_Drain(root->children[i]);
	}
}

// Resets each operation consume function to simply return NULL
// this will cause the execution-plan to quickly deplete
void ExecutionPlan_Drain(ExecutionPlan *plan) {
	ASSERT(plan && plan->root);
	_ExecutionPlan_Drain(plan->root);
}

static void _ExecutionPlan_InitProfiling(OpBase *root) {
	root->profile = root->consume;
	root->consume = OpBase_Profile;
	root->stats = rm_malloc(sizeof(OpStats));
	root->stats->profileExecTime = 0;
	root->stats->profileRecordCount = 0;

	if(root->childCount) {
		for(int i = 0; i < root->childCount; i++) {
			OpBase *child = root->children[i];
			_ExecutionPlan_InitProfiling(child);
		}
	}
}

static void _ExecutionPlan_FinalizeProfiling(OpBase *root) {
	if(root->childCount) {
		for(int i = 0; i < root->childCount; i++) {
			OpBase *child = root->children[i];
			root->stats->profileExecTime -= child->stats->profileExecTime;
			_ExecutionPlan_FinalizeProfiling(child);
		}
	}
	root->stats->profileExecTime *= 1000;   // Milliseconds.
}

ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan) {
	_ExecutionPlan_InitProfiling(plan->root);
	ResultSet *rs = ExecutionPlan_Execute(plan);
	_ExecutionPlan_FinalizeProfiling(plan->root);
	return rs;
}

void ExecutionPlan_IncreaseRefCount(ExecutionPlan *plan) {
	ASSERT(plan);
	__atomic_fetch_add(&plan->ref_count, 1, __ATOMIC_RELAXED);
}

int ExecutionPlan_DecRefCount(ExecutionPlan *plan) {
	ASSERT(plan);
	return __atomic_sub_fetch(&plan->ref_count, 1, __ATOMIC_RELAXED);
}

static void _ExecutionPlan_FreeInternals(ExecutionPlan *plan) {
	if(plan == NULL) return;

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
	}

	QueryGraph_Free(plan->query_graph);
	if(plan->record_map) raxFree(plan->record_map);
	if(plan->record_pool) ObjectPool_Free(plan->record_pool);
	if(plan->ast_segment) AST_Free(plan->ast_segment);
	rm_free(plan);
}

// Free an op tree and its associated ExecutionPlan segments.
static ExecutionPlan *_ExecutionPlan_FreeOpTree(OpBase *op) {
	if(op == NULL) return NULL;
	ExecutionPlan *child_plan = NULL;
	ExecutionPlan *prev_child_plan = NULL;
	// Store a reference to the current plan.
	ExecutionPlan *current_plan = (ExecutionPlan *)op->plan;
	for(uint i = 0; i < op->childCount; i ++) {
		child_plan = _ExecutionPlan_FreeOpTree(op->children[i]);
		// In most cases all children will share the same plan, but if they don't
		// (for an operation like UNION) then free the now-obsolete previous child plan.
		if(prev_child_plan != child_plan) {
			_ExecutionPlan_FreeInternals(prev_child_plan);
			prev_child_plan = child_plan;
		}
	}

	// Free this op.
	OpBase_Free(op);

	// Free each ExecutionPlan segment once all ops associated with it have been freed.
	if(current_plan != child_plan) _ExecutionPlan_FreeInternals(child_plan);

	return current_plan;
}
void ExecutionPlan_Free(ExecutionPlan *plan) {
	if(plan == NULL) return;
	if(ExecutionPlan_DecRefCount(plan) >= 0) return;

	// Free all ops and ExecutionPlan segments.
	_ExecutionPlan_FreeOpTree(plan->root);

	// Free the final ExecutionPlan segment.
	_ExecutionPlan_FreeInternals(plan);
}

