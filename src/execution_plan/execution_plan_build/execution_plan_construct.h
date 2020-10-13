/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../execution_plan.h"
#include "../../ast/ast.h"
#include "../ops/op_filter.h"

// Build a Skip operation from SKIP clause
OpBase *buildSkipOp(ExecutionPlan *plan, const cypher_astnode_t *skip);

// Build a Limit operation from LIMIT clause
OpBase *buildLimitOp(ExecutionPlan *plan, const cypher_astnode_t *limit);

// Build a procedure call operation from CALL clause
void buildCallOp(AST *ast, ExecutionPlan *plan, const cypher_astnode_t *call_clause);

// Convert a MATCH clause into a sequence of scan and traverse ops
void buildMatchOpTree(ExecutionPlan *plan, AST *ast, const cypher_astnode_t *clause);

// Convert a RETURN clause into a Project or Aggregate op
void buildReturnOps(ExecutionPlan *plan, const cypher_astnode_t *clause);

// Convert a WITH clause into a Project or Aggregate op
void buildWithOps(ExecutionPlan *plan, const cypher_astnode_t *clause);

// Convert a MERGE clause into a matching traversal and creation op tree
void buildMergeOp(ExecutionPlan *plan, AST *ast, const cypher_astnode_t *clause, GraphContext *gc);

// Reduce a filter operation into an apply operation
void ExecutionPlan_ReduceFilterToApply(ExecutionPlan *plan, OpFilter *filter);

// Place filter ops at the appropriate positions within the op tree
void ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan, OpBase *root, const OpBase *recurse_limit,
								  FT_FilterNode *ft);

// Convert a clause into the appropriate sequence of ops
void ExecutionPlanSegment_ConvertClause(GraphContext *gc, AST *ast, ExecutionPlan *plan,
										const cypher_astnode_t *clause);

