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

static ExecutionType _GetExecutionTypeFromAST
(
	const AST *ast
) {
	const cypher_astnode_type_t root_type = cypher_astnode_type(ast->root);

	if(root_type == CYPHER_AST_QUERY) {
		return EXECUTION_TYPE_QUERY;
	}

	if(root_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX) {
		return EXECUTION_TYPE_INDEX_CREATE;
	}

	if(root_type == CYPHER_AST_CREATE_PATTERN_PROPS_INDEX) {
		return EXECUTION_TYPE_INDEX_CREATE;
	}

	if(root_type == CYPHER_AST_DROP_PROPS_INDEX) {
		return EXECUTION_TYPE_INDEX_DROP;
	}

	ASSERT(false && "Unknown execution type");
	return 0;
}

static AST *_ExecutionCtx_ParseAST
(
	const char *q_str
) {
	cypher_parse_result_t *query_parse_result = parse_query(q_str);
	// if no output from the parser, the query is not valid
	if(ErrorCtx_EncounteredError() || query_parse_result == NULL) {
		parse_result_free(query_parse_result);
		return NULL;
	}

	// prepare the constructed AST
	AST *ast = AST_Build(query_parse_result);

	return ast;
}

static ExecutionCtx *_ExecutionCtx_New
(
	AST *ast,
	ExecutionPlan *plan,
	ExecutionType exec_type
) {
	// a new execution context must be parameters free
	ASSERT(ast->params_parse_result == NULL);

	ExecutionCtx *exec_ctx = rm_malloc(sizeof(ExecutionCtx));

	exec_ctx->ast       = ast;
	exec_ctx->plan      = plan;
	exec_ctx->cached    = false;
	exec_ctx->exec_type = exec_type;

	return exec_ctx;
}

// clone the execution ctx and return a shallow copy for the ast
// deep copy for the execution plan
ExecutionCtx *ExecutionCtx_Clone
(
	const ExecutionCtx *ctx  // execution context to clone
) {
	ExecutionCtx *clone = rm_malloc(sizeof(ExecutionCtx));

	clone->ast = AST_ShallowCopy(ctx->ast);

	// set the AST copy in thread local storage
	QueryCtx_SetAST(clone->ast);

	clone->plan      = ExecutionPlan_Clone(ctx->plan);
	clone->cached    = ctx->cached;
	clone->exec_type = ctx->exec_type;

	return clone;
}

// returns the objects and information required for query execution
// if the query contains error, a ExecutionCtx struct with the AST
// and Execution plan objects will be NULL
// and EXECUTION_TYPE_INVALID is returned
// returns ExecutionCtx populated with the current execution relevant objects
ExecutionCtx *ExecutionCtx_FromQuery
(
	const char *q  // string representing the query
) {
	ASSERT(q != NULL);

	ExecutionCtx *ret;
	const char *q_str;  // query string excluding query parameters

	// parse and validate parameters only
	// extract query string
	// return invalid execution context if there isn't a parser result
	cypher_parse_result_t *params_parse_result = parse_params(q, &q_str);

	// parameter parsing failed, return NULL
	if(params_parse_result == NULL) {
		return NULL;
	}

	// query included only params e.g. 'cypher a=1' was provided
	if(unlikely(strlen(q_str) == 0)) {
		parse_result_free(params_parse_result);
		ErrorCtx_SetError("Error: empty query.");
		return NULL;
	}

	// update query context with the query without params
	QueryCtx *ctx = QueryCtx_GetQueryCtx();
	ctx->query_data.query_no_params = q_str;

	// get cache
	Cache *cache = GraphContext_GetCache(QueryCtx_GetGraphCtx());

	// see if we already have a cached execution-ctx for given query
	ret = Cache_GetValue(cache, q_str);
	if(ret != NULL) {
		// set query parameters in the execution AST
		AST_SetParamsParseResult(ret->ast, params_parse_result);
		ret->cached = true;  // mark cached execution
		return ret;
	}

	// no cached execution plan, try to parse the query
	AST *ast = _ExecutionCtx_ParseAST(q_str);

	// if query parsing failed, return NULL
	if(ast == NULL) {
		// if no error has been set, emit one now
		if(!ErrorCtx_EncounteredError()) {
			ErrorCtx_SetError("Error: could not parse query");
		}
		if(params_parse_result != NULL) {
			parse_result_free(params_parse_result);
		}
		return NULL;
	}

	ExecutionType exec_type = _GetExecutionTypeFromAST(ast);
	// in case of valid query
	// create execution plan, and cache it and the AST
	if(exec_type == EXECUTION_TYPE_QUERY) {
		//----------------------------------------------------------------------
		// build execution-plan
		//----------------------------------------------------------------------
		ExecutionPlan *plan = NewExecutionPlan();

		// TODO: there must be a better way to understand if the execution-plan
		// was constructed correctly,
		// maybe free the plan within NewExecutionPlan, if error was encountered
		// and return NULL ?
		if(ErrorCtx_EncounteredError()) {
			// failed to construct plan
			// clean up and return NULL
			AST_Free(ast);
			ExecutionPlan_Free(plan);
			parse_result_free(params_parse_result);
			return NULL;
		}

		ExecutionCtx *exec_ctx = _ExecutionCtx_New(ast, plan, exec_type);
		ret = Cache_SetGetValue(cache, q_str, exec_ctx);

		// set query parameters in the execution AST
		AST_SetParamsParseResult(ret->ast, params_parse_result);
	} else {
		ret = _ExecutionCtx_New(ast, NULL, exec_type);
	}

	return ret;
}

// free an ExecutionCTX struct and its inner fields
void ExecutionCtx_Free
(
	ExecutionCtx *ctx  // execution context to free
) {
	if(ctx == NULL) {
		return;
	}

	if(ctx->plan != NULL) {
		ExecutionPlan_Free(ctx->plan);
	}

	if(ctx->ast != NULL) {
		AST_Free(ctx->ast);
	}

	rm_free(ctx);
}

