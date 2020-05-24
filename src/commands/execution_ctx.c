/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "execution_ctx.h"
#include "../query_ctx.h"

// Invalid single instance or execution context.
static ExecutionCtx invalid_ctx = {0};

static ExecutionType _GetExecutionTypeFromAST(AST *ast) {
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);
	if(root_type == CYPHER_AST_QUERY) return EXECUTION_TYPE_QUERY;
	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_CREATE;
	if(root_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) return EXECUTION_TYPE_INDEX_DROP;
	assert(false && "Uknown execution type");
}

void ExecutionInformation_FromQuery(const char *query, ExecutionPlan **plan, AST **ast,
									ExecutionType *exec_type, bool *cached) {
	// Set everything to mark an invalid execution.
	*plan = NULL;
	*ast = NULL;
	*exec_type = EXECUTION_TYPE_INVALID;
	*cached = false;

	const char *query_string;
	// Parse and validate parameters only. Extract query string.
	cypher_parse_result_t *params_parse_result = parse_params(query, &query_string);
	// Return if there isn't a parser result.
	if(params_parse_result == NULL) return;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Cache *cache = GraphContext_GetCache(gc);
	// Check the LRU cache to see if we already have a context for this query.
	ExecutionCtx *exec_ctx = Cache_GetValue(cache, query_string);
	if(exec_ctx) {
		// Clone ast and plan. Set the execution type for query execution and indicate a cache hit.
		*ast = AST_Clone(exec_ctx->ast);
		*plan = ExecutionPlan_Clone(exec_ctx->exec_plan_template);
		*exec_type = EXECUTION_TYPE_QUERY;
		*cached = true;
		// Set parameters parse result in the execution ast.
		AST_SetParamsParseResult(*ast, params_parse_result);
		// Set AST as it is retrived from cache.
		QueryCtx_SetAST(exec_ctx->ast);
		return;
	}
	// No cache execution plan, try to parse the query.
	cypher_parse_result_t *query_parse_result = parse_query(query_string);
	// If no output from the parser, the query is not valid.
	if(query_parse_result) {
		parse_result_free(params_parse_result);
		return;
	}

	// Prepare the constructed AST for accesses from the module
	AST *ast_template = AST_Build(query_parse_result);
	*exec_type = _GetExecutionTypeFromAST(ast_template);
	ExecutionPlan *exec_plan_template = NULL;
	if(*exec_type == EXECUTION_TYPE_QUERY) exec_plan_template = NewExecutionPlan();
	// Created new valid execution context.
	exec_ctx = rm_calloc(1, sizeof(ExecutionCtx));
	exec_ctx->ast = ast_template;
	// Clone ast.
	*ast = AST_Clone(ast_template);
	// Set parameters parse result in the execution ast.
	AST_SetParamsParseResult(*ast, params_parse_result);
	if(exec_plan_template) {
		exec_ctx->exec_plan_template = exec_plan_template;
		// Cache execution context.
		Cache_SetValue(cache, query_string, exec_ctx);
		// Clone execution plan.
		*plan = ExecutionPlan_Clone(exec_plan_template);
	}
}

void ExecutionCtx_Free(ExecutionCtx *ctx) {
	if(!ctx) return;
	if(ctx->ast) AST_Free(ctx->ast);
	if(ctx->exec_plan_template) ExecutionPlan_Free(ctx->exec_plan_template);
	rm_free(ctx);
}

