/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "execution_ctx.h"
#include "RG.h"
#include "../errors.h"
#include "../query_ctx.h"
#include "../execution_plan/execution_plan_clone.h"

static ExecutionType _GetExecutionTypeFromAST(AST *ast) {
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) return EXECUTION_TYPE_QUERY;
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_CREATE;
	if(root_type == CYPHER_AST_CREATE_PATTERN_PROPS_INDEX) return EXECUTION_TYPE_INDEX_CREATE;
	if(root_type == CYPHER_AST_DROP_PROPS_INDEX) return EXECUTION_TYPE_INDEX_DROP;
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
	if(ErrorCtx_EncounteredError() || query_parse_result == NULL) {
		parse_result_free(query_parse_result);
		query_parse_result = NULL;
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

	// Parse and validate parameters only. Extract query string.
	// Return invalid execution context if there isn't a parser result.
	cypher_parse_result_t *params_parse_result = parse_params(query,
															  &query_string);

	// Parameter parsing failed, return NULL.
	if(params_parse_result == NULL) return NULL;

	// query included only params e.g. 'cypher a=1' was provided
	if(strlen(query_string) == 0) {
		parse_result_free(params_parse_result);
		ErrorCtx_SetError("Error: empty query.");
		return NULL;
	}
	// update query context with the query without params
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ctx->query_data.query_no_params = query_string;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Cache *cache = GraphContext_GetCache(gc);

	// Check the cache to see if we already have a cached context for this query.
	ret = Cache_GetValue(cache, query_string);
	if(ret) {
		// Set parameters parse result in the execution ast.
		AST_SetParamsParseResult(ret->ast, params_parse_result);
		ret->cached = true;
		return ret;
	}

	// No cached execution plan, try to parse the query.
	AST *ast = _ExecutionCtx_ParseAST(query_string, params_parse_result);
	// if query parsing failed, return NULL
	if(!ast) {
		// if no error has been set, emit one now
		if(!ErrorCtx_EncounteredError()) {
			ErrorCtx_SetError("Error: could not parse query");
		}
		return NULL;
	}

	ExecutionType exec_type = _GetExecutionTypeFromAST(ast);
	// In case of valid query, create execution plan, and cache it and the AST.
	if(exec_type == EXECUTION_TYPE_QUERY) {
		ExecutionPlan *plan = NewExecutionPlan();

		// TODO: there must be a better way to understand if the execution-plan
		// was constructed correctly,
		// maybe free the plan within NewExecutionPlan, if error was encountered
		// and return NULL ?
		if(ErrorCtx_EncounteredError()) {
			// Encountered an error in ExecutionPlan construction,
			// clean up and return NULL.
			AST_Free(ast);
			ExecutionPlan_Free(plan);
			return NULL;
		}
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

