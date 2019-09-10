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
	return pthread_getspecific(_tlsQueryCtxKey);
}

bool QueryCtx_Init(void) {
	return (pthread_key_create(&_tlsQueryCtxKey, NULL) == 0);
}

void QueryCtx_Finalize(void) {
	assert(pthread_key_delete(_tlsQueryCtxKey));
}

void QueryCtx_Begin(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx(); // Attempt to retrieve the QueryCtx.
	if(!ctx) {
		// Set a new thread-local QueryCtx if one has not been created.
		ctx = rm_calloc(1, sizeof(QueryCtx));
		pthread_setspecific(_tlsQueryCtxKey, ctx);
	}
	// Allocate space for an exception-handling breakpoint if necessary.
	if(ctx->breakpoint == NULL) ctx->breakpoint = rm_malloc(sizeof(jmp_buf));
	simple_tic(ctx->timer); // Start the execution timer.
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->ast = ast;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetError(char *error) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->error = error;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->ast);
	return ctx->ast;
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

jmp_buf *QueryCtx_GetExceptionHandler(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->breakpoint);
	return ctx->breakpoint;
}

char *QueryCtx_GetError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->error;
}

inline bool QueryCtx_ShouldFreeExceptionCause(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->free_cause;
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

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}

