/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "apply_join.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "rax.h"
#include "../ops/op_value_hash_join.h"
#include "../ops/op_cartesian_product.h"
#include "optimizations_util.h"

// Tests to see if given filter can act as a join condition.
static inline bool _applicableFilter(const FT_FilterNode *f) {
	// Can only convert filters that test equality
	if(f->t != FT_N_PRED || f->pred.op != OP_EQUAL) return false;
	// TODO allowing AR_ExpNodes that refer directly to graph entities currently causes memory errors.
	// This restriction should be lifted later.
	if((f->pred.lhs->type == AR_EXP_OPERAND &&
		f->pred.lhs->operand.type == AR_EXP_VARIADIC &&
		f->pred.lhs->operand.variadic.entity_prop == NULL) ||
	   (f->pred.rhs->type == AR_EXP_OPERAND &&
		f->pred.rhs->operand.type == AR_EXP_VARIADIC &&
		f->pred.rhs->operand.variadic.entity_prop == NULL)) return false;
	return true;
}

// Collects all consecutive filters beneath given op.
static OpFilter **_locate_filters(OpBase *cp) {
	OpBase *parent = cp->parent;
	OpFilter **filters = array_new(OpFilter *, 0);

	while(parent && parent->type == OPType_FILTER) {
		OpFilter *filter_op = (OpFilter *)parent;
		if(_applicableFilter(filter_op->filterTree)) filters = array_append(filters, filter_op);
		parent = parent->parent;
	}

	return filters;
}

// This function builds a Hash Join operation given its left and right branches and join criteria.
static OpBase *_build_hash_join_op(const ExecutionPlan *plan, OpBase *left_branch,
								   OpBase *right_branch, AR_ExpNode *lhs_join_exp, AR_ExpNode *rhs_join_exp) {
	OpBase *value_hash_join;

	/* The Value Hash Join will cache its left-hand stream. To reduce the cache size,
	 * prefer to cache the stream which will produce the smallest number of records.
	 * Our current heuristic for this is to prefer a stream which contains a filter operation. */
	bool left_branch_filtered = (ExecutionPlan_LocateFirstOp(left_branch, OPType_FILTER) != NULL);
	bool right_branch_filtered = (ExecutionPlan_LocateFirstOp(right_branch, OPType_FILTER) != NULL);
	if(!left_branch_filtered && right_branch_filtered) {
		// Only the RHS stream is filtered, swap the input streams and expressions.
		value_hash_join = NewValueHashJoin(plan, rhs_join_exp, lhs_join_exp);
		OpBase *t = left_branch;
		left_branch = right_branch;
		right_branch = t;
	} else {
		value_hash_join = NewValueHashJoin(plan, lhs_join_exp, rhs_join_exp);
	}

	// Add the detached streams to the join op.
	ExecutionPlan_AddOp(value_hash_join, left_branch);
	ExecutionPlan_AddOp(value_hash_join, right_branch);

	return value_hash_join;
}

// Reduces a cartisian product to hash joins operations.
static void _reduce_cp_to_hashjoin(ExecutionPlan *plan, OpBase *cp) {
	// Retrieve all equality filter operations located upstream from the Cartesian Product.
	OpFilter **filter_ops = _locate_filters(cp);
	uint filter_count = array_len(filter_ops);

	// For each stream joined by the Cartesian product, collect all entities the stream resolves.
	int stream_count = cp->childCount;
	rax *stream_entities[stream_count];
	OptimizeUtils_BuildStreamFromOp(cp, stream_entities, stream_count);
	for(uint i = 0; i < filter_count; i ++) {
		// Try reduce the cartesian product to value hash join with the current filter.
		OpFilter *filter_op = filter_ops[i];

		/* Each filter being considered here tests for equality between its left and right values.
		 * The Cartesian Product can be replaced if both sides of the filter can be fully and
		 * separately resolved by exactly two child streams. */
		FT_FilterNode *f = filter_op->filterTree;

		/* Make sure LHS of the filter is resolved by a stream. */
		AR_ExpNode *lhs = f->pred.lhs;
		uint lhs_resolving_stream = OptimizeUtils_RelateExpToStream(lhs, stream_entities, stream_count);
		if(lhs_resolving_stream == NOT_RESOLVED) continue;
		/* Make sure RHS of the filter is resolved by a stream. */
		AR_ExpNode *rhs = f->pred.rhs;
		uint rhs_resolving_stream = OptimizeUtils_RelateExpToStream(rhs, stream_entities, stream_count);
		if(rhs_resolving_stream == NOT_RESOLVED) continue;

		/* Fix for issue #869 https://github.com/RedisGraph/RedisGraph/issues/869.
		* When trying to replace a cartesian product which followed by multiple filters that can be resolved by the same two branches
		* the application crashed on assert(lhs_resolving_stream != rhs_resolving_stream) since before the fix only one filter was used
		* to create the hash join, and the other were ignored. */
		// This stream is solved by a single cartesian product child and needs to be propogate up.
		if(lhs_resolving_stream == rhs_resolving_stream) {
			OptimizeUtils_MigrateFilterOp(plan, cp->children[rhs_resolving_stream], filter_op);
			continue;
		}

		// Clone the filter expressions.
		lhs = AR_EXP_Clone(lhs);
		rhs = AR_EXP_Clone(rhs);

		// Retrieve the relevent branch roots.
		OpBase *right_branch = cp->children[rhs_resolving_stream];
		OpBase *left_branch = cp->children[lhs_resolving_stream];
		// Detach the streams for the Value Hash Join from the execution plan.
		ExecutionPlan_DetachOp(right_branch);
		ExecutionPlan_DetachOp(left_branch);
		// Build hash join op.
		OpBase *value_hash_join = _build_hash_join_op
								  (cp->plan, left_branch, right_branch, lhs, rhs);

		// The filter will now be resolved by the join operation; remove it.
		ExecutionPlan_RemoveOp(plan, (OpBase *)filter_op);
		OpBase_Free((OpBase *)filter_op);

		// Place hash join op.
		if(cp->childCount == 0) {
			// The entire Cartesian Product can be replaced with the join op.
			ExecutionPlan_ReplaceOp(plan, cp, value_hash_join);
			OpBase_Free(cp);
			// Try to popagate up the remaining filters.
			i++;
			for(; i < filter_count; i++) {
				OptimizeUtils_MigrateFilterOp(plan, value_hash_join, filter_ops[i]);
			}
		} else {
			// The Cartesian Product still has a child operation; introduce the join op as another child.
			ExecutionPlan_AddOp(cp, value_hash_join);
			// Streams are no longer valid since cartesian product changed.
			for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
			// Re-collect cartesian product streams.
			stream_count = cp->childCount;
			OptimizeUtils_BuildStreamFromOp(cp, stream_entities, stream_count);
		}
	}
	// Clean up.
	for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
	array_free(filter_ops);
}

// TODO: Consider changing Cartesian Products such that each has exactly two child operations.
/* Try to replace Cartesian Products (cross joins) with Value Hash Joins.
 * This is viable when a Cartesian Product is combining two streams that each satisfies
 * one side of an EQUALS filter operation, like:
 * MATCH (a), (b) WHERE ID(a) = ID(b) */
void applyJoin(ExecutionPlan *plan) {
	OpBase **cps = ExecutionPlan_LocateOps(plan->root, OPType_CARTESIAN_PRODUCT);
	uint cp_count = array_len(cps);

	for(uint i = 0; i < cp_count; i++) {
		OpBase *cp = cps[i];
		_reduce_cp_to_hashjoin(plan, cp);
	}
	array_free(cps);
}
