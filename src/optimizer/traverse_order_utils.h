/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../IR/filter_tree/filter_tree.h"
#include "../IR/algebraic_expression/algebraic_expression.h"
#include "../../deps/rax/rax.h"

// algebraic expression associated with a score
typedef struct {
	int score;                 // score given to expression
	AlgebraicExpression *exp;  // algebraic expression
} ScoredExp;

 // collect independent entities
 // and the number of their independent occurrences from a filter tree
 // an indpendent entity is an entity that is the single entity in a predicate
 // 'root' - filter tree root
 // 'entities' rax holding independent entities frequency
void FilterTree_CollectIndependentEntities
(
	const FT_FilterNode *root,
	rax *entities
);

// associates each expression with a score
// a score for a given expression is based on a score given to other expressions
void TraverseOrder_ScoreExpressions
(
	ScoredExp *scored_exps,      // sorted array of scored expressions
	AlgebraicExpression **exps,  // expressions to score
	uint nexp,                   // number of expressions
	rax *bound_vars,             // map of bounded entities
	rax *filtered_entities,      // map of filtered entities
	const QueryGraph *qg         // query graph
);

