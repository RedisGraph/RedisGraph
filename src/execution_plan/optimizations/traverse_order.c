/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./traverse_order.h"
#include "../../RG.h"
#include "../../util/arr.h"
#include "../../util/strcmp.h"
#include "../../util/rmalloc.h"
#include "traverse_order_utils.h"


// Print arrangement.
static inline void _Arrangement_Print(AlgebraicExpression **arrangement, uint size) {
	printf("Arrangement_Print\n");
	for(uint i = 0; i < size; i++) {
		AlgebraicExpression *exp = arrangement[i];
		printf("%d, src: %s, dest: %s\n", i, AlgebraicExpression_Source(exp),
			   AlgebraicExpression_Destination(exp));
	}
}

// having chosen which algebraic expression will be evaluated first
// determine whether it is worthwhile to transpose it
// thus swap the source and destination
static bool _should_transpose_entry_point(const QueryGraph *qg,
		AlgebraicExpression *ae, rax *filtered_entities,
		rax *independent_entities, rax *bound_vars) {
	// validate inputs
	ASSERT(qg                    !=  NULL);
	ASSERT(ae                    !=  NULL);
	ASSERT(filtered_entities     !=  NULL);
	ASSERT(independent_entities  !=  NULL);
	ASSERT(bound_vars            !=  NULL);

	// consider src and dest as stand-alone expressions
	const char *src  = AlgebraicExpression_Source(ae);
	const char *dest = AlgebraicExpression_Destination(ae);

	AlgebraicExpression *src_exp = AlgebraicExpression_NewOperand(GrB_NULL,
			false, src, src, NULL, NULL);
	AlgebraicExpression *dest_exp = AlgebraicExpression_NewOperand(GrB_NULL,
			false, dest, dest, NULL, NULL);

	// transpose if top scored expression is 'dest_exp'
	int src_score = score_expression(src_exp, qg, bound_vars,
			filtered_entities, independent_entities);
	int dest_score = score_expression(dest_exp, qg, bound_vars,
			filtered_entities, independent_entities);
	bool transpose = dest_score > src_score;

	AlgebraicExpression_Free(src_exp);
	AlgebraicExpression_Free(dest_exp);

	return transpose;
}

// transpose out-of-order expressions such that each expresson's source
// is resolved in the winning arrangment
static void _resolve_winning_sequence(AlgebraicExpression **exps, uint exp_count) {
	// skip opening expression
	for(uint i = 1; i < exp_count; i ++) {
		bool src_resolved = false;
		AlgebraicExpression *exp = exps[i];
		const char *src = AlgebraicExpression_Source(exp);

		// see if source is already resolved
		for(int j = i - 1; j >= 0; j--) {
			AlgebraicExpression *prev_exp = exps[j];
			if(!RG_STRCMP(AlgebraicExpression_Source(prev_exp), src) ||
			   !RG_STRCMP(AlgebraicExpression_Destination(prev_exp), src)) {
				src_resolved = true;
				break;
			}
		}
		if(!src_resolved) AlgebraicExpression_Transpose(exps + i);
		// TODO: when DEBUG is enabled, make sure transposed expression
		// source (prev dest) is resolved
	}
}

// construct a sorted list of valid expressions to consider, given a subset of
// expression already in use 'arrangment' these will not show up in the
// returned list.
// the elements are sorted by their score
AlgebraicExpression **_valid_expressions
(
	const ScoredExp *exps,             // input list of expressions
	uint nexp,                         // number of expressions
	AlgebraicExpression **restricted,  // expressions already in use
	uint nrestricted                   // number of elements in restricted
) {
	// sorted array of valid expressions to return
	AlgebraicExpression **options = array_new(AlgebraicExpression *, 0);

	for(int i = nexp - 1; i >= 0; i--) {
		// see if current expression is a valid expression to use
		// a valid expression is one which isn't already in use
		// and either its source or destination have been encountered
		bool valid = true;
		AlgebraicExpression *exp = exps[i].exp;

		// see if expression already in use
		for(uint j = 0; j < nrestricted && valid; j++) {
			valid = (exp != restricted[j]);
		}
		if(!valid) continue;

		// see if either exp source or destination were already encountered
		const char *src  = AlgebraicExpression_Source(exp);
		const char *dest = AlgebraicExpression_Destination(exp);

		for(uint j = 0; j < nrestricted; j++) {
			valid = false;
			AlgebraicExpression *used = restricted[j];
			const char *used_src = AlgebraicExpression_Source(used);
			const char *used_dest  = AlgebraicExpression_Destination(used);

			if( RG_STRCMP(src, used_src)   == 0  ||
				RG_STRCMP(src, used_dest)  == 0  ||
				RG_STRCMP(dest, used_src)  == 0  ||
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
	AlgebraicExpression **follows = NULL;

	//--------------------------------------------------------------------------
	// Find the best posible expression to place at position i
	//--------------------------------------------------------------------------

	// as long as we didn't find an expression for position i
	// and there are options to go through
	while(!position_set && array_len(options) > 0) {
		// options are sorted by score
		AlgebraicExpression *exp = array_pop(options);

		// set current expression in arrangment
		arrangment[i] = exp;

		// compose a list of valid expressions for next position
		follows = _valid_expressions(exps, nexp, arrangment, i + 1);
		
		// if follows doesn't contains enough options we're destined to fail
		// try the next expression for the current position
		if(array_len(follows) != nexp - (i + 1)) {
			array_free(follows);
			continue;
		}

		// position i set, recursively advance to next position
		position_set = _arrangment_set_expression(arrangment, exps, nexp,
				follows, i + 1);
	}

	array_free(options);
	return position_set;
}

void _order_expressions
(
	AlgebraicExpression **arrangment,   // arrangment of expressions
	const ScoredExp *exps,				// input list of expressions
	uint nexp                           // number of expressions
) {
	// collect all possible expression for first position in arrangment
	AlgebraicExpression **options = _valid_expressions(exps, nexp, NULL, 0);

	// construct arrangment
	bool res = _arrangment_set_expression(arrangment, exps, nexp, options, 0);
	ASSERT(res == true);
}

/* Given a set of algebraic expressions representing a graph traversal
 * we pick the order in which the expressions will be evaluated
 * taking into account filters and transposes.
 * 'exps' will be reordered. */
void orderExpressions(const QueryGraph *qg, AlgebraicExpression **exps,
		uint exp_count, const FT_FilterNode *ft, rax *bound_vars) {
	// Validate inputs
	ASSERT(qg   != NULL);
	ASSERT(exps != NULL && exp_count > 0);

	ScoredExp scored_exps[exp_count];
	AlgebraicExpression *arrangment[exp_count];

	// collect all filtered aliases
	rax  *filtered_entities     =  NULL;
	rax  *independent_entities  =  NULL;
	if(ft) {
		filtered_entities    = FilterTree_CollectModified(ft);
		independent_entities = FilterTree_CollectIndependentEntities(ft);
	}

	//--------------------------------------------------------------------------
	// score each expression and sort
	//--------------------------------------------------------------------------

	// associate each expression with a score
	TraverseOrder_Rank_Expressions(scored_exps, exps, exp_count, bound_vars,
					   filtered_entities, independent_entities, qg);

	//--------------------------------------------------------------------------
	// Find the highest score valid arrangment
	//--------------------------------------------------------------------------

	_order_expressions(arrangment, scored_exps, exp_count);

	// overwrite the original expressions array with the optimal arrangement
	memcpy(exps, arrangment, exp_count * sizeof(AlgebraicExpression *));

	// transpose expressions as necessary so that the traversals will work in
	// the selected order
	_resolve_winning_sequence(exps, exp_count);

	// transpose the winning expression if the destination node is a more
	// efficient starting point
	if(_should_transpose_entry_point(qg, exps[0], filtered_entities,
				independent_entities, bound_vars)) {
		AlgebraicExpression_Transpose(exps);
	}

	if(ft) {
		raxFree(filtered_entities);
		raxFree(independent_entities);
	}
}

