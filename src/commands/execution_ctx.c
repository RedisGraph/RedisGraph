/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "execution_ctx.h"
#include "RG.h"
#include "../query_ctx.h"
#include "../execution_plan/execution_plan_clone.h"

static ExecutionType _GetExecutionTypeFromAST(AST *ast) {
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) return EXECUTION_TYPE_QUERY;
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_CREATE;
	if(root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_DROP;
	ASSERT(false && "Unknown execution type");
	return 0;
}

static ExecutionCtx *_ExecutionCtx_New(AST *ast, ExecutionPlan *plan,
  		ExecutionType exec_type) {
	ExecutionCtx *exec_ctx = rm_malloc(sizeof(ExecutionCtx));

	exec_ctx->ast       = ast;
	exec_ctx->plan      = plan;
	exec_ctx->cached    = false;
	exec_ctx->exec_type = exec_type;

	return exec_ctx;
}

ExecutionCtx *ExecutionCtx_Clone(ExecutionCtx *orig) {
	ExecutionCtx *execution_ctx = rm_malloc(sizeof(ExecutionCtx));

	execution_ctx->ast = AST_ShallowCopy(orig->ast);
	// set the AST copy in thread local storage
	QueryCtx_SetAST(execution_ctx->ast);

	execution_ctx->plan      = ExecutionPlan_Clone(orig->plan);
	execution_ctx->cached    = orig->cached;
	execution_ctx->exec_type = orig->exec_type;

	return execution_ctx;
}

static AST *_ExecutionCtx_ParseAST(const char *query_string,
								   cypher_parse_result_t *params_parse_result) {
	cypher_parse_result_t *query_parse_result = parse_query(query_string);
	// If no output from the parser, the query is not valid.
	if(!query_parse_result) {
		parse_result_free(params_parse_result);
		return NULL;
	}

	// Prepare the constructed AST.
	AST *ast = AST_Build(query_parse_result);
	// Set parameters parse result in the execution ast.
	AST_SetParamsParseResult(ast, params_parse_result);
	return ast;
}

ExecutionCtx *ExecutionCtx_FromQuery(const char *query) {
	ASSERT(query != NULL);

	ExecutionCtx *ret;
	const char *query_string;

	// Have an invalid ctx for errors.
	ExecutionCtx *invalid_ctx = _ExecutionCtx_New(NULL, NULL,
			EXECUTION_TYPE_INVALID);

	// Parse and validate parameters only. Extract query string.
	// Return invalid execution context if there isn't a parser result.
	cypher_parse_result_t *params_parse_result = parse_params(query,
			&query_string);

	if(params_parse_result == NULL) return invalid_ctx;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Cache *cache = GraphContext_GetCache(gc);

	// Check the cache to see if we already have a cached context for this query.
	ret = Cache_GetValue(cache, query_string);
	if(ret) {
		ExecutionCtx_Free(invalid_ctx);
		// Set parameters parse result in the execution ast.
		AST_SetParamsParseResult(ret->ast, params_parse_result);
		ret->cached = true;
		return ret;
	}

	// No cached execution plan, try to parse the query.
	AST *ast = _ExecutionCtx_ParseAST(query_string, params_parse_result);
	// Invalid query, return invalid execution context.
	if(!ast) return invalid_ctx;

	ExecutionCtx_Free(invalid_ctx);
	ExecutionType exec_type = _GetExecutionTypeFromAST(ast);
	// In case of valid query, create execution plan, and cache it and the AST.
	if(exec_type == EXECUTION_TYPE_QUERY) {
		ExecutionPlan *plan = NewExecutionPlan();
		ExecutionCtx *exec_ctx_to_cache = _ExecutionCtx_New(ast, plan,
		  exec_type);
		ExecutionCtx *exec_ctx_from_cache = Cache_SetGetValue(cache,
		  query_string, exec_ctx_to_cache);
		return exec_ctx_from_cache;
	} else {
		return _ExecutionCtx_New(ast, NULL, exec_type);
	}
}

void ExecutionCtx_Free(ExecutionCtx *ctx) {
	if(ctx == NULL) return;
	if(ctx->plan != NULL) ExecutionPlan_Free(ctx->plan);
	if(ctx->ast != NULL) AST_Free(ctx->ast);

	rm_free(ctx);
}

