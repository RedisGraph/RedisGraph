/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../ast/ast.h"
#include "../execution_plan/execution_plan.h"

/**
 * @brief  Execution type derived from a query
 */
typedef enum {
	EXECUTION_TYPE_QUERY,             // Normal query execution.
	EXECUTION_TYPE_INDEX_CREATE,      // Create index execution.
	EXECUTION_TYPE_INDEX_DROP,        // Drop index execution.
	EXECUTION_TYPE_CONSTRAINT_CREATE, // Create constraint execution.
	EXECUTION_TYPE_CONSTRAINT_DROP    // Drop constraint execution.
} ExecutionType;

/**
 * @brief  A struct for saving execution objects in cache.
 */
typedef struct {
	AST *ast;                   // AST
	bool cached;                // cache hit/miss
	ExecutionPlan *plan;        // execution plan
	ExecutionType exec_type;    // execution type: query, index create/delete
} ExecutionCtx;

/**
 * @brief  Returns the objects and information required for query execution.
 * @note   If the query contains error, a ExecutionCtx struct with the AST and Execution plan objects will be NULL and EXECUTION_TYPE_INVALID is returned.
 * @param  *query: String representing the query.
 * @retval ExecutionCtx populated with the current execution relevant objects.
 */
ExecutionCtx *ExecutionCtx_FromQuery(const char *query);

/**
 * @brief  Clone the execution ctx and return it (shallow copy for the ast, deep copy for the execution plan).
 * @param  *ctx: A pointer to ExecutionCTX struct
 */
ExecutionCtx *ExecutionCtx_Clone(ExecutionCtx *ctx);

/**
 * @brief  Free an ExecutionCTX struct and its inner fields.
 * @param  *ctx: ExecutionCTX struct
 */
void ExecutionCtx_Free(ExecutionCtx *ctx);

