/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "query_ctx.h"
#include "util/simple_timer.h"
#include "arithmetic/arithmetic_expression.h"
#include "serializers/graphcontext_type.h"
#include <assert.h>

// GraphContext type as it is registered at Redis.
extern RedisModuleType *GraphContextRedisModuleType;

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

/* rax callback routine for freeing computed parameter values. */
static void _ParameterFreeCallback(void *param_val) {
	AR_EXP_Free(param_val);
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

void QueryCtx_SetGlobalExecutionCtx(CommandCtx *cmd_ctx) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = CommandCtx_GetGraphContext(cmd_ctx);
	ctx->query_data.query = CommandCtx_GetQuery(cmd_ctx);
	ctx->global_exec_ctx.bc = CommandCtx_GetBlockingClient(cmd_ctx);
	ctx->global_exec_ctx.redis_ctx = CommandCtx_GetRedisCtx(cmd_ctx);
	ctx->global_exec_ctx.command_name = CommandCtx_GetCommandName(cmd_ctx);
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->query_data.ast = ast;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->gc = gc;
}

void QueryCtx_SetError(char *err_fmt, ...) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	// An error is already set - free it
	if(ctx->internal_exec_ctx.error) free(ctx->internal_exec_ctx.error);
	// Set the new error
	va_list valist;
	va_start(valist, err_fmt);
	vasprintf(&ctx->internal_exec_ctx.error, err_fmt, valist);
	va_end(valist);
}

void QueryCtx_SetResultSet(ResultSet *result_set) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->internal_exec_ctx.result_set = result_set;
}

void QueryCtx_SetLastWriter(OpBase *last_writer) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ctx->internal_exec_ctx.last_writer = last_writer;
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

ResultSet *QueryCtx_GetResultSet(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return ctx->internal_exec_ctx.result_set;
}

ResultSetStatistics *QueryCtx_GetResultSetStatistics(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	return &ctx->internal_exec_ctx.result_set->stats;
}

void QueryCtx_PrintQuery(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	printf("%s\n", ctx->query_data.query);
}

static void _QueryCtx_ThreadSafeContextLock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) RedisModule_ThreadSafeContextLock(ctx->global_exec_ctx.redis_ctx);
}

static void _QueryCtx_ThreadSafeContextUnlock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) RedisModule_ThreadSafeContextUnlock(ctx->global_exec_ctx.redis_ctx);
}

bool QueryCtx_LockForCommit(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	if(ctx->internal_exec_ctx.locked_for_commit) return true;
	// Lock GIL.
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;
	RedisModuleString *graphID = RedisModule_CreateString(redis_ctx, gc->graph_name,
														  strlen(gc->graph_name));
	_QueryCtx_ThreadSafeContextLock(ctx);
	// Open key and verify.
	RedisModuleKey *key = RedisModule_OpenKey(redis_ctx, graphID, REDISMODULE_WRITE);
	RedisModule_FreeString(redis_ctx, graphID);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		QueryCtx_SetError("Encountered an empty key when opened key %s", ctx->gc->graph_name);
		goto clean_up;
	}
	if(RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
		QueryCtx_SetError("Encountered a non-graph value type when opened key %s", ctx->gc->graph_name);
		goto clean_up;

	}
	if(gc != RedisModule_ModuleTypeGetValue(key)) {
		QueryCtx_SetError("Encountered different graph value when opened key %s", ctx->gc->graph_name);
		goto clean_up;
	}
	ctx->internal_exec_ctx.key = key;
	// Acquire graph write lock.
	Graph_AcquireWriteLock(gc->g);
	ctx->internal_exec_ctx.locked_for_commit = true;

	return true;

clean_up:
	// Free key handle.
	RedisModule_CloseKey(key);
	// Unlock GIL.
	_QueryCtx_ThreadSafeContextUnlock(ctx);
	// If there is a break point for runtime exception, raise it, otherwise return false.
	QueryCtx_RaiseRuntimeException();
	return false;

}

void QueryCtx_UnlockCommit(OpBase *writer_op) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	// Check that the writer_op is entitled to release the lock.
	if(ctx->internal_exec_ctx.last_writer != writer_op) return;
	if(!ctx->internal_exec_ctx.locked_for_commit) return;
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;
	if(ResultSetStat_IndicateModification(ctx->internal_exec_ctx.result_set->stats))
		// Replicate only in case of changes.
		RedisModule_Replicate(redis_ctx, ctx->global_exec_ctx.command_name, "cc!", gc->graph_name,
							  ctx->query_data.query);
	ctx->internal_exec_ctx.locked_for_commit = false;
	// Release graph R/W lock.
	Graph_ReleaseLock(gc->g);
	// Close Key.
	RedisModule_CloseKey(ctx->internal_exec_ctx.key);
	// Unlock GIL.
	_QueryCtx_ThreadSafeContextUnlock(ctx);
}

void QueryCtx_ForceUnlockCommit() {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	if(!ctx->internal_exec_ctx.locked_for_commit) return;
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;
	RedisModule_Log(redis_ctx, "warning",
					"RedisGraph used forced unlocking commit flow for the query %s",
					ctx->query_data.query);
	if(ResultSetStat_IndicateModification(ctx->internal_exec_ctx.result_set->stats))
		// Replicate only in case of changes.
		RedisModule_Replicate(redis_ctx, ctx->global_exec_ctx.command_name, "cc!", gc->graph_name,
							  ctx->query_data.query);
	ctx->internal_exec_ctx.locked_for_commit = false;
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
		raxFreeWithCallback(ctx->query_data.params, _ParameterFreeCallback);
		ctx->query_data.params = NULL;
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}
