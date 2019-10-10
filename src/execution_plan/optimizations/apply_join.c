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

// Tests to see if given filter can act as a join condition.
static inline bool _applicableFilter(const FT_FilterNode *f) {
	// Can only convert filters that test equality
	bool equality_check = (f->t == FT_N_PRED && f->pred.op == OP_EQUAL);
	// TODO allowing AR_ExpNodes that refer directly to graph entities currently causes memory errors.
	// This restriction should be lifted later.
	bool comparing_graph_entities = (f->pred.lhs->type == AR_EXP_OPERAND &&
									 f->pred.lhs->operand.type == AR_EXP_VARIADIC &&
									 f->pred.lhs->operand.variadic.entity_prop == NULL);
	return equality_check && !comparing_graph_entities;
}

// Collects all consecutive filters beneath given op.
static OpFilter **_locate_filters(OpBase *cp) {
	OpBase *parent = cp->parent;
	OpFilter **filters = array_new(OpFilter *, 0);

	while(parent && parent->type == OPType_FILTER) {
		filters = array_append(filters, (OpFilter *)parent);
		parent = parent->parent;
	}

	return filters;
}

// Tests if stream resolves all entities.
static bool _stream_resolves_entities(const OpBase *root, rax *entities) {
	if(root->modifies) {
		uint modifies_count = array_len(root->modifies);
		for(uint i = 0; i < modifies_count; i++) {
			const char *modified = root->modifies[i];
			raxRemove(entities, (unsigned char *)modified, strlen(modified), NULL);
		}
	}

	if(raxSize(entities) == 0) return true;

	for(int i = 0; i < root->childCount; i++) {
		if(_stream_resolves_entities(root->children[i], entities)) {
			return true;
		}
	}

	return false;
}

/* filter is composed of two expressions:
 * left and right hand side
 * to apply a join operation each expression
 * must be entirely resolved by one of the branches
 * either left or right of the cartesian product operation
 * _relate_exp_to_stream will try to associate each expression
 * lhs and rhs with the appropriate branch. */
static void _relate_exp_to_stream(const OpBase *cp, const FT_FilterNode *f, AR_ExpNode **lhs,
								  AR_ExpNode **rhs) {
	*lhs = NULL;
	*rhs = NULL;
	bool expression_resolved = true;

	AR_ExpNode *lhs_exp = f->pred.lhs;
	AR_ExpNode *rhs_exp = f->pred.rhs;

	assert(cp->childCount == 2);
	OpBase *left_child = cp->children[0];
	OpBase *right_child = cp->children[1];

	rax *entities = raxNew();

	/* Make sure LHS and RHS expressions can be resolved.
	 * Left expression - Left branch
	 * Right expression - Right branch */
	AR_EXP_CollectEntities(lhs_exp, entities);
	if(_stream_resolves_entities(left_child, entities)) {
		// entities is now empty.
		AR_EXP_CollectEntities(rhs_exp, entities);
		if(_stream_resolves_entities(right_child, entities)) {
			*lhs = lhs_exp;
			*rhs = rhs_exp;
			raxFree(entities);
			return;
		}
	}

	/* Left expression - Right branch
	 * Right expression - Left branch */
	*lhs = NULL;
	*rhs = NULL;

	raxFree(entities);
	entities = raxNew();

	AR_EXP_CollectEntities(lhs_exp, entities);
	if(_stream_resolves_entities(right_child, entities)) {
		// entities is now empty.
		AR_EXP_CollectEntities(rhs_exp, entities);
		if(_stream_resolves_entities(left_child, entities)) {
			*lhs = rhs_exp;
			*rhs = lhs_exp;
			raxFree(entities);
			return;
		}
	}

	// Couldn't completely resolve either lhs_exp or rhs_exp.
	*lhs = NULL;
	*rhs = NULL;
	raxFree(entities);
}

/* Try to replace cartesian product with a value hash join operation */
void applyJoin(ExecutionPlan *plan) {
	OpBase **cps = ExecutionPlan_LocateOps(plan->root, OPType_CARTESIAN_PRODUCT);
	int cp_count = array_len(cps);

	for(int i = 0; i < cp_count; i++) {
		OpBase *cp = cps[i];
		/* TODO: change the way our cartesian product
		 * today we alow for cartesian product with > 2 child ops
		 * I think it might be better to create a chain of cartesian products
		 * where each pulls from exactly 2 streams
		 * consider: MATCH a,b,c WHERE a.v = b.v. */
		if(cp->childCount != 2) continue;
		OpFilter **filters = _locate_filters(cp);

		int filter_count = array_len(filters);
		for(int j = 0; j < filter_count; j++) {
			OpFilter *filter = filters[j];
			if(_applicableFilter(filter->filterTree)) {
				// Reduce cartesian product to value hash join
				AR_ExpNode *lhs = NULL;
				AR_ExpNode *rhs = NULL;

				/* Make sure lhs expression is resolved by
				 * left stream, if not swap. */
				_relate_exp_to_stream(cp, filter->filterTree, &lhs, &rhs);
				/* There are cases where either lhs or rhs expressions
				 * require data from both streams, consider:
				 * a.v + c.v = b.v + d.v
				 * where a and d are resolved by lhs stream and
				 * b and c are resolved by rhs stream.
				 * in which case we can't perform join. */
				if(lhs == NULL || rhs == NULL) continue;

				assert(lhs != rhs);
				lhs = AR_EXP_Clone(lhs);
				rhs = AR_EXP_Clone(rhs);

				/* In order to reduce value-hash-join cache size
				 * prefer to cache a branch which will produce the smallest number
				 * of records, currently prefer a branch with a filter. */
				OpBase *left_branch = cp->children[0];
				OpBase *right_branch = cp->children[1];
				bool left_branch_filtered = (ExecutionPlan_LocateOp(left_branch, OPType_FILTER) != NULL);
				bool right_branch_filtered = (ExecutionPlan_LocateOp(right_branch, OPType_FILTER) != NULL);
				if(!left_branch_filtered && right_branch_filtered) {
					// Swap branches!
					cp->children[0] = right_branch;
					cp->children[1] = left_branch;

					AR_ExpNode *t = lhs;
					lhs = rhs;
					rhs = t;
				}

				OpBase *value_hash_join = NewValueHashJoin(cp->plan, lhs, rhs);

				/* Remove filter which is now part of the join operation
				 * replace cartesian product with join. */
				ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
				OpBase_Free((OpBase *)filter);
				ExecutionPlan_ReplaceOp(plan, cp, value_hash_join);
				OpBase_Free(cp);

				break;
			}
		}
		array_free(filters);
	}
	array_free(cps);
}

