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

/**
 * @brief  Returns the query graph node label scoring.
 * @param  *node: Query graph node.
 * @retval Node's score - number of lables.
 */
static inline int _query_graph_node_labels_score(const QGNode *node) {
	// TODO: support multiple labels.
	return node->label ? 1 : 0;
}

/**
 * @brief  Returns the score for filtered entities.
 * @param  *entity: Entity variable name.
 * @param  *filtered_entities: rax containing the filtered entities.
 * @retval 1 if the entity is filtered, 0 otherwise.
 */
static inline int _filter_existence_score(const char *entity, rax *filtered_entities) {
	return raxFind(filtered_entities, (unsigned char *)entity, strlen(entity)) != raxNotFound ? 1 : 0;
}

/**
 * @brief  Returns the score for indpendent filtered entities.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *entity: Entity variable name.
 * @param  *independent_entities: rax containing the independent filtered entities and the number of their occurrences.
 * @retval The number of the entity's independent filtered occurrences.
 */
static inline int _independent_filter_score(const char *entity, rax *independent_entities) {
	void *find_result = raxFind(independent_entities, (unsigned char *)entity, strlen(entity));
	// Avoid compiler warnings.
	int res = (int64_t)find_result;
	return find_result != raxNotFound ? res : 0;
}

/**
 * @brief  Returns the score for bound entity.
 * @param  *entity: Entity variable name.
 * @param  *bound_vars: rax containing the bounded entities.
 * @retval 1 if the entity is bounded, 0 otherwise.
 */
static inline int _bound_variable_score(const char *entity, rax *bound_vars) {
	return raxFind(bound_vars, (unsigned char *)entity, strlen(entity)) != raxNotFound ? 1 : 0;
}

/**
 * @brief  Returns the algebraic expression label scoring.
 * @param  *exp: Algebraic expression
 * @param  *qg: Query graph.
 * @retval The labels score for the expression source and destination query graph nodes.
 */
static int _expression_labels_score(AlgebraicExpression *exp, const QueryGraph *qg) {
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	QGNode *src_node = QueryGraph_GetNodeByAlias(qg, src);
	QGNode *dest_node = QueryGraph_GetNodeByAlias(qg, dest);
	// TODO: Consider cumulative scoring for multiple labels support. (:A) should be scored less than (:A:B) as multiple labels reduces
	score += _query_graph_node_labels_score(src_node);
	score += _query_graph_node_labels_score(dest_node);
	return score;
}

/**
 * @brief  Returns the score for filtered entities in an algebraic expression.
 * @param  *exp: Algebraic expression.
 * @param  *filtered_entities: rax containing the filtered entities.
 * @retval The filtered entities score for the expression source and destination.
 */
static int _expression_filter_existence_score(AlgebraicExpression *exp, rax *filtered_entities) {
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += _filter_existence_score(src, filtered_entities);
	score += _filter_existence_score(dest, filtered_entities);
	return score;
}

/**
 * @brief  Returns the score for indpendent filtered entities in an algebraic expression.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *exp: Algebraic expression.
 * @param  *independent_entities: rax containing the independent filtered entities and the number of their occurrences.
 * @retval The independent filtered entities score for the expression source and destination.
 */
static int _expression_independent_filter_score(AlgebraicExpression *exp,
												rax *independent_entities) {
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += _independent_filter_score(src, independent_entities);
	score += _independent_filter_score(dest, independent_entities);
	return score;
}

/**
 * @brief Returns the score for bounded entities in an algebraic expression.
 * @param  *exp: Algebraic expression.
 * @param  *bound_vars: rax containing the bounded entities.
 * @retval The bound variables score for the expression source and destination.
 */
static int _expression_bound_variable_score(AlgebraicExpression *exp, rax *bound_vars) {
	int score = 0;
	const char *src = AlgebraicExpression_Source(exp);
	const char *dest = AlgebraicExpression_Destination(exp);
	score += _bound_variable_score(src, bound_vars);
	score += _bound_variable_score(dest, bound_vars);
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
static void _FilterTree_CollectIndependentEntities(const FT_FilterNode *root,
												   rax *independent_entities) {
	if(root == NULL) return;

	switch(root->t) {
	case FT_N_COND: {
		_FilterTree_CollectIndependentEntities(root->cond.left, independent_entities);
		_FilterTree_CollectIndependentEntities(root->cond.right, independent_entities);
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
	src_score = _query_graph_node_labels_score(src_node);
	dst_score = _query_graph_node_labels_score(dest_node);
	max = MAX(src_score, dst_score);

	if(filtered_entities) {
		// Filtered entities.
		src_score += (max + 1) * _filter_existence_score(src, filtered_entities);
		dst_score += (max + 1) * _filter_existence_score(dest, filtered_entities);
		max = MAX(src_score, dst_score);

		// Independent filtered entities.
		src_score += (max + 1) * _independent_filter_score(src, independent_entities);
		dst_score += (max + 1) * _independent_filter_score(dest, independent_entities);
		max = MAX(src_score, dst_score);
	}

	// Bound variables.
	if(bound_vars) {
		src_score += (max + 1) * _bound_variable_score(src, bound_vars);
		dst_score += (max + 1) * _bound_variable_score(dest, bound_vars);
	}

	return src_score >= dst_score;
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
	rax *independent_entities,
	rax *filtered_entities,
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
		scored_exp->score = _expression_labels_score(exp, qg);
		max = max < scored_exp->score ? scored_exp->score : max;
	}
	// Update phase 1 maximum score.
	int currmax = max;
	if(filtered_entities) {
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) * _expression_filter_existence_score(exp, filtered_entities);
			max = max < scored_exp->score ? scored_exp->score : max;
		}
		// Update phase 2 maximum score.
		currmax = max;
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) *
								 _expression_independent_filter_score(exp, independent_entities);
			max = max < scored_exp->score ? scored_exp->score : max;
		}
		// Update phase 3 maximum score.
		currmax = max;
	}
	if(bound_vars) {
		for(uint i = 0; i < nexp; i ++) {
			AlgebraicExpression *exp = exps[i];
			ScoredExp *scored_exp = scored_exps + i;
			scored_exp->score += (1 + currmax) * _expression_bound_variable_score(exp, bound_vars);
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
		_FilterTree_CollectIndependentEntities(filter_tree, independent_entities);
	}

	//--------------------------------------------------------------------------
	// Score each expression and sort
	//--------------------------------------------------------------------------

	// Associate each expression with a score.
	_score_expressions(scored_exps, exps, exp_count, bound_vars,
					   independent_entities, filtered_entities, qg);

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

