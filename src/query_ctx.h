/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "ast/ast.h"
#include "redismodule.h"
#include "util/rmalloc.h"
#include "graph/graphcontext.h"
#include "commands/cmd_context.h"
#include "resultset/resultset.h"
#include "execution_plan/ops/op.h"
#include <pthread.h>

extern pthread_key_t _tlsQueryCtxKey;  // thread local storage query context key

typedef struct {
	AST *ast;           // the scoped ast associated with this query
	rax *params;        // query parameters
	const char *query;  // query string
} QueryCtx_QueryData;

typedef struct {
	uint version;               // MVCC version
	double timer[2];            // query execution time tracking
	RedisModuleKey *key;        // saves an open key value, for later extraction and closing
	ResultSet *result_set;      // save the execution result set
	bool locked_for_commit;     // indicates if a call for queryctx_lockforcommit issued before
	OpBase *last_writer;        // the last writer operation which indicates the need for commit
} QueryCtx_InternalExecCtx;

typedef struct {
	RedisModuleCtx *redis_ctx;      // the redis module context
	RedisModuleBlockedClient *bc;   // blocked client
	const char *command_name;       // command name
} QueryCtx_GlobalExecCtx;

typedef struct {
	QueryCtx_QueryData query_data;              // the data related to the query syntax
	QueryCtx_InternalExecCtx internal_exec_ctx; // the data related to internal query execution
	QueryCtx_GlobalExecCtx global_exec_ctx;     // the data rlated to global redis execution
	GraphContext *gc;                           // the GraphContext associated with this query's graph
} QueryCtx;

// instantiate the thread-local QueryCtx on module load
bool QueryCtx_Init(void);

// retrieve this thread's QueryCtx
QueryCtx *QueryCtx_GetQueryCtx();

// set the provided QueryCtx in this thread's storage key
void QueryCtx_SetTLS(QueryCtx *query_ctx);

// NULL-set this thread's storage key
void QueryCtx_RemoveFromTLS();

// start timing query execution
void QueryCtx_BeginTimer(void);

//------------------------------------------------------------------------------
// setters
//------------------------------------------------------------------------------

// sets the global execution context
void QueryCtx_SetGlobalExecutionCtx(CommandCtx *cmd_ctx);

// set the provided AST for access through the QueryCtx
void QueryCtx_SetAST(AST *ast);

// set the provided GraphCtx for access through the QueryCtx
void QueryCtx_SetGraphCtx(GraphContext *gc);

// set the resultset
void QueryCtx_SetResultSet(ResultSet *result_set);

// set the last writer which needs to commit
void QueryCtx_SetLastWriter(OpBase *op);

//------------------------------------------------------------------------------
// getters
//------------------------------------------------------------------------------

// retrieve the AST
AST *QueryCtx_GetAST(void);

// retrueve MVCC version associated with current thread
uint QueryCtx_GetVersion(void);

// retrive the query parameters values map
rax *QueryCtx_GetParams(void);

// retrieve the Graph object
Graph *QueryCtx_GetGraph(void);

// retrieve the GraphCtx
GraphContext *QueryCtx_GetGraphCtx(void);

// retrieve the Redis module context
RedisModuleCtx *QueryCtx_GetRedisModuleCtx(void);

// retrive the resultset
ResultSet *QueryCtx_GetResultSet(void);

// retrive the resultset statistics
ResultSetStatistics *QueryCtx_GetResultSetStatistics(void);

// print the current query
void QueryCtx_PrintQuery(void);

// starts a locking flow before commiting changes in the graph and Redis keyspace
// locking flow is:
// 1. lOCK GIL
// 2. key open with `write` flag
// 3. graph R/W lock with write flag
// Since 2PL protocal is implemented, the method returns true if the it managed to achieve
// locks in this call or a previous call. In case that the locks are already locked, there will
// be no attempt to lock them again
// This method returns false if the key has changed from the current graph,
// and sets the relevant error message
bool QueryCtx_LockForCommit(void);

// starts an ulocking flow and notifies Redis after commiting changes in the graph and Redis keyspace
// the only writer which allow to perform the unlock and commit(replicate) is the last_writer
// the method get an OpBase and compares it to the last writer, if they are equal then the commit
// and unlock flow will start.
// unlocking flow is:
// 1. replicate
// 2. unlock graph R/W lock
// 3. close key
// 4. unlock GIL
void QueryCtx_UnlockCommit(OpBase *writer_op);

//------------------------------------------------------------------------------
// FOR SAFETY ONLY
//------------------------------------------------------------------------------

// this method force releases the locks acquired during commit flow if for
// some reason the last writer op has not invoked QueryCtx_UnlockCommit and Redis is locked
void QueryCtx_ForceUnlockCommit(void);

// compute and return elapsed query execution time
double QueryCtx_GetExecutionTime(void);

// free the allocations within the QueryCtx and reset it for the next query
void QueryCtx_Free(void);

