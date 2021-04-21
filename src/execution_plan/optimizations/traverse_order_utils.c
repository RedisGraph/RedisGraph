/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/strcmp.h"
#include "traverse_order_utils.h"

//------------------------------------------------------------------------------
// Scoring functions
//------------------------------------------------------------------------------

// score expression based on its source and destination labels
// score = src label count + dest label count
int TraverseOrder_LabelsScore(AlgebraicExpression *exp, const QueryGraph *qg) {
	ASSERT(exp != NULL);
	ASSERT(gq  != NULL);

	int         score       =  0;
	const char  *src        =  AlgebraicExpression_Source(exp);
	const char  *dest       =  AlgebraicExpression_Destination(exp);
	QGNode      *src_node   =  QueryGraph_GetNodeByAlias(qg, src);
	QGNode      *dest_node  =  QueryGraph_GetNodeByAlias(qg, dest);

	score += QGNode_LabelCount(src_node);
	score += QGNode_LabelCount(dest_node);
	return score;
}

// score expression based on the existence of filters applied to entities
// in the expression
int TraverseOrder_FilterExistenceScore(AlgebraicExpression *exp,
											 rax *filtered_entities) {
	ASSERT(exp != NULL);

	if(!filtered_entities) return 0;

	int          score  =  0;
	const  char  *src   =  AlgebraicExpression_Source(exp);
	const  char  *dest  =  AlgebraicExpression_Destination(exp);
	const  char  *edge  =  AlgebraicExpression_Edge(exp);

	bool src_filtered = raxFind(filtered_entities, (unsigned char *)src,
			strlen(src)) != raxNotFound;

	bool dest_filtered = raxFind(filtered_entities, (unsigned char *)dest,
			strlen(dest)) != raxNotFound;

	bool edge_filtered = raxFind(filtered_entities, (unsigned char *)edge,
			strlen(edge)) != raxNotFound;

	// filters operating on nodes are considered more valuable
	if(src_filtered)  score += 2;
	if(dest_filtered) score += 2;
	if(edge_filtered) score += 1;

	return score;
}

// score expression based on the existence of a filter operating solely on one
// of the entities in the expression
int TraverseOrder_IndependentFilterScore(AlgebraicExpression *exp,
											   rax *independent_entities) {
	ASSERT(exp != NULL);

	if(!independent_entities) return 0;

	int        score       =  0;
	void       *frequency  =  NULL;
	const char *src        =  AlgebraicExpression_Source(exp);
	const char *dest       =  AlgebraicExpression_Destination(exp);
	const char *edge       =  AlgebraicExpression_Edge(exp);

	// filters operating on nodes are considered more valuable
	frequency = raxFind(independent_entities, (unsigned char *)src,
			strlen(src));
	if(frequency != raxNotFound) score += (int64_t)frequency * 2;

	frequency = raxFind(independent_entities, (unsigned char *)dest,
			strlen(dest));
	if(frequency != raxNotFound) score += (int64_t)frequency * 2;

	frequency = raxFind(independent_entities, (unsigned char *)edge,
			strlen(edge));
	if(frequency != raxNotFound) score += (int64_t)frequency;

	return score;
}

// score expression based on bounded nodes
int TraverseOrder_BoundVariableScore(AlgebraicExpression *exp,
		rax *bound_vars) {
	ASSERT(exp != NULL);

	if(!bound_vars) return 0;

	int        score  =  0;
	const char *src   =  AlgebraicExpression_Source(exp);
	const char *dest  =  AlgebraicExpression_Destination(exp);

	bool src_bound = raxFind(bound_vars, (unsigned char *)src,
			strlen(src)) != raxNotFound;

	bool dest_bound = raxFind(bound_vars, (unsigned char *)dest,
			strlen(dest)) != raxNotFound;

	if(src_bound)  score += 1;
	if(dest_bound) score += 1;

	return score;
}

// scores individual expression
// scoreing is done according to 4 criterias
// ordered by weakest to strongest 
// 1. Label(s) on the expression source or destination
// 2. Existence of filters on either source or destinaion
// 3. The number of independent filters on the source or destination
// 4. The source or destination are bound
int score_expression(AlgebraicExpression *exp, const QueryGraph *qg, rax *bound_vars,
							 rax *filtered_entities, rax *independent_entities) {
	// score from least significant to most significat attribute
	int score = 0;
	score += MAX(1, score + TraverseOrder_LabelsScore(exp, qg));
	score += MAX(1, score + TraverseOrder_FilterExistenceScore(exp, filtered_entities));
	score += MAX(1, score + TraverseOrder_IndependentFilterScore(exp, independent_entities));
	score += MAX(1, score + TraverseOrder_BoundVariableScore(exp, bound_vars));
	return score;
}

// scores given arrangement
// taking into considiration the position of each algebraic expression in the
// arrangement, ealier expression are scored higly then later expressions
// TODO: move to unit test
int score_arrangement(OrderScoreCtx *score_ctx, AlgebraicExpression **exps,
		uint exp_count) {
	int score = 0;
	for(uint i = 0; i < exp_count; i++) {
		// value expression[i] in arrangment more then expression[j], i < j
		uint factor = exp_count - i;
		score += factor * score_expression(exps[i], score_ctx->qg,
				score_ctx->bound_vars, score_ctx->filtered_entities,
				score_ctx->independent_entities);
	}
	return score;
}

// move to unit test
bool valid_position(AlgebraicExpression **exps, int pos, AlgebraicExpression *exp,
		QueryGraph *qg) {
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

 // collect independent entities
 // and the number of their independent occurrences from a filter tree
 // an indpendent entity is an entity that is the single entity in a predicate
 // 'root' - filter tree root
 // returns rax holding independent entities and their frequency
rax *FilterTree_CollectIndependentEntities(const FT_FilterNode *root) {
	ASSERT(root != NULL);

	rax            *set            =  raxNew();
	FT_FilterNode  *tree           =  FilterTree_Clone(root);
	FT_FilterNode  **sub_trees     =  FilterTree_SubTrees(tree);
	uint           sub_tree_count  =  array_len(sub_trees);

	for(uint i = 0; i < sub_tree_count; i++) {
		raxIterator it;
		FT_FilterNode *t = sub_trees[i];
		rax *entities = FilterTree_CollectModified(t);

		if(raxSize(entities) == 1) {
			raxStart(&it, entities);
			raxSeek(&it, "^", NULL, 0);
			raxNext(&it);
			
			// add entity to set
			void *freq = raxFind(set, it.key, it.key_len);
			freq = (freq == raxNotFound) ? (void*)1 : (void*)(freq + 1);
			raxInsert(set, it.key, it.key_len, freq, NULL);

			raxStop(&it);
		}

		raxFree(entities);
		FilterTree_Free(t);
	}

	array_free(sub_trees);

	return set;
}

// score each expression and sort expressions by score
void TraverseOrder_Rank_Expressions
(
	ScoredExp *scored_exps,      // sorted array of scored expressions
	AlgebraicExpression **exps,  // expressions to score
	uint nexp,                   // number of expressions
	rax *bound_vars,             // map of bounded entities
	rax *filtered_entities,      // map of filtered entities
	rax *independent_entities,   // TODO: mark independent (true/flase) in filtered_entities
	const QueryGraph *qg         // query graph
) {
	// scoreing of algebraic expression is done according to 4 criterias
	// ordered by to strongest weakest:
	// 1. The source or destination are bound
	// 2. The number of independent filters on the source or destination
	// 3. Existence of filters on either source or destinaion
	// 4. Label(s) on the expression source or destination
	//
	// the expressions will be evaluated in 4 phases, one for each criteria
	// the score given for the for each criteria is:
	// (1 + the maximum score given in the previous criteria) * (criteria scoring function)
	// where the first criteria starts with 1 * (criteria scoring function)
	//
	// phase 1 - check for labels on either source or destinaion
	// expression scoring = 1 * _expression_labels_score
	//
	// phase 2 - check for existence of filters on either source or destinaion
	// expression scoring = (1 + max(phase 1 scoring results)) * _expression_filter_existence_score
	//
	// phase 3 - number of independent filters on the source or destination
	// expression scoring = (1 + max(phase 2 scoring results)) * _expression_independent_filter_score
	//
	// phase 4 - bound variables
	// phase expression scoring = (1 + max(phase 3 scoring results)) *  _expression_bound_variable_score
	//

	int                  max          =  0;
	int                  score        =  0;
	AlgebraicExpression  *exp         =  NULL;
	ScoredExp            *scored_exp  =  NULL;

	//--------------------------------------------------------------------------
	//  phase 1 score label
	//--------------------------------------------------------------------------

	for(uint i = 0; i < nexp; i ++) {
		exp = exps[i];
		scored_exp = scored_exps + i;

		score = TraverseOrder_LabelsScore(exp, qg);
		scored_exp->exp = exp;
		scored_exp->score = score;

		max = MAX(max, score);
	}

	// update phase 1 maximum score
	int currmax = max;

	//--------------------------------------------------------------------------
	//  phase 2 score filters
	//--------------------------------------------------------------------------

	if(filtered_entities) {
		for(uint i = 0; i < nexp; i ++) {
			exp = exps[i];
			scored_exp = scored_exps + i;

			score = (1 + currmax) + TraverseOrder_FilterExistenceScore(
					exp, filtered_entities);

			scored_exp->score += score;
																				
			max = MAX(max, scored_exp->score);
		}

		// update phase 2 maximum score
		currmax = max;

		//----------------------------------------------------------------------
		//  phase 3 score independent filters
		//---------------------------------------------------------------------

		for(uint i = 0; i < nexp; i ++) {
			exp = exps[i];
			scored_exp = scored_exps + i;

			score = (1 + currmax) + TraverseOrder_IndependentFilterScore(exp, independent_entities);

			scored_exp->score += score;
			max = MAX(max, scored_exp->score);
		}

		// update phase 3 maximum score.
		currmax = max;
	}

	//--------------------------------------------------------------------------
	//  phase 4 score bound variables
	//--------------------------------------------------------------------------

	if(bound_vars) {
		for(uint i = 0; i < nexp; i ++) {
			exp = exps[i];
			scored_exp = scored_exps + i;

			scored_exp->score += (1 + currmax) + TraverseOrder_BoundVariableScore(exp, bound_vars);
		}
	}

	// Sort scored_exps on score in descending order.
	// Compare macro used to sort scored expressions.
	#define score_cmp(a,b) ((*a).score > (*b).score)
	QSORT(ScoredExp, scored_exps, nexp, score_cmp);
}

