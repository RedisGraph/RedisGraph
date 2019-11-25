/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_ctx.h"
#include "util/simple_timer.h"
#include <assert.h>
#include "graph/serializers/graphcontext_type.h"

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
	return ctx->internal_exec_ctx.breakpoint;
}

/* Retrieve the error message if the query generated one. */
static char *_QueryCtx_GetError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->internal_exec_ctx.error;
}

bool QueryCtx_Init(void) {
	return (pthread_key_create(&_tlsQueryCtxKey, NULL) == 0);
}

void QueryCtx_Finalize(void) {
	assert(pthread_key_delete(_tlsQueryCtxKey));
}

void QueryCtx_BeginTimer(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx(); // Attempt to retrieve the QueryCtx.
	simple_tic(ctx->internal_exec_ctx.timer); // Start the execution timer.
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

void QueryCtx_SetGlobalExecCtx(CommandCtx *cmd_ctx) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->global_exec_ctx.redis_ctx = CommandCtx_GetRedisCtx(cmd_ctx);
	ctx->global_exec_ctx.bc = CommandCtx_GetBlockingClient(cmd_ctx);
	ctx->global_exec_ctx.command_name = CommandCtx_GetCommandName(cmd_ctx);
	ctx->gc = CommandCtx_GetGraphContext(cmd_ctx);
	ctx->query_data.query = CommandCtx_GetQuery(cmd_ctx);
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->query_data.ast = ast;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetError(char *error) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->internal_exec_ctx.error = error;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	assert(ctx->query_data.ast);
	return ctx->query_data.ast;
}

rax *QueryCtx_GetParams(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	if(!ctx->query_data.params) ctx->query_data.params = raxNew();
	return ctx->query_data.params;
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
	return ctx->global_exec_ctx.redis_ctx;
}

static void _QueryCtx_ThreadSafeContextLock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) RedisModule_ThreadSafeContextLock(ctx->global_exec_ctx.redis_ctx);
}

static void _QueryCtx_ThreadSafeContextUnlock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) RedisModule_ThreadSafeContextUnlock(ctx->global_exec_ctx.redis_ctx);
}

bool QueryCtx_LockForCommit(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	// Lock GIL.
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;
	_QueryCtx_ThreadSafeContextLock(ctx);
	// Open key and verify.
	RedisModuleString *graphID = RedisModule_CreateString(redis_ctx, gc->graph_name,
														  strlen(gc->graph_name));
	RedisModuleKey *key = RedisModule_OpenKey(redis_ctx, graphID, REDISMODULE_WRITE);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		asprintf(&ctx->internal_exec_ctx.error, "Encountered an empty key when opened key %s",
				 ctx->gc->graph_name);

		goto clean_up;
	}
	if(RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
		asprintf(&ctx->internal_exec_ctx.error, "Encountered a non-graph value type when opened key %s",
				 ctx->gc->graph_name);
		goto clean_up;

	}
	if(gc != RedisModule_ModuleTypeGetValue(key)) {
		asprintf(&ctx->internal_exec_ctx.error, "Encountered different graph value when opened key %s",
				 ctx->gc->graph_name);
		goto clean_up;
	}
	RedisModule_FreeString(redis_ctx, graphID);
	ctx->internal_exec_ctx.key = key;
	// Acquire graph write lock.
	Graph_AcquireWriteLock(gc->g);
	return true;

clean_up:
	// Unlock GIL.
	_QueryCtx_ThreadSafeContextUnlock(ctx);
	// If there is a break point for runtime exception, raise it, otherwise return false.
	QueryCtx_RaiseRuntimeException();
	return false;

}

void QueryCtx_NotifyCommitAndUnlock(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;

	// Replicate.
	RedisModule_Replicate(redis_ctx, ctx->global_exec_ctx.command_name, "cc!", gc->graph_name,
						  ctx->query_data.query);
	// Release graph R/W lock.
	Graph_ReleaseLock(gc->g);
	// Close Key.
	RedisModule_CloseKey(ctx->internal_exec_ctx.key);
	// Unlock GIL.
	_QueryCtx_ThreadSafeContextUnlock(ctx);
}

inline bool QueryCtx_EncounteredError(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->internal_exec_ctx.error != NULL;
}

double QueryCtx_GetExecutionTime(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return simple_toc(ctx->internal_exec_ctx.timer) * 1000;
}

void QueryCtx_Free(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();

	if(ctx->internal_exec_ctx.error) {
		free(ctx->internal_exec_ctx.error);
		ctx->internal_exec_ctx.error = NULL;
	}

	if(ctx->internal_exec_ctx.breakpoint) {
		rm_free(ctx->internal_exec_ctx.breakpoint);
		ctx->internal_exec_ctx.breakpoint = NULL;
	}

	if(ctx->query_data.params) {
		raxFree(ctx->query_data.params);
		ctx->query_data.params = NULL;
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}

