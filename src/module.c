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

/* Creates a new node
 * Args:
 * argv[1] graph name
 * argv[2] label (optional)
 * argv[I] attribute name
 * argv[I+1] attribute value 
 * node gets a unique ID
 * and added to the nodes store */
int MGraph_CreateNode(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    /* Expecting at least 4 arguments
     * number of arguments should be an even number. */
    if(argc < 4) {
        return RedisModule_WrongArity(ctx);
    }

    int propStartIdx = 2;
    int labelSpecified = 0;
    if(argc % 2 == 1) {
        /* Label specified. */
        propStartIdx = 3;
        labelSpecified = 1;
    }

    const char *graph;
    RMUtil_ParseArgs(argv, argc, 1, "c", &graph);

    RedisModuleString **properties = argv+propStartIdx;
    char *nodeID;

    int propCount = (argc-propStartIdx)/2;
    char **propKeys = malloc(sizeof(char*)*propCount);
    SIValue *propValues = malloc(sizeof(SIValue)*propCount);

    for(int i = 0; i < propCount; i++) {
        propKeys[i] = strdup(RedisModule_StringPtrLen(properties[i*2], NULL));
        
        size_t prop_len;
        const char *prop = RedisModule_StringPtrLen(properties[i*2+1], &prop_len);
        SIValue_FromString(&propValues[i], strdup(prop), prop_len);
    }

    long int id = get_new_id();
    asprintf(&nodeID, "%ld", id);

    Node *n = NewNode(id, NULL);
    Node_Add_Properties(n, propCount, propKeys, propValues);
    
    /* Place node id within node store. */
    Store *store = GetStore(ctx, STORE_NODE, graph, NULL);
    Store_Insert(store, nodeID, n);

    /* Place node id within labeled node store. */
    if(labelSpecified) {
        RedisModuleString *label = argv[2];

        /* Set node's label. */
        const char *strLabel = RedisModule_StringPtrLen(label, NULL);
        n->label = strdup(strLabel);

        /* Store node within label store. */
        store = GetStore(ctx, STORE_NODE, graph, strLabel);
        Store_Insert(store, nodeID, n);
    }
    
    RedisModule_ReplyWithSimpleString(ctx, nodeID);
    free(nodeID);

    return REDISMODULE_OK;
}

/* Adds a new edge to the graph.
 * Args:
 * argv[1] graph name
 * argv[2] source node
 * argv[3] edge type
 * argv[4] dest node
 * argv[5] edge properties
 * connects source node with dest node with a directional edge. */

int MGraph_AddEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc < 5 || argc%2 == 0) {
        return RedisModule_WrongArity(ctx);
    }
    
    char *graph;
    char *src;
    char *dest;
    char *edge_type;

    RMUtil_ParseArgs(argv, argc, 1, "cccc", &graph, &src, &edge_type, &dest);
    
    /* Retreive source and dest nodes from node store. */
    Store *node_store = GetStore(ctx, STORE_NODE, graph, NULL);
    Node *src_node = Store_Get(node_store, src);
    Node *dest_node = Store_Get(node_store, dest);

    /* Make sure both src and dest nodes exists. */
    if(src_node == NULL || dest_node == NULL) {
        /* Linking none-existing node(s). */
        RedisModule_ReplyWithSimpleString(ctx, "Error, missing node(s)");
        return REDISMODULE_OK;
    }
    
    char *edge_id;
    long int id = get_new_id();
    asprintf(&edge_id, "%ld", id);

    /* Create the actual edge. */
    Edge *edge = NewEdge(id, src_node, dest_node, edge_type);

    /* Save edge properties. */
    int prop_count = (argc-5)/2;
    if(prop_count > 0) {
        RedisModuleString **properties = argv + 5;

        char **prop_keys = malloc(sizeof(char*)*prop_count);
        SIValue *prop_values = malloc(sizeof(SIValue)*prop_count);

        for(int i = 0; i < prop_count; i++) {
            prop_keys[i] = strdup(RedisModule_StringPtrLen(properties[i*2], NULL));
            prop_values[i] = SI_StringValC(strdup(RedisModule_StringPtrLen(properties[i*2+1], NULL)));
        }
        Edge_Add_Properties(edge, prop_count, prop_keys, prop_values);
        free(prop_keys);
    }

    /* Place edge within edge store(s). */
    Store *edge_store = GetStore(ctx, STORE_EDGE, graph, NULL);
    Store_Insert(edge_store, edge_id, edge);
    
    edge_store = GetStore(ctx, STORE_EDGE, graph, edge_type);
    Store_Insert(edge_store, edge_id, edge);
    Node_ConnectNode(src_node, dest_node, edge);
    
    /* Store relation within hexastore
    * one triplet is used as key, this one contains the 
    * edge lable and id, while the other triplet
    * stored as the value does contains only IDs, no labels */
    Triplet *triplet = NewTriplet(src_node, edge, dest_node);
    HexaStore *hexastore = GetHexaStore(ctx, graph);
    HexaStore_InsertAllPerm(hexastore, triplet);

    RedisModule_ReplyWithSimpleString(ctx, edge_id);    
    free(edge_id);
    return REDISMODULE_OK;
}

/* Removes edge from the graph.
 * Args:
 * argv[1] graph name
 * argv[2] edge id
 * removes all 6 triplets representing
 * the connection (predicate) between subject and object */
int MGraph_RemoveEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    char *graph;
    char *edge_id;

    RMUtil_ParseArgs(argv, argc, 1, "cc", &graph, &edge_id);
    
    /* Retreive source and dest nodes from node store. */
    Store *edge_store = GetStore(ctx, STORE_EDGE, graph, NULL);
    Edge *edge = Store_Get(edge_store, edge_id);

    /* Make sure edge exists. */
    if(edge == NULL) {
        RedisModule_ReplyWithSimpleString(ctx, "Error, missing edge");
        return REDISMODULE_OK;
    }

    Triplet *t = TripletFromEdge(edge);

    HexaStore *hexa_store = GetHexaStore(ctx, graph);
    HexaStore_RemoveAllPerm(hexa_store, t);

    FreeTriplet(t);

    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

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
    if (argc < 2) return RedisModule_WrongArity(ctx);

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
    ResultSet_Replay(ctx, resultSet);
    ResultSet_Free(ctx, resultSet);
    ExecutionPlanFree(plan);

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

    if(RedisModule_CreateCommand(ctx, "graph.CREATENODE", MGraph_CreateNode, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    /* Think about renaming. */
    if(RedisModule_CreateCommand(ctx, "graph.ADDEDGE", MGraph_AddEdge, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.REMOVEEDGE", MGraph_RemoveEdge, "write", 1, 1, 1) == REDISMODULE_ERR) {
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