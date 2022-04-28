/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

extern pthread_key_t _tlsQueryCtxKey;  // Thread local storage query context key

// define a RWLock status
typedef enum {
	LOCK_STATUS_UNLOCKED = 0,  // lock isn't acquired
	LOCK_STATUS_READLOCKED  ,  // lock is acquired for reading
	LOCK_STATUS_WRITELOCKED ,  // lock is acquired for writing
} LockStatus;

typedef struct {
	AST *ast;                     // the scoped AST associated with this query
	rax *params;                  // query parameters
	const char *query;            // query string
	const char *query_no_params;  // query string without parameters part
} QueryCtx_QueryData;

typedef struct {
	double timer[2];            // query execution time tracking
	RedisModuleKey *key;        // saves an open key value, for later extraction and closing
	ResultSet *result_set;      // save the execution result set
	bool locked_for_commit;     // indicates if a call for QueryCtx_LockForCommit issued before
	OpBase *last_writer;        // the last writer operation which indicates the need for commit
	LockStatus lock_status;     // RWLock status
} QueryCtx_InternalExecCtx;

typedef struct {
	RedisModuleCtx *redis_ctx;      // The Redis module context.
	RedisModuleBlockedClient *bc;   // Blocked client.
	const char *command_name;       // Command name.
} QueryCtx_GlobalExecCtx;

typedef struct {
	QueryCtx_QueryData query_data;              // The data related to the query syntax.
	QueryCtx_InternalExecCtx internal_exec_ctx; // The data related to internal query execution.
	QueryCtx_GlobalExecCtx global_exec_ctx;     // The data rlated to global redis execution.
	GraphContext *gc;                           // The GraphContext associated with this query's graph.
} QueryCtx;

/* Instantiate the thread-local QueryCtx on module load. */
bool QueryCtx_Init(void);

/* Retrieve this thread's QueryCtx. */
QueryCtx *QueryCtx_GetQueryCtx();

/* Set the provided QueryCtx in this thread's storage key. */
void QueryCtx_SetTLS(QueryCtx *query_ctx);

/* Null-set this thread's storage key. */
void QueryCtx_RemoveFromTLS();

/* Start timing query execution. */
void QueryCtx_BeginTimer(void);

//------------------------------------------------------------------------------
// setters
//------------------------------------------------------------------------------

// Sets the global execution context
void QueryCtx_SetGlobalExecutionCtx(CommandCtx *cmd_ctx);
// Set the provided AST for access through the QueryCtx
void QueryCtx_SetAST(AST *ast);
// Set the provided GraphCtx for access through the QueryCtx
void QueryCtx_SetGraphCtx(GraphContext *gc);
// Set the resultset
void QueryCtx_SetResultSet(ResultSet *result_set);
// Set the parameters map
void QueryCtx_SetParams(rax *params);
// Set the last writer which needs to commit
void QueryCtx_SetLastWriter(OpBase *op);
// set lock status
void QueryCtx_SetLockStatus(LockStatus status);

//------------------------------------------------------------------------------
// getters
//------------------------------------------------------------------------------

// retrieve the AST
AST *QueryCtx_GetAST(void);
// retrieve the query parameters values map
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
// retrieve the lock status: unlocked, read-locked, write-locked
LockStatus QueryCtx_LockStatus(void);

// print the current query
void QueryCtx_PrintQuery(void);

/* Starts a locking flow before commiting changes in the graph and Redis keyspace.
 * Locking flow is:
 * 1. LOCK GIL
 * 2. Key open with `write` flag
 * 3. Graph R/W lock with write flag
 * Since 2PL protocal is implemented, the method returns true if the it managed to achieve
 * locks in this call or a previous call. In case that the locks are already locked, there will
 * be no attempt to lock them again.
 * This method returns false if the key has changed from the current graph,
 * and sets the relevant error message. */
bool QueryCtx_LockForCommit(void);

/* Starts an ulocking flow and notifies Redis after commiting changes in the graph and Redis keyspace.
 * The only writer which allow to perform the unlock and commit(replicate) is the last_writer.
 * The method get an OpBase and compares it to the last writer, if they are equal then the commit
 * and unlock flow will start.
 * Unlocking flow is:
 * 1. Replicate.
 * 2. Unlock graph R/W lock
 * 3. Close key
 * 4. Unlock GIL */
void QueryCtx_UnlockCommit(OpBase *writer_op);

//------------------------------------------------------------------------------
// safety mechanism
//------------------------------------------------------------------------------
// This method force releases the locks acquired during commit flow
// if for some reason the last writer op has not invoked QueryCtx_UnlockCommit
// and Redis is locked
void QueryCtx_ForceUnlockCommit(void);

/* Compute and return elapsed query execution time. */
double QueryCtx_GetExecutionTime(void);

/* Free the allocations within the QueryCtx and reset it for the next query. */
void QueryCtx_Free(void);

