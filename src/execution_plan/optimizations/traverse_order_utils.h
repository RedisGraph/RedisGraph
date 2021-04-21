/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/rax/rax.h"

// the purpose of this utility file is to be in use in both the:
// traverse_order optimizer, as well as in the module's unit test
typedef struct {
	QueryGraph *qg;
	rax *bound_vars;
	rax *filtered_entities;
	rax *independent_entities;
	AlgebraicExpression **best_arrangement;
	int max_score;
} OrderScoreCtx;

// Algebraic expression associated with a score
typedef struct {
	int score;                 // score given to expression
	AlgebraicExpression *exp;  // algebraic expression
} ScoredExp;

/**
 * @brief
 * @note
 * @param  **expf:
 * @param  pos:
 * @param  *exp:
 * @param  *qg:
 * @retval
 */
bool valid_position
(
	AlgebraicExpression **exps,
	int pos,
   	AlgebraicExpression *exp,
   	QueryGraph *qg
);

int score_expression
(
	AlgebraicExpression *exp,
	const QueryGraph *qg,
	rax *bound_vars,
	rax *filtered_entities,
	rax *independent_entities
);

/**
 * @brief  A method to be used
 * @note
 * @param  *score_ctx:
 * @param  **exps:
 * @param  exp_count:
 * @retval
 */
int score_arrangement
(
	OrderScoreCtx *score_ctx,
	AlgebraicExpression **exps,
	uint exp_count
);

 // collect independent entities
 // and the number of their independent occurrences from a filter tree
 // an indpendent entity is an entity that is the single entity in a predicate
 // 'root' - filter tree root
 // returns rax holding independent entities and their frequency
rax *FilterTree_CollectIndependentEntities
(
	const FT_FilterNode *root
);

void TraverseOrder_Rank_Expressions
(
	ScoredExp *scored_exps,      // sorted array of scored expressions
	AlgebraicExpression **exps,  // expressions to score
	uint nexp,                   // number of expressions
	rax *bound_vars,             // map of bounded entities
	rax *filtered_entities,      // map of filtered entities
	rax *independent_entities,   // TODO: mark independent (true/flase) in filtered_entities
	const QueryGraph *qg         // query graph
);

