/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "execution_ctx.h"
#include "../query_ctx.h"

static ExecutionType _GetExecutionTypeFromAST(AST *ast) {
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) return EXECUTION_TYPE_QUERY;
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_CREATE;
	if(root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_DROP;
	assert(false && "Uknown execution type");
}

static ExecutionCtx *_ExecutionCtx_New(AST *ast, ExecutionPlan *plan) {
	ExecutionCtx *exec_ctx = rm_calloc(1, sizeof(ExecutionCtx));
	exec_ctx->ast = ast;
	exec_ctx->exec_plan_template = plan;
	return exec_ctx;
}

ExecutionType ExecutionInformation_FromQuery(const char *query, ExecutionPlan **plan, AST **ast,
											 bool *cached) {
	// Set everything to mark an invalid execution.
	*ast = NULL;
	*plan = NULL;
	*cached = false;
	ExecutionType exec_type = EXECUTION_TYPE_INVALID;

	const char *query_string;
	// Parse and validate parameters only. Extract query string.
	cypher_parse_result_t *params_parse_result = parse_params(query, &query_string);
	// Return if there isn't a parser result.
	if(params_parse_result == NULL) return exec_type;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Cache *cache = GraphContext_GetCache(gc);
	// Check the cache to see if we already have a cached context for this query.
	ExecutionCtx *exec_ctx = Cache_GetValue(cache, query_string);
	if(exec_ctx) {
		// Cache hit - Clone ast and plan. Set the execution type for query execution and indicate a cache hit.
		*ast = AST_ShallowCopy(exec_ctx->ast);
		// Set AST as it is retrived from cache.
		QueryCtx_SetAST(exec_ctx->ast);
		*plan = ExecutionPlan_Clone(exec_ctx->exec_plan_template);
		exec_type = EXECUTION_TYPE_QUERY;
		*cached = true;
		// Set parameters parse result in the execution ast.
		AST_SetParamsParseResult(*ast, params_parse_result);
		return exec_type;
	}
	// No cached execution plan, try to parse the query.
	cypher_parse_result_t *query_parse_result = parse_query(query_string);
	// If no output from the parser, the query is not valid.
	if(!query_parse_result) {
		parse_result_free(params_parse_result);
		return exec_type;
	}

	// Prepare the constructed AST for accesses from the module
	AST *ast_template = AST_Build(query_parse_result);
	exec_type = _GetExecutionTypeFromAST(ast_template);
	*ast = ast_template;
	// Set parameters parse result in the execution ast.
	AST_SetParamsParseResult(*ast, params_parse_result);
	// In case of valid query, create execution plan, cache it, and return a clone.
	if(exec_type == EXECUTION_TYPE_QUERY) {
		ExecutionPlan *exec_plan_template = NewExecutionPlan();
		// Created new valid execution context.
		exec_ctx = _ExecutionCtx_New(AST_ShallowCopy(ast_template), exec_plan_template);
		// Cache execution context.
		Cache_SetValue(cache, query_string, exec_ctx);
		// Clone execution plan.
		*plan = ExecutionPlan_Clone(exec_plan_template);
	}
	return exec_type;
}

void ExecutionCtx_Free(ExecutionCtx *ctx) {
	if(!ctx) return;
	if(ctx->ast) AST_Free(ctx->ast);
	if(ctx->exec_plan_template) ExecutionPlan_Free(ctx->exec_plan_template);
	rm_free(ctx);
}

