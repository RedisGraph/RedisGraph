/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "execution_plan.h"
#include "../RG.h"
#include "./ops/ops.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "./optimizations/optimizer.h"
#include "../ast/ast_build_filter_tree.h"
#include "execution_plan_build/execution_plan_construct.h"
#include "execution_plan_build/execution_plan_modify.h"
#include "../path_patterns/path_pattern_ctx_construction.h"

#include <setjmp.h>

// Allocate a new ExecutionPlan segment.
inline ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void) {
	return rm_calloc(1, sizeof(ExecutionPlan));
}

void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Initialize the plan's record mapping if necessary.
	// It will already be set if this ExecutionPlan has been created to populate a single stream.
	if(plan->record_map == NULL) plan->record_map = raxNew();

	// Build and set path pattern context.
	PathPatternCtx *path_pattern_ctx = PathPatternCtx_Build(ast, Graph_RequiredMatrixDim(gc->g));
	QueryCtx_SetPathPatternCtx(path_pattern_ctx);
	plan->path_pattern_ctx = path_pattern_ctx;

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
	ASSERT(union_count > 1);

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
	const cypher_astnode_t *union_clause = AST_GetClause(ast, CYPHER_AST_UNION,
														 NULL);
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
		ASSERT(sub_plan->root->type == OPType_RESULTS);

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

static ExecutionPlan *_process_segment(AST *ast, uint segment_start_idx,
									   uint segment_end_idx) {
	ASSERT(ast != NULL);
	ASSERT(segment_start_idx <= segment_end_idx);

	ExecutionPlan *segment = NULL;

	// Construct a new ExecutionPlanSegment.
	segment = ExecutionPlan_NewEmptyExecutionPlan();
	segment->ast_segment = ast;
	ExecutionPlan_PopulateExecutionPlan(segment);

	return segment;
}

static ExecutionPlan **_process_segments(AST *ast) {
	uint nsegments = 0;               // number of segments
	uint seg_end_idx = 0;             // segment clause end index
	uint clause_count = 0;            // number of clauses
	uint seg_start_idx = 0;           // segment clause start index
	AST *ast_segment = NULL;          // segment AST
	uint *segment_indices = NULL;     // array segment bounds
	ExecutionPlan *segment = NULL;    // portion of the entire execution plan
	ExecutionPlan **segments = NULL;  // constructed segments

	clause_count = cypher_ast_query_nclauses(ast->root);

	//--------------------------------------------------------------------------
	// bound segments
	//--------------------------------------------------------------------------

	/* retrieve the indices of each WITH clause to properly set
	 * the segment's bounds.
	 * Every WITH clause demarcates the beginning of a new segment. */
	segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);

	// last segment
	segment_indices = array_append(segment_indices, clause_count);
	nsegments = array_len(segment_indices);
	segments = array_new(ExecutionPlan *, nsegments);

	//--------------------------------------------------------------------------
	// process segments
	//--------------------------------------------------------------------------

	seg_start_idx = 0;
	for(uint i = 0; i < nsegments; i++) {
		seg_end_idx = segment_indices[i];

		if((seg_end_idx - seg_start_idx) == 0) continue; // skip empty segment

		// slice the AST to only include the clauses in the current segment
		AST *ast_segment = AST_NewSegment(ast, seg_start_idx, seg_end_idx);

		// create ExecutionPlan segment that represents this slice of the AST
		segment = _process_segment(ast_segment, seg_start_idx, seg_end_idx);
		segments = array_append(segments, segment);

		// The next segment will start where the current one ended.
		seg_start_idx = seg_end_idx;
	}

	// Restore the overall AST.
	QueryCtx_SetAST(ast);
	array_free(segment_indices);

	return segments;
}

static ExecutionPlan *_tie_segments(ExecutionPlan **segments,
									uint segment_count) {
	FT_FilterNode *ft = NULL;            // filters following WITH
	OpBase *connecting_op = NULL;        // op connecting one segment to another
	OpBase *prev_connecting_op = NULL;   // root of previous segment
	ExecutionPlan *prev_segment = NULL;
	ExecutionPlan *current_segment = NULL;
	AST *master_ast = QueryCtx_GetAST(); // top-level AST of plan

	//--------------------------------------------------------------------------
	// merge segments
	//--------------------------------------------------------------------------

	for(int i = 0; i < segment_count; i++) {
		ExecutionPlan *segment = segments[i];
		AST *ast = segment->ast_segment;

		// find the firstmost non-argument operation in this segment
		prev_connecting_op = connecting_op;
		OpBase **taps = ExecutionPlan_LocateTaps(segment);
		ASSERT(array_len(taps) > 0);
		connecting_op = taps[0];
		array_free(taps);

		// tie the current segment's tap to the previous segment's root op
		if(prev_segment != NULL) {
			// Validate the connecting operation.
			// The connecting operation may already have children
			// if it's been attached to a previous scope.
			ASSERT(connecting_op->type == OPType_PROJECT ||
				   connecting_op->type == OPType_AGGREGATE);

			ExecutionPlan_AddOp(connecting_op, prev_segment->root);
		}

		prev_segment = segment;

		//----------------------------------------------------------------------
		// introduce projection filters
		//----------------------------------------------------------------------

		// Retrieve the current projection clause to build any necessary filters
		const cypher_astnode_t *opening_clause = cypher_ast_query_get_clause(ast->root, 0);
		cypher_astnode_type_t type = cypher_astnode_type(opening_clause);
		// Only WITH clauses introduce filters at this level;
		// all other scopes will be fully built at this point.
		if(type != CYPHER_AST_WITH) continue;

		// Build filters required by current segment.
		QueryCtx_SetAST(ast);
		ft = AST_BuildFilterTreeFromClauses(ast, &opening_clause, 1);
		if(ft == NULL) continue;

		// If any of the filtered variables operate on a WITH alias,
		// place the filter op above the projection.
		if(FilterTree_FiltersAlias(ft, opening_clause)) {
			OpBase *filter_op = NewFilterOp(current_segment, ft);
			ExecutionPlan_PushBelow(connecting_op, filter_op);
		} else {
			// None of the filtered variables are aliases;
			// filter ops may be placed anywhere in the scope.
			ExecutionPlan_PlaceFilterOps(segment, connecting_op, prev_connecting_op, ft);
		}
	}

	// Restore the master AST.
	QueryCtx_SetAST(master_ast);

	// The last ExecutionPlan segment is the master ExecutionPlan.
	ExecutionPlan *plan = segments[segment_count - 1];

	return plan;
}

// Add an implicit "Result" operation to ExecutionPlan if necessary.
static inline void _implicit_result(ExecutionPlan *plan) {
	// If the query culminates in a procedure call, it implicitly returns results.
	if(plan->root->type == OPType_PROC_CALL) {
		OpBase *results_op = NewResultsOp(plan);
		ExecutionPlan_UpdateRoot(plan, results_op);
	}
}

ExecutionPlan *NewExecutionPlan(void) {
	AST *ast = QueryCtx_GetAST();

	// handle UNION if there are any
	bool union_query = AST_ContainsClause(ast, CYPHER_AST_UNION);
	if(union_query) return _ExecutionPlan_UnionPlans(ast);

	// execution plans are created in 1 or more segments
	ExecutionPlan **segments = _process_segments(ast);
	ASSERT(segments != NULL);
	uint segment_count = array_len(segments);
	ASSERT(segment_count > 0);

	// connect all segments into a single ExecutionPlan
	ExecutionPlan *plan = _tie_segments(segments, segment_count);

	// the root operation is OpResults only if the query culminates in a RETURN
	// or CALL clause
	_implicit_result(plan);

	// clean up
	array_free(segments);

	return plan;
}

void ExecutionPlan_PreparePlan(ExecutionPlan *plan) {
	// Plan should be prepared only once.
	ASSERT(!plan->prepared);
	optimizePlan(plan);
	QueryCtx_SetLastWriter(_ExecutionPlan_FindLastWriter(plan->root));
	plan->prepared = true;
}

inline rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	ASSERT(plan && plan->record_map);
	return plan->record_map;
}

Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan) {
	rax *mapping = ExecutionPlan_GetMappings(plan);
	ASSERT(plan->record_pool);

	// Get a Record from the pool and set its owner and mapping.
	Record r = ObjectPool_NewItem(plan->record_pool);
	r->owner = plan;
	r->mapping = mapping;
	return r;
}

void ExecutionPlan_ReturnRecord(ExecutionPlan *plan, Record r) {
	ASSERT(plan && r);
	ObjectPool_DeleteItem(plan->record_pool, r);
}

//------------------------------------------------------------------------------
// Execution plan initialization
//------------------------------------------------------------------------------

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
	ASSERT(plan->prepared)
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

//------------------------------------------------------------------------------
// Execution plan draining
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Execution plan ref count
//------------------------------------------------------------------------------

void ExecutionPlan_IncreaseRefCount(ExecutionPlan *plan) {
	ASSERT(plan);
	__atomic_fetch_add(&plan->ref_count, 1, __ATOMIC_RELAXED);
}

int ExecutionPlan_DecRefCount(ExecutionPlan *plan) {
	ASSERT(plan);
	return __atomic_sub_fetch(&plan->ref_count, 1, __ATOMIC_RELAXED);
}

//------------------------------------------------------------------------------
// Execution plan profiling
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Execution plan free functions
//------------------------------------------------------------------------------

static void _ExecutionPlan_FreeInternals(ExecutionPlan *plan) {
	if(plan == NULL) return;

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
	}

	QueryGraph_Free(plan->query_graph);
	PathPatternCtx_Free(plan->path_pattern_ctx);
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

