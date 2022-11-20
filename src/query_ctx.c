/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "query_ctx.h"
#include "RG.h"
#include "errors.h"
#include "util/simple_timer.h"
#include "arithmetic/arithmetic_expression.h"
#include "serializers/graphcontext_type.h"
#include "undo_log/undo_log.h"

// GraphContext type as it is registered at Redis.
extern RedisModuleType *GraphContextRedisModuleType;

pthread_key_t _tlsQueryCtxKey;  // Thread local storage query context key.

// retrieve or instantiate new QueryCtx
static inline QueryCtx *_QueryCtx_GetCreateCtx(void) {
	QueryCtx *ctx = pthread_getspecific(_tlsQueryCtxKey);
	if(!ctx) {
		// Set a new thread-local QueryCtx if one has not been created.
		ctx = rm_calloc(1, sizeof(QueryCtx));
		ctx->undo_log = UndoLog_New();
		pthread_setspecific(_tlsQueryCtxKey, ctx);
	}
	return ctx;
}

// retrieve QueryCtx, return NULL if one does not exist
static inline QueryCtx *_QueryCtx_GetCtx(void) {
	QueryCtx *ctx = pthread_getspecific(_tlsQueryCtxKey);
	return ctx;
}

/* rax callback routine for freeing computed parameter values. */
static void _ParameterFreeCallback(void *param_val) {
	AR_EXP_Free(param_val);
}

bool QueryCtx_Init(void) {
	return (pthread_key_create(&_tlsQueryCtxKey, NULL) == 0);
}

inline QueryCtx *QueryCtx_GetQueryCtx() {
	return _QueryCtx_GetCreateCtx();
}

inline void QueryCtx_SetTLS(QueryCtx *query_ctx) {
	pthread_setspecific(_tlsQueryCtxKey, query_ctx);
}

inline void QueryCtx_RemoveFromTLS() {
	pthread_setspecific(_tlsQueryCtxKey, NULL);
}

void QueryCtx_BeginTimer(void) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx(); // Attempt to retrieve the QueryCtx.
	simple_tic(ctx->internal_exec_ctx.timer); // Start the execution timer.
}

void QueryCtx_SetGlobalExecutionCtx(CommandCtx *cmd_ctx) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	ctx->gc = CommandCtx_GetGraphContext(cmd_ctx);
	ctx->query_data.query = CommandCtx_GetQuery(cmd_ctx);
	ctx->global_exec_ctx.bc = CommandCtx_GetBlockingClient(cmd_ctx);
	ctx->global_exec_ctx.redis_ctx = CommandCtx_GetRedisCtx(cmd_ctx);
	ctx->global_exec_ctx.command_name = CommandCtx_GetCommandName(cmd_ctx);
}

void QueryCtx_SetAST(AST *ast) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	ctx->query_data.ast = ast;
}

void QueryCtx_SetGraphCtx(GraphContext *gc) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	ctx->gc = gc;
}

void QueryCtx_SetResultSet(ResultSet *result_set) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	ctx->internal_exec_ctx.result_set = result_set;
}

void QueryCtx_SetParams(rax *params) {
	ASSERT(params != NULL);
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	ctx->query_data.params = params;
}

AST *QueryCtx_GetAST(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);
	return ctx->query_data.ast;
}

rax *QueryCtx_GetParams(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);
	return ctx->query_data.params;
}

GraphContext *QueryCtx_GetGraphCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx && ctx->gc);
	return ctx->gc;
}

Graph *QueryCtx_GetGraph(void) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	return gc->g;
}

RedisModuleCtx *QueryCtx_GetRedisModuleCtx(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);
	return ctx->global_exec_ctx.redis_ctx;
}

ResultSet *QueryCtx_GetResultSet(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);
	return ctx->internal_exec_ctx.result_set;
}

ResultSetStatistics *QueryCtx_GetResultSetStatistics(void) {
	ResultSetStatistics  *stats       =  NULL;
	ResultSet            *result_set  =  QueryCtx_GetResultSet();
	if(result_set) stats = &result_set->stats;
	return stats;
}

void QueryCtx_PrintQuery(void) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	printf("%s\n", ctx->query_data.query);
}

static void _QueryCtx_ThreadSafeContextLock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) {
		RedisModule_ThreadSafeContextLock(ctx->global_exec_ctx.redis_ctx);
	}
}

static void _QueryCtx_ThreadSafeContextUnlock(QueryCtx *ctx) {
	if(ctx->global_exec_ctx.bc) RedisModule_ThreadSafeContextUnlock(ctx->global_exec_ctx.redis_ctx);
}

bool QueryCtx_LockForCommit(void) {
	QueryCtx *ctx = _QueryCtx_GetCreateCtx();
	if(ctx->internal_exec_ctx.locked_for_commit) return true;

	// lock GIL
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;
	GraphContext *gc = ctx->gc;
	RedisModuleString *graphID = RedisModule_CreateString(redis_ctx, gc->graph_name,
														  strlen(gc->graph_name));
	_QueryCtx_ThreadSafeContextLock(ctx);

	// open key and verify
	RedisModuleKey *key = RedisModule_OpenKey(redis_ctx, graphID, REDISMODULE_WRITE);
	RedisModule_FreeString(redis_ctx, graphID);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		ErrorCtx_SetError("Encountered an empty key when opened key %s", ctx->gc->graph_name);
		goto clean_up;
	}
	if(RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
		ErrorCtx_SetError("Encountered a non-graph value type when opened key %s", ctx->gc->graph_name);
		goto clean_up;

	}
	if(gc != RedisModule_ModuleTypeGetValue(key)) {
		ErrorCtx_SetError("Encountered different graph value when opened key %s", ctx->gc->graph_name);
		goto clean_up;
	}
	ctx->internal_exec_ctx.key = key;

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);
	ctx->internal_exec_ctx.locked_for_commit = true;

	return true;

clean_up:
	// free key handle
	RedisModule_CloseKey(key);

	// unlock GIL
	_QueryCtx_ThreadSafeContextUnlock(ctx);

	// if there is a break point for runtime exception, raise it, otherwise return false
	ErrorCtx_RaiseRuntimeException(NULL);
	return false;
}

static void _QueryCtx_UnlockCommit(QueryCtx *ctx) {
	GraphContext *gc = ctx->gc;

	ctx->internal_exec_ctx.locked_for_commit = false;
	// release graph R/W lock
	Graph_ReleaseLock(gc->g);

	// close Key
	RedisModule_CloseKey(ctx->internal_exec_ctx.key);

	// unlock GIL
	_QueryCtx_ThreadSafeContextUnlock(ctx);
}

// replicate command
void QueryCtx_Replicate
(
	QueryCtx *ctx
) {
	ASSERT(ctx != NULL);

	GraphContext   *gc        = ctx->gc;
	RedisModuleCtx *redis_ctx = ctx->global_exec_ctx.redis_ctx;

	// replicate
	RedisModule_Replicate(redis_ctx, ctx->global_exec_ctx.command_name,
			"cc!", gc->graph_name, ctx->query_data.query);
}

void QueryCtx_UnlockCommit() {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	if(!ctx) return;

	// already unlocked?
	if(!ctx->internal_exec_ctx.locked_for_commit) return;

	_QueryCtx_UnlockCommit(ctx);
}

double QueryCtx_GetExecutionTime(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);
	return simple_toc(ctx->internal_exec_ctx.timer) * 1000;
}

void QueryCtx_Free(void) {
	QueryCtx *ctx = _QueryCtx_GetCtx();
	ASSERT(ctx != NULL);

	UndoLog_Free(ctx->undo_log);

	if(ctx->query_data.params) {
		raxFreeWithCallback(ctx->query_data.params, _ParameterFreeCallback);
		ctx->query_data.params = NULL;
	}

	rm_free(ctx);
	// NULL-set the context for reuse the next time this thread receives a query
	QueryCtx_RemoveFromTLS();
}

