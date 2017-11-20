#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "graph/edge.h"
#include "graph/node.h"
#include "graph/graph.h"

#include "value.h"
#include "redismodule.h"
#include "query_executor.h"
#include "arithmetic_expression.h"

#include "util/prng.h"
#include "util/snowflake.h"
#include "util/triemap/triemap_type.h"

#include "rmutil/util.h"
#include "rmutil/vector.h"

#include "parser/ast.h"
#include "parser/grammar.h"
#include "parser/parser_common.h"

#include "stores/store.h"

#include "grouping/group_cache.h"
#include "aggregate/agg_funcs.h"
#include "hexastore/hexastore.h"
#include "hexastore/triplet.h"

#include "resultset/record.h"
#include "resultset/resultset.h"

#include "execution_plan/execution_plan.h"

/* Removes given graph.
 * Args:
 * argv[1] graph name
 * sorted set holding graph name is deleted */
int MGraph_DeleteGraph(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key;
    char *graph;
    char *storeId;
    RMUtil_ParseArgs(argv, argc, 1, "c", &graph);
    
    Store *store = GetStore(ctx, STORE_NODE, graph, NULL);
    StoreIterator *it = Store_Search(store, "");
    
    char *nodeId;
    tm_len_t nodeIdLen;
    Node *node;

    while(StoreIterator_Next(it, &nodeId, &nodeIdLen, (void**)&node)) {
        RedisModuleString* strKey = RedisModule_CreateString(ctx, nodeId, nodeIdLen);
        key = RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);
        RedisModule_FreeString(ctx, strKey);
        RedisModule_DeleteKey(key);
        RedisModule_CloseKey(key);
    }
    StoreIterator_Free(it);
    
    int storeIdLen = Store_ID(&storeId, STORE_NODE, graph, NULL);
    RedisModuleString *rmStoreId = RedisModule_CreateString(ctx, storeId, storeIdLen);
    free(storeId);
    key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);
    
    /* Deletes the actual store + each stored node. */
    RedisModule_DeleteKey(key);
    RedisModule_CloseKey(key);

    /* TODO: delete store key. */
    store = GetStore(ctx, STORE_EDGE, graph, NULL);
    it = Store_Search(store, "");
    char *edgeId;
    tm_len_t edgeIdLen;
    Edge *edge;

    while(StoreIterator_Next(it, &edgeId, &edgeIdLen, (void**)&edge)) {
        RedisModuleString* strKey = RedisModule_CreateString(ctx, edgeId, edgeIdLen);
        key = RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);
        RedisModule_FreeString(ctx, strKey);
        RedisModule_DeleteKey(key);
        RedisModule_CloseKey(key);
    }
    StoreIterator_Free(it);

    storeIdLen = Store_ID(&storeId, STORE_EDGE, graph, NULL);
    rmStoreId = RedisModule_CreateString(ctx, storeId, storeIdLen);
    free(storeId);
    key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);
    
    /* Deletes the actual store + each stored edge. */
    RedisModule_DeleteKey(key);
    RedisModule_CloseKey(key);
    
    /* TODO: delete store key.
     * TODO: Delete label stores... */
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) return RedisModule_WrongArity(ctx);

    /* Time query execution */
    clock_t start = clock();
    clock_t end;

    const char *graphName;
    const char *query;
    RMUtil_ParseArgs(argv, argc, 1, "cc", &graphName, &query);
    
    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST_QueryExpressionNode* ast = ParseQuery(query, strlen(query), &errMsg);
    
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    
    /* Modify AST */
    if(ReturnClause_ContainsCollapsedNodes(ast) == 1) {
        /* Expend collapsed nodes. */
        ReturnClause_ExpandCollapsedNodes(ctx, ast, graphName);
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, graphName, ast);
    ResultSet* resultSet = ExecutionPlan_Execute(plan);

    /* Send result-set back to client. */
    ExecutionPlanFree(plan);
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
    end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double elapsedMS = elapsed * 1000; 
    char* strElapsed;
    asprintf(&strElapsed, "Query internal execution time: %f milliseconds", elapsedMS);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    
    /* TODO: free AST
     * FreeQueryExpressionNode(ast); */
    return REDISMODULE_OK;
}

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 2) return RedisModule_WrongArity(ctx);
    
    const char *graphName;
    const char *query;
    RMUtil_ParseArgs(argv, argc, 1, "cc", &graphName, &query);

    /* Parse query, get AST. */
    char *errMsg = NULL;
    AST_QueryExpressionNode *ast = ParseQuery(query, strlen(query), &errMsg);
    
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    
    /* Modify AST */
    if(ReturnClause_ContainsCollapsedNodes(ast) == 1) {
        /* Expend collapsed nodes. */
        ReturnClause_ExpandCollapsedNodes(ctx, ast, graphName);
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, graphName, ast);
    char* strPlan = ExecutionPlanPrint(plan);
    /* TODO: free execution plan.
     * ExecutionPlanFree(plan); */

    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));
    free(strPlan);
    
    /* TODO: free AST
     * FreeQueryExpressionNode(ast); */
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    InitGroupCache();
    Agg_RegisterFuncs();
    AR_RegisterFuncs(); /* Register arithmetic expression functions. */

    if (snowflake_init(1, 1) != 1) {
        RedisModule_Log(ctx, "error", "Failed to initialize snowflake");
        return REDISMODULE_ERR;
    }

    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(TrieMapType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register triemaptype\n");
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_DeleteGraph, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", MGraph_Explain, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}