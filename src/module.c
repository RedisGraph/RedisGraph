#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

#include "graph/graph.h"
#include "graph/graph_type.h"
#include "graph/edge.h"

#include "value.h"
#include "redismodule.h"
#include "query_executor.h"
#include "bulk_insert.h"
#include "arithmetic/arithmetic_expression.h"

#include "util/simple_timer.h"
#include "util/triemap/triemap_type.h"

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

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    /* Time query execution */
    double tic [2], t;
    simple_tic(tic);

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
    
    ModifyAST(ctx, ast, graph_name);

    // Try to get graph.
    Graph *g = Graph_Get(ctx, argv[1]);
    if(!g) {
        if(ast->createNode || ast->mergeNode) {
            g = _MGraph_CreateGraph(ctx, argv[1]);
            /* TODO: free graph if no entities were created. */
        } else {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            return REDISMODULE_OK;
        }
    }

    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        return REDISMODULE_OK;
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast);
    ResultSet* resultSet = ExecutionPlan_Execute(plan);
    ExecutionPlanFree(plan);

    /* Send result-set back to client. */
    ResultSet_Replay(ctx, resultSet);

    /* Replicate query only if it modified the keyspace. */
    if(ResultSetStat_IndicateModification(resultSet->stats)) {
        RedisModule_ReplicateVerbatim(ctx);
    }

    ResultSet_Free(ctx, resultSet);

    /* Report execution timing. */
    t = simple_toc(tic) * 1000;
    char* strElapsed;
    asprintf(&strElapsed, "Query internal execution time: %.6f milliseconds", t);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    
    Free_AST_Query(ast);
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

    // Try to get graph.
    Graph *g = Graph_Get(ctx, argv[1]);
    if(!g) {
        if(ast->createNode || ast->mergeNode) {
            g = _MGraph_CreateGraph(ctx, argv[1]);
            /* TODO: free graph if no entities were created. */
        } else {
            RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
            return REDISMODULE_OK;
        }
    }

    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    ModifyAST(ctx, ast, graph_name);

    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        return REDISMODULE_OK;
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast);
    char* strPlan = ExecutionPlanPrint(plan);
    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));
    
    ExecutionPlanFree(plan);
    Free_AST_Query(ast);
    return REDISMODULE_OK;
}

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    double tic [2], t;

    Graph *g;           // Graph to populate.
    size_t nodes = 0;   // Number of nodes created.
    size_t edges = 0;   // Number of edge created.
    RedisModuleString *rs_graph_name = argv[1];
    const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);
    
    // Try to get graph, if graph does not exists create it.
    g = Graph_Get(ctx, rs_graph_name);
    if(g == NULL)
        g = _MGraph_CreateGraph(ctx, rs_graph_name);

    simple_tic(tic);
    Bulk_Insert(ctx, argv+2, argc-2, g, graph_name, &nodes, &edges);

    // Force graph pendding operations to complete.
    Graph_CommitPendingOps(g);

    // Replay to caller.
    t = simple_toc(tic);
    char timings[1024] = {0};
    sprintf(timings, "%zu Nodes created, %zu Edges created, time: %.6f sec\n", nodes, edges, t);
    RedisModule_ReplyWithStringBuffer(ctx, timings, strlen(timings));
    
    /* Replicate query. */
    RedisModule_ReplicateVerbatim(ctx);
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

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
    /* TODO: when module unloads call GrB_finalize. */

    InitGroupCache();
    Agg_RegisterFuncs();
    AR_RegisterFuncs(); /* Register arithmetic expression functions. */

    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (_RegisterDataTypes(ctx) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write", 1, 1, 1) == REDISMODULE_ERR) {
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
