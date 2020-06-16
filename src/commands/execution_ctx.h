/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../ast/ast.h"
#include "../execution_plan/execution_plan.h"

/**
 * @brief  Execution type derived from a query
 */
typedef enum {
	EXECUTION_TYPE_INVALID,         // Execution is not valid due to invalid query.
	EXECUTION_TYPE_QUERY,           // Normal query execution.
	EXECUTION_TYPE_INDEX_CREATE,    // Create index execution.
	EXECUTION_TYPE_INDEX_DROP       // Drop index execution.
} ExecutionType;

/**
 * @brief  A struct for saving execution objects in cache.
 */
typedef struct {
	AST *ast;
	ExecutionPlan *exec_plan_template;
} ExecutionCtx;

/**
 * @brief  Returns the objects and information required for query execution.
 * @note   If the query contains error, the AST and Execution plan objects will be NULL and EXECUTION_TYPE_INVALID is returned.
 * @param  *query: String representing the query.
 * @param  **plan: Out-By-Ref Execution plan object generated from the query.
 * @param  **ast: Out-By-Ref AST object.
 * @param  *cached: Out-By-Ref indication if the ast and execution plan were extracted from cache.
 * @retval Execution type
 */
ExecutionType ExecutionInformation_FromQuery(const char *query, ExecutionPlan **plan, AST **ast,
											 bool *cached);

/**
 * @brief  Free an ExecutionCTX struct
 * @param  *ctx: ExecutionCTX struct
 */
void ExecutionCtx_Free(ExecutionCtx *ctx);

