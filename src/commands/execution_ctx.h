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
	AST *ast;                   // AST relevant for the current execution context.
	bool cached;                // Indicate if this struct was returned from cache.
	ExecutionPlan *plan;        // Execution plan relevant for the current execution context.
	ExecutionType exec_type;
} ExecutionCtx;

/**
 * @brief  Returns the objects and information required for query execution.
 * @note   If the query contains error, a ExecutionCtx struct with the AST and Execution plan objects will be NULL and EXECUTION_TYPE_INVALID is returned.
 * @param  *query: String representing the query.
 * @retval ExecutionCtx populated with the current execution relevant objects.
 */
ExecutionCtx ExecutionCtx_FromQuery(const char *query);

/**
 * @brief  Free an ExecutionCTX struct
 * @param  *ctx: ExecutionCTX struct
 */
void ExecutionCtx_Free(ExecutionCtx *ctx);

