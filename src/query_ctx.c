/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_ctx.h"
#include "util/simple_timer.h"
#include <assert.h>

pthread_key_t _tlsQueryCtxKey;  // Thread local storage query context key.

static inline QueryCtx *_QueryCtx_GetCtx(void) {
	QueryCtx *ctx = pthread_getspecific(_tlsQueryCtxKey);
	if(!ctx) {
		// Set a new thread-local QueryCtx if one has not been created.
		ctx = rm_calloc(1, sizeof(QueryCtx));
		pthread_setspecific(_tlsQueryCtxKey, ctx);
	}
	return ctx;
}

/* Retrieve the exception handler. */
static jmp_buf *_QueryCtx_GetExceptionHandler(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->breakpoint;
}

/* Retrieve the error message if the query generated one. */
static char *_QueryCtx_GetError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->error;
}

bool QueryCtx_Init(void) {
	return (pthread_key_create(&_tlsQueryCtxKey, NULL) == 0);
}

void QueryCtx_Finalize(void) {
	assert(pthread_key_delete(_tlsQueryCtxKey));
}

void QueryCtx_BeginTimer(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx(); // Attempt to retrieve the QueryCtx.
	simple_tic(ctx->timer); // Start the execution timer.
}

/* An error was encountered during evaluation, and has already been set in the QueryCtx.
 * If an exception handler has been set, exit this routine and return to
 * the point on the stack where the handler was instantiated.  */
void QueryCtx_RaiseRuntimeException(void) {
	jmp_buf *env = _QueryCtx_GetExceptionHandler();
	// If the exception handler hasn't been set, this function returns to the caller,
	// which will manage its own freeing and error reporting.
	if(env) longjmp(*env, 1);
}

void QueryCtx_EmitException(void) {
	char *error = _QueryCtx_GetError();
	if(error) {
		RedisModuleCtx *ctx = QueryCtx_GetRedisModuleCtx();
		RedisModule_ReplyWithError(ctx, error);
	}
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->ast = ast;
}

void QueryCtx_SetParams(rax *params) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->params = params;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetError(char *error) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->error = error;
}

void QueryCtx_SetRedisModuleCtx(RedisModuleCtx *redisctx) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->redisctx = redisctx;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->ast);
	return ctx->ast;
}

rax *QueryCtx_GetParams(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->params;
}

GraphContext *QueryCtx_GetGraphCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->gc);
	return ctx->gc;
}

Graph *QueryCtx_GetGraph(void) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	return gc->g;
}

RedisModuleCtx *QueryCtx_GetRedisModuleCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->redisctx;
}

inline bool QueryCtx_EncounteredError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->error != NULL;
}

double QueryCtx_GetExecutionTime(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return simple_toc(ctx->timer) * 1000;
}

void QueryCtx_Free(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();

	if(ctx->error) {
		free(ctx->error);
		ctx->error = NULL;
	}

	if(ctx->breakpoint) {
		rm_free(ctx->breakpoint);
		ctx->breakpoint = NULL;
	}

	if(ctx->params) {
		raxFree(ctx->params);
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}

