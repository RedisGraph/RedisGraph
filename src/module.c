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

/* Tries to retrieve graph object stored within Redis under graph_name key,
 * If specified key does not exists and create is set to 1, a new graph object
 * is created and stored, in case key already stores a different object type
 * NULL is returned. */
Graph *_MGraph_CreateGraph(RedisModuleCtx *ctx, RedisModuleString *graph_name) {
    Graph *g = NULL;

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);

    // Key does not exists, create a new graph.
	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		g = Graph_New(GRAPH_DEFAULT_NODE_CAP);
		RedisModule_ModuleTypeSetValue(key, GraphRedisModuleType, g);        
	} else {
        RedisModule_ReplyWithError(ctx, "can not create graph, graph name is used by some other key.");
        RedisModule_CloseKey(key);
    }

    // Returned stored graph.
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
    
    // Try to get graph.
    Graph *g = Graph_Get(ctx, argv[1]);
    if(!g) {
        if(ast->createNode) {
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
    
    /* Modify AST */
    if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
        /* Expand collapsed nodes. */
        ReturnClause_ExpandCollapsedNodes(ctx, ast, graph_name);
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, g, graph_name, ast);
    ResultSet* resultSet = ExecutionPlan_Execute(plan);
    ExecutionPlanFree(plan);

    /* Send result-set back to client. */
    ResultSet_Replay(ctx, resultSet);

    /* Replicate query only if it modified the keyspace. */
    if(Query_Modifies_KeySpace(ast) &&
       (resultSet->labels_added > 0 ||
       resultSet->nodes_created > 0 ||
       resultSet->properties_set > 0 ||
       resultSet->relationships_created > 0 ||
       resultSet->nodes_deleted > 0 ||
       resultSet->relationships_deleted > 0)) {
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

    // Try to get graph.
    Graph *g = Graph_Get(ctx, argv[1]);
    if(!g) {
        /* At the moment EXPLAIN requires that the graph would exists. */
        RedisModule_ReplyWithError(ctx, "key doesn't contains a graph object.");
        return REDISMODULE_OK;
    }

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST_Query* ast = ParseQuery(query, strlen(query), &errMsg);
    
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    
    char *reason;
    if (AST_Validate(ast, &reason) != AST_VALID) {
        RedisModule_ReplyWithError(ctx, reason);
        return REDISMODULE_OK;
    }
    
    /* Modify AST */
    if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
        /* Expand collapsed nodes. */
        ReturnClause_ExpandCollapsedNodes(ctx, ast, graph_name);
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
    /*
    GRAPH.BULK my_graph NODES 6 2 Person 2 2 first_name last_name Country 2 2 name population roi lipman hila rahamani Israel 7000000 Japan 127000000 2 key1 val1 key2 2 2 foo bar bar baz RELATIONS 5 2 knows 2 visited 2 0 1 1 0 0 2 1 3 4 5
    */

    Graph *g;           // Graph to populate.
    size_t nodes = 0;   // Number of nodes created.
    size_t edges = 0;   // Number of edge created.
    RedisModuleString *rs_graph_name = argv[1];
    const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);

    // Get graph from Redis keyspace, create if missing.
    g = Graph_Get(ctx, rs_graph_name);
    if(g == NULL) {
        g = _MGraph_CreateGraph(ctx, rs_graph_name);
        if(g == NULL) {
            // Specified graph name points to an existing key which is not of type Graph.
            // Error already emitted.
            return REDISMODULE_OK;
        }
    }

    // simple_tic(tic);
    Bulk_Insert(ctx, argv+2, argc-2, g, graph_name, &nodes, &edges);

    // Replay to caller.
    // t = simple_toc(tic);
    char timings[1024] = {0};
    // sprintf(timings, "%zu Nodes created, %zu Edges created, time: %.6f sec\n", nodes, edges, t);
    sprintf(timings, "%zu Nodes created, %zu Edges created\n", nodes, edges);
    RedisModule_ReplyWithStringBuffer(ctx, timings, strlen(timings));
    
    /* Replicate query only if it modified the keyspace. */
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
