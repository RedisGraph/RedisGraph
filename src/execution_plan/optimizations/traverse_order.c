/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./traverse_order.h"
#include "../../RG.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/strcmp.h"
#include "../../util/rmalloc.h"
#include "traverse_order_utils.h"

// Compare macro used to sort scored expressions.
#define score_cmp(a,b) ((*a).score > (*b).score)

// Algebraic expression associated with a score
typedef struct {
	int score;                 // Score given to expression
	AlgebraicExpression *exp;  // Algebraic expression
} ScoredExp;

// Print arrangement.
static inline void _Arrangement_Print(AlgebraicExpression **arrangement, uint size) {
	printf("Arrangement_Print\n");
	for(uint i = 0; i < size; i++) {
		AlgebraicExpression *exp = arrangement[i];
		printf("%d, src: %s, dest: %s\n", i, AlgebraicExpression_Source(exp),
			   AlgebraicExpression_Destination(exp));
	}
}

/* Having chosen which algebraic expression will be evaluated first, determine whether
 * it is worthwhile to transpose it and thus swap the source and destination.
 * We will score both endpoint according to the following order of importance:
 * bound variable >> independent filtered entity >> filtered entity >> labeled entity */
static bool _should_transpose_entry_point(const QueryGraph *qg, AlgebraicExpression *ae,
										  rax *filtered_entities, rax *independent_entities, rax *bound_vars) {
	// Single operand, source equals to destination.
	if(AlgebraicExpression_OperandCount(ae) == 1 &&
	   !RG_STRCMP(AlgebraicExpression_Source(ae), AlgebraicExpression_Destination(ae))) return false;

	const char *src = AlgebraicExpression_Source(ae);
	const char *dest = AlgebraicExpression_Destination(ae);
	QGNode *src_node = QueryGraph_GetNodeByAlias(qg, src);
	QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, dest);

	int src_score = 0;
	int dst_score = 0;
	int max;

	// Label scoring.
	src_score = QGNode_LabelsScore(src_node);
	dst_score = QGNode_LabelsScore(dest_node);
	max = MAX(src_score, dst_score);

	if(filtered_entities) {
		// Filtered entities.
		src_score += (max + 1) * QGEntity_FilterExistenceScore(src, filtered_entities);
		dst_score += (max + 1) * QGEntity_FilterExistenceScore(dest, filtered_entities);
		max = MAX(src_score, dst_score);

		// Independent filtered entities.
		src_score += (max + 1) * QGEntity_IndependentFilterScore(src, independent_entities);
		dst_score += (max + 1) * QGEntity_IndependentFilterScore(dest, independent_entities);
		max = MAX(src_score, dst_score);
	}

	// Bound variables.
	if(bound_vars) {
		src_score += (max + 1) * QGEntity_BoundVariableScore(src, bound_vars);
		dst_score += (max + 1) * QGEntity_BoundVariableScore(dest, bound_vars);
	}

	return src_score < dst_score;
}

// Transpose out-of-order expressions so that each expresson's source is resolved
// in the winning sequence.
static void _resolve_winning_sequence(AlgebraicExpression **exps, uint exp_count) {
	for(uint i = 1; i < exp_count; i ++) {
		bool src_resolved = false;
		AlgebraicExpression *exp = exps[i];
		const char *src = AlgebraicExpression_Source(exp);

		// See if source is already resolved.
		for(int j = i - 1; j >= 0; j--) {
			AlgebraicExpression *prev_exp = exps[j];
			if(!RG_STRCMP(AlgebraicExpression_Source(prev_exp), src) ||
			   !RG_STRCMP(AlgebraicExpression_Destination(prev_exp), src)) {
				src_resolved = true;
				break;
			}
		}
		if(!src_resolved) AlgebraicExpression_Transpose(exps + i);
	}
}

// Score each individual expression and reorder the array in order of score.
static void _score_expressions
(
	ScoredExp *scored_exps,
	AlgebraicExpression **exps,
	uint nexp,
	rax *bound_vars,
	rax *filtered_entities,
	rax *independent_entities,
	const QueryGraph *qg
) {
	/* The scoreing of algebraic expression is done according to 4 criteria (ordered by weakest to strongest):
	 * 1. Label(s) on the expression source or destination (inclusive or).
	 * 2. Existence of filters on either source or destinaion.
	 * 3. The number of independent filters on the source or destination (e.g. the node is not evaluated against other entities).
	 * 4. The source or destination are bound variables.
	 *
	 * The expressions will be evaluated in 4 phases, one for each criteria.
	 * The score given for the for each criteria is:
	 * (1 + the maximum score given in the previous criteria) * (criteria scoring function)
	 * where the first criteria starts with 1 * (criteria scoring function)
	 *
	 * phase 1 - check for labels on either source or destinaion:
	 * phase 1 expression scoring = 1 * _expression_labels_score
	 *
	 * phase 2 - check for existence of filters on either source or destinaion.
	 * phase 2 expression scoring = (1 + max(phase 1 scoring results)) * _expression_filter_existence_score
	 *
	 * phase 3 - number of independent filters on the source or destination
	 * phase 3 expression scoring = (1 + max(phase 2 scoring results)) * _expression_independent_filter_score
	 *
	 * phase 4 - bound variables.
	 * phase expression scoring = (1 + max(phase 3 scoring results)) *  _expression_bound_variable_score
	 */

	int max = 0;
	for(uint i = 0; i < nexp; i ++) {
		AlgebraicExpression *exp = exps[i];
		ScoredExp *scored_exp = scored_exps + i;
		scored_exp->exp = exp;
		scored_exp->score = AlgebraicExpression_LabelsScore(exp, qg);
		max = max < scored_exp->score ? scored_exp->score : max;
	}
	// Update phase 1 maximum score.
	int currmax = max;
	if(filtered_entities) {
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) * AlgebraicExpression_FilterExistenceScore(exp,
																						  filtered_entities);
			max = max < scored_exp->score ? scored_exp->score : max;
		}
		// Update phase 2 maximum score.
		currmax = max;
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) *
								 AlgebraicExpression_IndependentFilterScore(exp, independent_entities);
			max = max < scored_exp->score ? scored_exp->score : max;
		}
		// Update phase 3 maximum score.
		currmax = max;
	}
	if(bound_vars) {
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) * AlgebraicExpression_BoundVariableScore(exp, bound_vars);
		}
	}
}

// Construct a sorted list of valid expressions to consider, given a subset of
// expression already in use 'arrangment' these will not show up in the
// returned list.
// The elements are sorted by their score.
AlgebraicExpression **_valid_expressions
(
	const ScoredExp *exps,             // input list of expressions
	uint nexp,                         // number of expressions
	AlgebraicExpression **restricted,  // expressions already in use
	uint nrestricted                   // number of elements in restricted
) {
	// Sorted array of valid expressions to return.
	AlgebraicExpression **options = array_new(AlgebraicExpression *, 0);

	for(int i = nexp - 1; i >= 0; i--) {
		// See if current expression is a valid expression to use
		// A valid expression is one which isn't already in use
		// and either its source or destination have been encountered.
		bool valid = true;
		AlgebraicExpression *exp = exps[i].exp;

		// See if expression already in use.
		uint j = 0;
		for(; j < nrestricted && valid; j++) valid = (exp != restricted[j]);
		if(!valid) continue;

		// See if either exp source or destination were already encountered.
		const char *src = AlgebraicExpression_Source(exp);
		const char *dest  = AlgebraicExpression_Destination(exp);
		for(j = 0; j < nrestricted; j++) {
			valid = false;
			AlgebraicExpression *used = restricted[j];
			const char *used_src = AlgebraicExpression_Source(used);
			const char *used_dest  = AlgebraicExpression_Destination(used);
			if(RG_STRCMP(src, used_src) == 0 ||
			   RG_STRCMP(src, used_dest) == 0 ||
			   RG_STRCMP(dest, used_src) == 0 ||
			   RG_STRCMP(dest, used_dest) == 0) {
				valid = true;
				break;
			}
		}

		if(valid) options = array_append(options, exp);
	}

	return options;
}

bool _arrangment_set_expression
(
	AlgebraicExpression **arrangment,   // arrangment of expressions
	const ScoredExp *exps,				// input list of expressions
	uint nexp,                          // number of expressions
	AlgebraicExpression **options,	    // posible expressions for position i
	uint i                              // index in arrangment to resolve
) {
	// Done.
	if(i == nexp) {
		array_free(options);
		return true;
	}

	bool position_set = false;  // did we manage to resolve position i..n

	//--------------------------------------------------------------------------
	// Find the best posible expression to place at position i
	//--------------------------------------------------------------------------

	/* As long as we didn't find an expression for position i
	 * and there are options to go through. */
	while(!position_set && array_len(options) > 0) {
		// options are sorted by score.
		AlgebraicExpression *exp = array_pop(options);

		// Set current expression in arrangment.
		arrangment[i] = exp;

		// Compose a list of valid expressions for next position.
		AlgebraicExpression **follows;
		follows = _valid_expressions(exps, nexp, arrangment, i + 1);
		position_set = _arrangment_set_expression(arrangment, exps, nexp, follows, i + 1);
	}

	array_free(options);
	return position_set;
}

void _order_expressions
(
	AlgebraicExpression **arrangment,   // arrangment of expressions
	const ScoredExp *exps,				// input list of expressions
	uint nexp,                          // number of expressions
	uint i                              // index in arrangment to resolve
) {
	AlgebraicExpression **options = _valid_expressions(exps, nexp, NULL, 0);
	bool res = _arrangment_set_expression(arrangment, exps, nexp, options, 0);
	ASSERT(res == true);
}

/* Given a set of algebraic expressions representing a graph traversal
 * we pick the order in which the expressions will be evaluated
 * taking into account filters and transposes.
 * exps will be reordered. */
void orderExpressions(const QueryGraph *qg, AlgebraicExpression **exps, uint exp_count,
					  const FT_FilterNode *filter_tree, rax *bound_vars) {
	// Validate inputs
	ASSERT(qg != NULL);
	ASSERT(exps != NULL && exp_count > 0);

	ScoredExp scored_exps[exp_count];
	AlgebraicExpression *arrangment[exp_count];

	// Collect all filtered aliases.
	rax *filtered_entities = NULL;
	rax *independent_entities = NULL;
	if(filter_tree) {
		filtered_entities = FilterTree_CollectModified(filter_tree);
		independent_entities = raxNew();
		FilterTree_CollectIndependentEntities(filter_tree, independent_entities);
	}

	//--------------------------------------------------------------------------
	// Score each expression and sort
	//--------------------------------------------------------------------------

	// Associate each expression with a score.
	_score_expressions(scored_exps, exps, exp_count, bound_vars,
					   filtered_entities, independent_entities, qg);

	// Sort scored_exps on score in descending order.
	QSORT(ScoredExp, scored_exps, exp_count, score_cmp);

	//--------------------------------------------------------------------------
	// Find the highest score valid arrangment
	//--------------------------------------------------------------------------

	_order_expressions(arrangment, scored_exps, exp_count, 0);

	// Overwrite the original expressions array with the optimal arrangement.
	memcpy(exps, arrangment, exp_count * sizeof(AlgebraicExpression *));

	/* Transpose expressions as necessary so that
	 * the traversals will work in the selected order. */
	_resolve_winning_sequence(exps, exp_count);

	/* Transpose the winning expression if the destination node
	 * is a more efficient starting point. */
	if(_should_transpose_entry_point(qg, exps[0], filtered_entities, independent_entities, bound_vars))
		AlgebraicExpression_Transpose(exps);

	if(filter_tree) {
		raxFree(filtered_entities);
		raxFree(independent_entities);
	}
}

