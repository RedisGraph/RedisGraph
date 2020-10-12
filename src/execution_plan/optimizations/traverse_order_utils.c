/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../../RG.h"
#include "../../util/strcmp.h"
#include "traverse_order_utils.h"

int QGNode_LabelsScore(const QGNode *node) {
	// TODO: support multiple labels.
	return node->label ? 1 : 0;
}

int QGEntity_FilterExistenceScore(const char *entity, rax *filtered_entities) {
	return raxFind(filtered_entities, (unsigned char *)entity, strlen(entity)) != raxNotFound ? 1 : 0;
}

int QGEntity_IndependentFilterScore(const char *entity, rax *independent_entities) {
	void *find_result = raxFind(independent_entities, (unsigned char *)entity, strlen(entity));
	// Avoid compiler warnings.
	int res = (int64_t)find_result;
	return find_result != raxNotFound ? res : 0;
}

int QGEntity_BoundVariableScore(const char *entity, rax *bound_vars) {
	return raxFind(bound_vars, (unsigned char *)entity, strlen(entity)) != raxNotFound ? 1 : 0;
}

int AlgebraicExpression_LabelsScore(AlgebraicExpression *exp, const QueryGraph *qg) {
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	QGNode *src_node = QueryGraph_GetNodeByAlias(qg, src);
	QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, dest);
	// TODO: Consider cumulative scoring for multiple labels support. (:A) should be scored less than (:A:B) as multiple labels reduces
	score += QGNode_LabelsScore(src_node);
	score += QGNode_LabelsScore(dest_node);
	return score;
}

int AlgebraicExpression_FilterExistenceScore(AlgebraicExpression *exp,
											 rax *filtered_entities) {
	if(!filtered_entities) return 0;
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += QGEntity_FilterExistenceScore(src, filtered_entities);
	score += QGEntity_FilterExistenceScore(dest, filtered_entities);
	return score;
}

int AlgebraicExpression_IndependentFilterScore(AlgebraicExpression *exp,
											   rax *independent_entities) {
	if(!independent_entities) return 0;
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += QGEntity_IndependentFilterScore(src, independent_entities);
	score += QGEntity_IndependentFilterScore(dest, independent_entities);
	return score;
}

int AlgebraicExpression_BoundVariableScore(AlgebraicExpression *exp, rax *bound_vars) {
	if(!bound_vars) return 0;
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += QGEntity_BoundVariableScore(src, bound_vars);
	score += QGEntity_BoundVariableScore(dest, bound_vars);
	return score;
}

// move to unit test
bool valid_position(AlgebraicExpression **exps, int pos, AlgebraicExpression *exp, QueryGraph *qg) {
	if(pos == 0) {
		// The first operand cannot be a variable-length edge with a labeled source or destination,
		// as a labeled endpoint must be first to get replaced by a LabelScan op.
		QGNode *src_node = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Source(exp));
		QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Destination(exp));
		if((src_node->label || dest_node->label) &&
		   AlgebraicExpression_Edge(exp) &&
		   AlgebraicExpression_OperandCount(exp) == 1) return false;

		// Since the first operand has no previous variables to check, it is assumed to be valid.
		return true;
	}

	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	// Iterate over all previous expressions to make sure at least one of this expression's endpoints are bound.
	for(int i = pos - 1; i >= 0; i --) {
		AlgebraicExpression *prev = exps[i];
		const char *prev_src = AlgebraicExpression_Source(prev);
		const char *prev_dest = AlgebraicExpression_Destination(prev);
		if(!RG_STRCMP(prev_src, src)     ||
		   !RG_STRCMP(prev_dest, src)    ||
		   !RG_STRCMP(prev_src, dest)    ||
		   !RG_STRCMP(prev_dest, dest)) return true;
	}
	return false;
}

static int _score_expression(AlgebraicExpression *exp, const QueryGraph *qg, rax *bound_vars,
							 rax *filtered_entities, rax *independent_entities) {
	int score = 0;
	score += AlgebraicExpression_LabelsScore(exp, qg);
	score += AlgebraicExpression_FilterExistenceScore(exp, filtered_entities);
	score += AlgebraicExpression_IndependentFilterScore(exp, independent_entities);
	score += AlgebraicExpression_BoundVariableScore(exp, bound_vars);
	return score;
}

// move to unit test

int score_arrangement(OrderScoreCtx *score_ctx, AlgebraicExpression **exps, uint exp_count) {
	int score = 0;
	// TODO: check if we need to location consideration.
	for(uint i = 0; i < exp_count; i++) {
		score += _score_expression(exps[i], score_ctx->qg, score_ctx->bound_vars,
								   score_ctx->filtered_entities, score_ctx->independent_entities);
	}
	return score;
}


/**
 * @brief  Helper method to _FilterTree_CollectIndependentEntities, that does the actual verification and collection of
 * indpendent entities from predicate filter tree nodes.
 * @note   The method will assert upon non predicate filter tree nodes.
 * @param  *pred: Predicate filter tree node.
 * @param  *independent_entities: (in-out-by-ref) rax to hold the independent entities and the the number of their independent occurrences.
 */
static void _FilterTreePredicate_CollectIndependentEntity(const FT_FilterNode *pred,
														  rax *independent_entities) {
	ASSERT(pred->t == FT_N_PRED);
	// Collect the entities in the predicate.
	rax *entities = FilterTree_CollectModified(pred);
	// If this is the single entity in the predicate, it is an independent entity.
	if(raxSize(entities) == 1) {
		raxIterator it;
		raxStart(&it, entities);
		// Iterate over all keys in the rax.
		raxSeek(&it, "^", NULL, 0);
		while(raxNext(&it)) {
			// Add the entity to the independent_entities rax.
			void *res = raxFind(independent_entities, it.key, it.key_len);
			if(res == raxNotFound) {
				raxInsert(independent_entities, it.key, it.key_len, (void *)1, NULL);
			} else {
				raxInsert(independent_entities, it.key, it.key_len, (void *)(res + 1), NULL);
			}
		}
		raxStop(&it);
	}
	raxFree(entities);
}

/**
 * @brief  This method collect independent entities, and the number of their independent occurrences from a filter tree.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *root: Filter tree root.
 * @param  *independent_entities: (in-out-by-ref) rax to hold the independent entities and the the number of their independent occurrences.
 */
void FilterTree_CollectIndependentEntities(const FT_FilterNode *root,
										   rax *independent_entities) {
	if(root == NULL) return;

	switch(root->t) {
	case FT_N_COND: {
		FilterTree_CollectIndependentEntities(root->cond.left, independent_entities);
		FilterTree_CollectIndependentEntities(root->cond.right, independent_entities);
		break;
	}
	case FT_N_PRED: {
		_FilterTreePredicate_CollectIndependentEntity(root, independent_entities);
		break;
	}
	case FT_N_EXP: {
		break;
	}
	default: {
		ASSERT(0);
		break;
	}
	}
}