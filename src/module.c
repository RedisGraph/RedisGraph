/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "redismodule.h"

#include "graph/graph.h"
#include "graph/graph_type.h"
#include "graph/edge.h"

#include "value.h"
#include "delete_graph.h"
#include "query_executor.h"
#include "bulk_insert/bulk_insert.h"
#include "bulk_insert/bulk_insert_context.h"
#include "arithmetic/arithmetic_expression.h"

#include "util/simple_timer.h"
#include "util/triemap/triemap_type.h"
#include "util/thpool/thpool.h"

#include "rmutil/util.h"
#include "rmutil/vector.h"

#include "parser/ast.h"
#include "parser/grammar.h"
#include "parser/parser_common.h"

#include "stores/store.h"
#include "stores/store_type.h"

#include "grouping/group_cache.h"
#include "arithmetic/agg_funcs.h"

#include "resultset/record.h"
#include "resultset/resultset.h"

#include "execution_plan/execution_plan.h"

#include "index/index.h"
#include "index/index_type.h"

/* Thread pool. */
static threadpool _thpool = NULL;

/* Read Write lock */
static pthread_rwlock_t _rwlock;

/* Set up thread pool,
 * number of threads within pool should be
 * the number of available hyperthreads.
 * Returns 1 if thread pool initialized, 0 otherwise. */
int _Setup_ThreadPOOL() {
    // Create thread pool.
    int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
    if(CPUCount == -1) return 0;

    _thpool = thpool_init(CPUCount);
    if(_thpool == NULL) return 0;

    return 1;
}

//------------------------------------------------------------------------------
// Read/Write lock
//------------------------------------------------------------------------------

// TODO: Move lock functions to a suitable location.
/* Acquire lock for read, multiple threads might be reading concurrently. */
void _MGraph_AcquireReadLock() {
    pthread_rwlock_rdlock(&_rwlock);
}

/* Acquire lock for write, only a single thread can perform work
 * while holding a write lock,
 * In addition we're also acquiring Redis global lock
 * this is to prevent concurent writing while redis
 * might be performing replication or persists data to disk. */
void _MGraph_AcquireWriteLock(RedisModuleCtx *ctx) {
    pthread_rwlock_wrlock(&_rwlock);
    RedisModule_ThreadSafeContextLock(ctx);
}

// Release read/write lock.
void _MGraph_ReleaseLock(RedisModuleCtx *ctx) {
    pthread_rwlock_unlock(&_rwlock);
    /* Release Redis global lock,
     * this should only have an effect when the read/write lock
     * was acquired for writing. */
    RedisModule_ThreadSafeContextUnlock(ctx);
}

void _index_operation(RedisModuleCtx *ctx, const char *graphName, Graph *g, AST_IndexNode *indexNode) {
  /* Set up nested array response for index creation and deletion.
   * As we need no result set, there is only one top-level element, for statistics.
   * Index_Create or Index_Delete will enqueue one string response
   * to indicate the success of the operation, and the query runtime will be appended
   * after this call returns. */
  RedisModule_ReplyWithArray(ctx, 1);
  RedisModule_ReplyWithArray(ctx, 2);

  int ret;
  switch(indexNode->operation) {
    case CREATE_INDEX:
      ret = Index_Create(ctx, graphName, g, indexNode->label, indexNode->property);
      if (ret == INDEX_OK) {
        RedisModule_ReplyWithSimpleString(ctx, "Added 1 index.");
      } else {
        RedisModule_ReplyWithSimpleString(ctx, "(no changes, no records)");
      }
      break;
    case DROP_INDEX:
      ret = Index_Delete(ctx, graphName, indexNode->label, indexNode->property);
      if (ret == INDEX_OK) {
        RedisModule_ReplyWithSimpleString(ctx, "Removed 1 index.");
      } else {
        char *reply;
        asprintf(&reply, "ERR Unable to drop index on :%s(%s): no such index.", indexNode->label, indexNode->property);
        RedisModule_ReplyWithError(ctx, reply);
        free(reply);
      }
      break;
    default:
      assert(0);
  }
}

/* Retrieve graph stored within Redis under graph_name key,
 * If specified key does not exists a new graph object is created and stored
 * in case graph already exists NULL is returned. */
Graph *_MGraph_CreateGraph(RedisModuleCtx *ctx, RedisModuleString *graph_name) {
    Graph *g = NULL;
    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);

    // Key does not exists, create a new graph and store within Redis keyspace.
	if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		g = Graph_New(GRAPH_DEFAULT_NODE_CAP);
		RedisModule_ModuleTypeSetValue(key, GraphRedisModuleType, g);
	} else {
        RedisModule_ReplyWithError(ctx, "Can not create graph, graph ID is used by some other key.");
    }

    RedisModule_CloseKey(key);
    return g;
}

void _MGraph_Query(void *args) {
    QueryContext *qctx = (QueryContext*)args;
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(qctx->bc);
    AST_Query* ast = qctx->ast;
    const char *graph_name = RedisModule_StringPtrLen(qctx->graphName, NULL);

    ModifyAST(ctx, ast, graph_name);

    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        free(reason);
        goto cleanup;
    }

    // If this is a write query, acquire write lock.
    if(AST_ReadOnly(ast)) _MGraph_AcquireReadLock();
    else _MGraph_AcquireWriteLock(ctx);

    // Try to get graph.
    Graph *g = Graph_Get(ctx, qctx->graphName);
    if(!g) {
        if(ast->createNode || ast->mergeNode) {
            g = _MGraph_CreateGraph(ctx, qctx->graphName);
            /* TODO: free graph if no entities were created. */
        } else {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            _MGraph_ReleaseLock(ctx);
            goto cleanup;
        }
    }

    if (ast->indexNode) { // index operation
        _index_operation(ctx, graph_name, g, ast->indexNode);
        // Release read lock
        _MGraph_ReleaseLock(ctx);
    } else {
        ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast, false);
        ResultSet* resultSet = ExecutionPlan_Execute(plan);
        ExecutionPlanFree(plan);

        // Done accessing graph data, release lock.
        _MGraph_ReleaseLock(ctx);

        /* Send result-set back to client. */
        ResultSet_Replay(resultSet);

        /* Replicate query only if it modified the keyspace. */
        if(ResultSetStat_IndicateModification(resultSet->stats))
            RedisModule_ReplicateVerbatim(ctx);

        ResultSet_Free(resultSet);
    }

    /* Report execution timing. */
    double t = simple_toc(qctx->tic) * 1000;
    char* strElapsed;
    asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    
    // Clean up.
cleanup:
    RedisModule_UnblockClient(qctx->bc, NULL);
    Free_AST_Query(ast);
    free(qctx);
}

void _MGraph_BulkInsert(void *args) {
    BulkInsertContext *context = (BulkInsertContext *)args;
    Graph *g;           // Graph to populate.
    size_t nodes = 0;   // Number of nodes created.
    size_t edges = 0;   // Number of edge created.
    int argc = context->argc;
    RedisModuleString **argv = context->argv;
    RedisModuleString *rs_graph_name = argv[1];
    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(context->bc);
    const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);

    _MGraph_AcquireWriteLock(ctx);
    
    // Try to get graph, if graph does not exists create it.
    g = Graph_Get(ctx, rs_graph_name);
    if(g == NULL)
        g = _MGraph_CreateGraph(ctx, rs_graph_name);

    Bulk_Insert(ctx, argv+2, argc-2, g, graph_name, &nodes, &edges);

    // Force graph pendding operations to complete.
    Graph_CommitPendingOps(g);

    _MGraph_ReleaseLock(ctx);

    // Replay to caller.
    double t = simple_toc(context->tic);
    char timings[1024] = {0};
    snprintf(timings, 1024, "%zu Nodes created, %zu Edges created, time: %.6f sec\n", nodes, edges, t);
    RedisModule_ReplyWithStringBuffer(ctx, timings, strlen(timings));

    // Replicate query.
    RedisModule_ReplicateVerbatim(ctx);
    RedisModule_UnblockClient(context->bc, NULL);

    // Clean up
    BulkInsertContext_Free(context);
}

void _MGraph_Delete(void *args) {
    double tic[2];
    simple_tic(tic);
    DeleteGraphContext *dCtx = (DeleteGraphContext *)args;
    RedisModuleBlockedClient *bc = dCtx->bc;
    RedisModuleString *graphID = dCtx->graphID;

    RedisModuleCtx *ctx = RedisModule_GetThreadSafeContext(bc);
    const char *graphIDStr = RedisModule_StringPtrLen(graphID, NULL);

    _MGraph_AcquireWriteLock(ctx);
    
    Graph *g = Graph_Get(ctx, graphID);

    // Graph does not exists, nothing to delete.
    if(!g) goto cleanup;

    // Remove Label stores.
    size_t keyCount = 0;
    RedisModuleString **keys = LabelStore_GetKeys(ctx, graphIDStr, &keyCount);
    assert(keyCount>0 && keys);
    
    for(int idx = 0; idx < keyCount; idx++) {
        RedisModuleString *storeKeyStr = keys[idx];
        RedisModuleKey *key = RedisModule_OpenKey(ctx, storeKeyStr, REDISMODULE_WRITE);
        if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
            // Log error!
        }
        RedisModule_Free(storeKeyStr);
    }
    free(keys);

    // Remove Graph from Redis keyspace.
    RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_WRITE);
    if(RedisModule_DeleteKey(key) != REDISMODULE_OK) {
        // Log error!
    }

cleanup:
    _MGraph_ReleaseLock(ctx);
    DeleteGraphContext_free(dCtx);

    char* strElapsed;
    double t = simple_toc(tic) * 1000;
    asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    RedisModule_UnblockClient(bc, NULL);
}

//------------------------------------------------------------------------------
// Module Commands
//------------------------------------------------------------------------------

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    double tic[2];
    if (argc < 3) return RedisModule_WrongArity(ctx);

    simple_tic(tic);

    // Parse AST.
    // TODO: support concurent parsing.
    char *errMsg = NULL;
    const char *query = RedisModule_StringPtrLen(argv[2], NULL);
    AST_Query* ast = ParseQuery(query, strlen(query), &errMsg);
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    // Construct concurent query context.
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    QueryContext *context = QueryContext_New(ctx, bc, ast, argv[1]);

    context->tic[0] = tic[0];
    context->tic[1] = tic[1];    

    thpool_add_work(_thpool, _MGraph_Query, context);

    return REDISMODULE_OK;
}

/* Delete graph, removes every Redis key related to given graph 
 * free every resource allocated by the graph. */
int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) return RedisModule_WrongArity(ctx);
    
    // Construct delete operation context.
    RedisModuleString *graphID = argv[1];
    RedisModule_RetainString(ctx, graphID);
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    DeleteGraphContext *dCtx = DeleteGraphContext_new(graphID, bc);

    thpool_add_work(_thpool, _MGraph_Delete, dCtx);

    return REDISMODULE_OK;
}

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    const char *graph_name;
    const char *query;
    RMUtil_ParseArgs(argv, argc, 1, "cc", &graph_name, &query);

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST_Query* ast = ParseQuery(query, strlen(query), &errMsg);

    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    // Try to get graph.
    Graph *g = Graph_Get(ctx, argv[1]);
    if(!g) {
        RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
        return REDISMODULE_OK;
    }

    ModifyAST(ctx, ast, graph_name);

    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        free(reason);
        return REDISMODULE_OK;
    }

    if (ast->indexNode != NULL) { // index operation
        const char *strPlan = Index_OpPrint(ast->indexNode);
        RedisModule_ReplyWithSimpleString(ctx, strPlan);
        return REDISMODULE_OK;
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast, true);
    char* strPlan = ExecutionPlanPrint(plan);
    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));
    
    ExecutionPlanFree(plan);
    Free_AST_Query(ast);
    return REDISMODULE_OK;
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Prepare context.
    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    BulkInsertContext *context = BulkInsertContext_New(ctx, bc, argv, argc);
    simple_tic(context->tic);

    // Execute bulk insert on a dedicated thread.
    thpool_add_work(_thpool, _MGraph_BulkInsert, context);
    return REDISMODULE_OK;
}

int _RegisterDataTypes(RedisModuleCtx *ctx) {
    if(StoreType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register storetype\n");
        return REDISMODULE_ERR;
    }

    if(GraphType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register graphtype\n");
        return REDISMODULE_ERR;
    }

    if(IndexType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register indextype\n");
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
    /* TODO: when module unloads call GrB_finalize. */
    Agg_RegisterFuncs();
    AR_RegisterFuncs(); /* Register arithmetic expression functions. */

    if (!_Setup_ThreadPOOL()) {
        return REDISMODULE_ERR;
    }

    // Initialize read write lock.
    if (pthread_rwlock_init(&_rwlock, NULL)) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (_RegisterDataTypes(ctx) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_Delete, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", MGraph_Explain, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.BULK", MGraph_BulkInsert, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
