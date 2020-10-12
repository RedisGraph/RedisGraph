/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../../deps/rax/rax.h"

// The purpose of this module is to be in use in both traverse_order module, as well as in the module's unit test.

typedef struct {
	QueryGraph *qg;
	rax *bound_vars;
	rax *filtered_entities;
	rax *independent_entities;
	AlgebraicExpression **best_arrangement;
	int max_score;
} OrderScoreCtx;

/**
 * @brief  Returns the query graph node label scoring.
 * @param  *node: Query graph node.
 * @retval Node's score - number of lables.
 */
int QGNode_LabelsScore(const QGNode *node);

/**
 * @brief  Returns the score for filtered entities.
 * @param  *entity: Entity variable name.
 * @param  *filtered_entities: rax containing the filtered entities.
 * @retval 1 if the entity is filtered, 0 otherwise.
 */
int QGEntity_FilterExistenceScore(const char *entity, rax *filtered_entities);

/**
 * @brief  Returns the score for indpendent filtered entities.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *entity: Entity variable name.
 * @param  *independent_entities: rax containing the independent filtered entities and the number of their occurrences.
 * @retval The number of the entity's independent filtered occurrences.
 */
int QGEntity_IndependentFilterScore(const char *entity, rax *independent_entities);

/**
 * @brief  Returns the score for bound entity.
 * @param  *entity: Entity variable name.
 * @param  *bound_vars: rax containing the bounded entities.
 * @retval 1 if the entity is bounded, 0 otherwise.
 */
int QGEntity_BoundVariableScore(const char *entity, rax *bound_vars);

/**
 * @brief  Returns the algebraic expression label scoring.
 * @param  *exp: Algebraic expression
 * @param  *qg: Query graph.
 * @retval The labels score for the expression source and destination query graph nodes.
 */
int AlgebraicExpression_LabelsScore(AlgebraicExpression *exp, const QueryGraph *qg);

/**
 * @brief  Returns the score for filtered entities in an algebraic expression.
 * @param  *exp: Algebraic expression.
 * @param  *filtered_entities: rax containing the filtered entities.
 * @retval The filtered entities score for the expression source and destination.
 */
int AlgebraicExpression_FilterExistenceScore(AlgebraicExpression *exp,
											 rax *filtered_entities);

/**
 * @brief  Returns the score for indpendent filtered entities in an algebraic expression.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *exp: Algebraic expression.
 * @param  *independent_entities: rax containing the independent filtered entities and the number of their occurrences.
 * @retval The independent filtered entities score for the expression source and destination.
 */
int AlgebraicExpression_IndependentFilterScore(AlgebraicExpression *exp,
											   rax *independent_entities);

/**
 * @brief Returns the score for bounded entities in an algebraic expression.
 * @param  *exp: Algebraic expression.
 * @param  *bound_vars: rax containing the bounded entities.
 * @retval The bound variables score for the expression source and destination.
 */
int AlgebraicExpression_BoundVariableScore(AlgebraicExpression *exp, rax *bound_vars);

/**
 * @brief
 * @note
 * @param  **expf:
 * @param  pos:
 * @param  *exp:
 * @param  *qg:
 * @retval
 */
bool valid_position(AlgebraicExpression **exps, int pos, AlgebraicExpression *exp, QueryGraph *qg);

/**
 * @brief  A method to be used
 * @note
 * @param  *score_ctx:
 * @param  **exps:
 * @param  exp_count:
 * @retval
 */
int score_arrangement(OrderScoreCtx *score_ctx, AlgebraicExpression **exps, uint exp_count);

/**
 * @brief  This method collect independent entities, and the number of their independent occurrences from a filter tree.
 * @note   Indpendent entity - an entity that is the single entity in a predicate filter.
 * @param  *root: Filter tree root.
 * @param  *independent_entities: (in-out-by-ref) rax to hold the independent entities and the the number of their independent occurrences.
 */
void FilterTree_CollectIndependentEntities(const FT_FilterNode *root,
										   rax *independent_entities);
