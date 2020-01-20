/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "apply_join.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "rax.h"
#include "../ops/op_value_hash_join.h"
#include "../ops/op_cartesian_product.h"

#define NOT_RESOLVED -1

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

// Returns true if the stream resolves all required entities.
static bool _stream_resolves_entities(rax *stream_resolves, rax *entities_to_resolve) {
	bool resolved_all = true;
	raxIterator it;
	raxStart(&it, entities_to_resolve);

	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		if(raxFind(stream_resolves, it.key, it.key_len) == raxNotFound) {
			resolved_all = false;
			break;
		}
	}

	raxStop(&it);
	return resolved_all;
}

/* Given an expression node from a filter tree, returns the stream number
 * that fully resolves the expression's references. */
static int _relate_exp_to_stream(AR_ExpNode *exp, rax **stream_entities, int stream_count) {
	// Collect the referenced entities in the expression.
	rax *entities = raxNew();
	AR_EXP_CollectEntities(exp, entities);

	int stream_num;
	for(stream_num = 0; stream_num < stream_count; stream_num ++) {
		// See if the stream resolves all of the references.
		if(_stream_resolves_entities(stream_entities[stream_num], entities)) break;
	}
	raxFree(entities);

	if(stream_num == stream_count) return NOT_RESOLVED; // No single stream resolves all references.
	return stream_num;
}

// This function builds a Hash Join operation given its left and right branches and join criteria.
static OpBase *_build_hash_join_op(const ExecutionPlan *plan, OpBase *left_branch,
								   OpBase *right_branch, AR_ExpNode *lhs_join_exp, AR_ExpNode *rhs_join_exp) {
	OpBase *value_hash_join;
	// Detach the streams for the Value Hash Join from the execution plan.
	ExecutionPlan_DetachOp(right_branch);
	ExecutionPlan_DetachOp(left_branch);

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

/* This function will reorder a filter after a hash join operation has been built from a cartesian product,
 * and try to propogate the filter up to the first op that first shows the filter entities. */
static void _re_order_filter_op(ExecutionPlan *plan, OpBase *cp, OpFilter *filter) {
	FT_FilterNode *additional_filter_tree = filter->filterTree;

	rax *references = FilterTree_CollectModified(additional_filter_tree);
	OpBase *op = ExecutionPlan_LocateReferences(cp, NULL, references);
	ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
	ExecutionPlan_PushBelow(op, (OpBase *)filter);
	raxFree(references);

}

// Reduces a cartisian product to hash joins operations.
static void _reduce_cp_to_hashjoin(ExecutionPlan *plan, OpBase *cp) {
	// Retrieve all equality filter operations located upstream from the Cartesian Product.
	OpFilter **filter_ops = _locate_filters(cp);
	uint filter_count = array_len(filter_ops);

	while(filter_count > 0) {
		// For each stream joined by the Cartesian product, collect all entities the stream resolves.
		int stream_count = cp->childCount;
		rax *stream_entities[stream_count];
		for(int j = 0; j < stream_count; j ++) {
			stream_entities[j] = raxNew();
			ExecutionPlan_BoundVariables(cp->children[j], stream_entities[j]);
		}

		for(int j = 0; j < filter_count; j++) {
			// Reduce cartesian product to value hash join
			OpFilter *filter_op = filter_ops[j];

			/* Each filter being considered here tests for equality between its left and right values.
			 * The Cartesian Product can be replaced if both sides of the filter can be fully and
			 * separately resolved by exactly two child streams. */
			FT_FilterNode *f = filter_op->filterTree;

			/* Make sure LHS of the filter is resolved by a stream. */
			AR_ExpNode *lhs = f->pred.lhs;
			uint lhs_resolving_stream = _relate_exp_to_stream(lhs, stream_entities, stream_count);
			if(lhs_resolving_stream == NOT_RESOLVED) continue;

			/* Make sure RHS of the filter is resolved by a stream. */
			AR_ExpNode *rhs = f->pred.rhs;
			uint rhs_resolving_stream = _relate_exp_to_stream(rhs, stream_entities, stream_count);
			if(rhs_resolving_stream == NOT_RESOLVED) continue;

			assert(lhs_resolving_stream != rhs_resolving_stream);

			// Clone the filter expressions.
			lhs = AR_EXP_Clone(lhs);
			rhs = AR_EXP_Clone(rhs);
			// Retrieve the relevent branch roots.
			OpBase *right_branch = cp->children[rhs_resolving_stream];
			OpBase *left_branch = cp->children[lhs_resolving_stream];
			// Build hash join op.
			OpBase *value_hash_join = _build_hash_join_op
									  (cp->plan, left_branch, right_branch, lhs, rhs);

			// The filter will now be resolved by the join operation; remove it.
			ExecutionPlan_RemoveOp(plan, (OpBase *)filter_op);
			OpBase_Free((OpBase *)filter_op);

			if(cp->childCount == 0) {
				// The entire Cartesian Product can be replaced with the join op.
				ExecutionPlan_ReplaceOp(plan, cp, value_hash_join);
				OpBase_Free(cp);
			} else {
				// The Cartesian Product still has a child operation; introduce the join op as another child.
				ExecutionPlan_AddOp(cp, value_hash_join);
				// Search for additional filters which can be solved by the join operation branches.
				for(int k = j + 1; k < filter_count; k++) {
					/* Fix for issue #869 https://github.com/RedisGraph/RedisGraph/issues/869.
					 * When trying to replace a cartesian product which followed by multiple filters that can be resolved by the same two branches
					 * the application crashed on assert(lhs_resolving_stream != rhs_resolving_stream) since before the fix only one filter was used
					 * to create the hash join, and the other were ignored. */
					// Check for additional filters which checking for the same equality.
					OpFilter *additional_filter = filter_ops[k];
					_re_order_filter_op(plan, cp, additional_filter);
				}
			}
			break; // The operations have been updated, don't evaluate more filters.
		}
		for(int j = 0; j < stream_count; j ++) raxFree(stream_entities[j]);
		// We have modifed the filters array, we need to re-collect the remaining equality filters.
		array_free(filter_ops);
		filter_ops = _locate_filters(cp);
		filter_count = array_len(filter_ops);
	}
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
