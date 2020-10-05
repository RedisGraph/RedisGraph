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

#define T 1           // Transpose penalty.
#define L 2 * T       // Label score.
#define F 5 * T       // Filter score.
#define B 8 * F       // Bound variable bonus.

// Compare macro used to sort scored expressions.
#define score_cmp(a,b) ((*a).score >= (*b).score)

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
 * If the source is bounded, we will not transpose, if only the destination is bounded, we will.
 * If neither are bounded, we fall back to label and filter heuristics.
 * We'll choose to transpose if the destination is filtered and the source is not, or
 * if neither is filtered, if the destination is labeled and the source is not. */
static bool _should_transpose_entry_point(const QueryGraph *qg, AlgebraicExpression *ae,
										  rax *filtered_entities, rax *bound_vars) {
	// Single operand, source equals to destination.
	if(AlgebraicExpression_OperandCount(ae) == 1 &&
	   !RG_STRCMP(AlgebraicExpression_Source(ae), AlgebraicExpression_Destination(ae))) return false;

	const char *src = AlgebraicExpression_Source(ae);
	const char *dest = AlgebraicExpression_Destination(ae);
	uint src_len = strlen(src);
	uint dest_len = strlen(dest);

	// Always start at a bound variable if one is present.
	if(bound_vars) {
		// Source is bounded.
		if(raxFind(bound_vars, (unsigned char *)src, src_len) != raxNotFound) {
			return false;
		}

		// Destination is bounded.
		if(raxFind(bound_vars, (unsigned char *)dest, dest_len) != raxNotFound) {
			return true;
		}
	}

	bool src_filtered = false;
	bool dest_filtered = false;
	// See if either source or destination nodes are filtered.
	if(filtered_entities) {
		src_filtered = raxFind(filtered_entities, (unsigned char *)src, src_len) != raxNotFound;
		dest_filtered = raxFind(filtered_entities, (unsigned char *)dest, dest_len) != raxNotFound;
	}

	// No filters are applied prefer labeled entity.
	QGNode *src_node = QueryGraph_GetNodeByAlias(qg, src);
	QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, dest);
	bool src_labeled = src_node->label != NULL;
	bool dest_labeled = dest_node->label != NULL;

	if((src_filtered && !(dest_filtered && dest_labeled)) ||
	   (src_labeled && !dest_filtered)) {
		// Do not transpose if the destination lacks a label specification or filter
		// that the source possesses.
		return false;
	} else if((dest_filtered && !(src_filtered && src_labeled)) ||
			  (dest_labeled && !src_labeled)) {
		// Transpose if the destination has a filter and/or label and the source does not,
		// as this will shrink the scan results.
		return true;
	}
	return false;
}

static int _reward_expression
(
	AlgebraicExpression *exp,
	rax *bound_vars,
	FT_FilterNode **filters,
	rax *filtered_entities,
	const QueryGraph *qg
)
{
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	uint src_len = strlen(src);
	uint dest_len = strlen(dest);

	// TODO should destination  be considered or not?

	/* Reward bound variables such that any expression with a bound variable
	 * will be preferred over any expression without. */
	if(bound_vars) {
		if(raxFind(bound_vars, (unsigned char *)src, src_len) != raxNotFound) {
			score += B;
		}
		if(raxFind(bound_vars, (unsigned char *)dest, dest_len) != raxNotFound) {
			score += B;
		}
	}

	// Reward filters in expression.
	if(filtered_entities) {
		if(raxFind(filtered_entities, (unsigned char *)src, src_len) != raxNotFound) {
			score += F;
		}
		if(raxFind(filtered_entities, (unsigned char *)dest, dest_len) != raxNotFound) {
			score += F;
		}
	}

	QGNode *src_node = QueryGraph_GetNodeByAlias(qg, src);
	QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, dest);
	if(src_node->label) score += L;
	if(dest_node->label) score += L;

	return score;
}

static int _score_expression
(
	AlgebraicExpression *exp,
	rax *bound_vars,
	FT_FilterNode **filters,
	rax *filtered_entities,
	const QueryGraph *qg
)
{
	int score = _reward_expression(exp, bound_vars, filters,
			filtered_entities, qg);
	return score;
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
	FT_FilterNode **filters,
	rax *filtered_entities,
	const QueryGraph *qg
)
{
	// Score each individual expression.
	for(uint i = 0; i < nexp; i ++) {
		AlgebraicExpression *exp = exps[i];
		ScoredExp *scored_exp = scored_exps + i;
		scored_exp->exp = exp;
		scored_exp->score = _score_expression(exp, bound_vars, filters,
				filtered_entities, qg);
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
)
{
	// Sorted array of valid expressions to return.
	AlgebraicExpression **options = array_new(AlgebraicExpression*, 0);

	for(int i = nexp-1; i >= 0; i--) {
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
)
{
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
		follows = _valid_expressions(exps, nexp, arrangment, i+1);
		position_set = _arrangment_set_expression(arrangment, exps, nexp, follows, i+1);
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
)
{
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
	FT_FilterNode **filters = NULL;
	if(filter_tree) {
		filtered_entities = FilterTree_CollectModified(filter_tree);
		filters	= FilterTree_SubTrees(filter_tree);
	}

	//--------------------------------------------------------------------------
	// Score each expression and sort
	//--------------------------------------------------------------------------

	// Associate each expression with a score.
	_score_expressions(scored_exps, exps, exp_count, bound_vars,
			filters, filtered_entities, qg);

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
	if(_should_transpose_entry_point(qg, exps[0], filtered_entities, bound_vars))
		AlgebraicExpression_Transpose(exps);

	if(filter_tree) {
		array_free(filters);
		raxFree(filtered_entities);
	}
}

