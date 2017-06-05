#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "graph/edge.h"
#include "graph/node.h"
#include "graph/graph.h"

#include "value.h"
#include "value_cmp.h"
#include "redismodule.h"
#include "query_executor.h"

#include "rmutil/util.h"
#include "rmutil/vector.h"
#include "rmutil/strings.h"
#include "rmutil/test_util.h"
#include "util/prng.h"
#include "util/snowflake.h"
#include "util/triemap/triemap_type.h"

#include "parser/ast.h"
#include "parser/grammar.h"
#include "parser/parser_common.h"

#include "stores/store.h"

#include "aggregate/functions.h"
#include "aggregate/aggregate.h"

#include "grouping/group.h"
#include "grouping/group_cache.h"
#include "hexastore/hexastore.h"
#include "hexastore/triplet.h"

#include "resultset/record.h"
#include "resultset/resultset.h"

#include "execution_plan/execution_plan.h"

/* Creates a new node and stores its properties within a redis hash.
 * Returns the new node ID */
RedisModuleString* _CreateGraphEntity(RedisModuleCtx *ctx, RedisModuleString **properties, int propCount) {
    // Get an id
    unsigned char id[33] = {};
    get_new_id(id);

    // Store node properties within a redis hash
    RedisModuleString* entityID = RedisModule_CreateString(ctx, id, strlen(id));
    // RedisModuleString* entityID = RedisModule_CreateString(ctx, id, 32);
    RedisModuleKey *key = RedisModule_OpenKey(ctx, entityID, REDISMODULE_WRITE);
    // TODO: check if key exists
    
    // Insert node attributes
    for(int i = 0; i < propCount; i+=2) {
        RedisModule_HashSet(key, REDISMODULE_HASH_NONE, properties[i], properties[i+1], NULL);
    }
    
    RedisModule_CloseKey(key);
    return entityID;
}

/* Creates a new node
 * Args:
 * argv[1] graph name
 * argv[2] label (optional)
 * argv[I] attribute name
 * argv[I+1] attribute value 
 * node gets a unique ID
 * and added to the nodes store */
int MGraph_CreateNode(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Expecting at least 4 arguments
    // number of arguments should be an even number.
    if(argc < 4) {
        return RedisModule_WrongArity(ctx);
    }

    int propStartIdx = 2;
    int labelSpecified = 0;
    if(argc % 2 == 1) {
        // Label specified.
        propStartIdx = 3;
        labelSpecified = 1;
    }

    RedisModuleString *graph = argv[1];
    RedisModuleString **properties = argv+propStartIdx;
    RedisModuleString *nodeID = _CreateGraphEntity(ctx, properties, argc-propStartIdx);

    const char *strNodeID = RedisModule_StringPtrLen(nodeID, NULL);
    Node *n = NewNode(NULL, strNodeID, NULL);
    
    // Place node id within node store
    Store *store = GetStore(ctx, STORE_NODE, graph, NULL);
    Store_Insert(store, nodeID, n);

    // Place node id within labeled node store
    if(labelSpecified == 1) {
        RedisModuleString *label = argv[2];

        // Set node's label.
        const char *strLabel = RedisModule_StringPtrLen(label, NULL);
        n->label = strdup(strLabel);

        // Store node within label store.
        store = GetStore(ctx, STORE_NODE, graph, label);
        Store_Insert(store, nodeID, n);
    }
    
    RedisModule_ReplyWithString(ctx, nodeID);
    RedisModule_FreeString(ctx, nodeID);

    return REDISMODULE_OK;
}

/* Adds a new edge to the graph.
 * Args:
 * argv[1] graph name
 * argv[2] source node
 * argv[3] dest node
 * argv[4] edge type
 * argv[5] edge properties
 * connects source node with dest node with a directional edge. */

int MGraph_AddEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc < 5 || argc%2 == 0) {
        return RedisModule_WrongArity(ctx);
    }
    
    RedisModuleString *graph;
    RedisModuleString *src;
    RedisModuleString *dest;
    RedisModuleString *edgeType;

    RMUtil_ParseArgs(argv, argc, 1, "ssss", &graph, &src, &edgeType, &dest);
    
    const char *strSrc = RedisModule_StringPtrLen(src, NULL);
    const char *strDest = RedisModule_StringPtrLen(dest, NULL);
    const char* strEdgeType = RedisModule_StringPtrLen(edgeType, NULL);
    
    // Retreive source and dest nodes from node store
    Store *nodeStore = GetStore(ctx, STORE_NODE, graph, NULL);
    Node *srcNode = Store_Get(nodeStore, strSrc);
    Node *destNode = Store_Get(nodeStore, strDest);

    // Make sure both src and dest nodes exists
    if(srcNode == NULL || destNode == NULL) {
        // Linking none-existing node(s)
        RedisModule_ReplyWithSimpleString(ctx, "Error, missing node(s)");
        return REDISMODULE_OK;
    }

    // Save edge properties
    RedisModuleString **properties = argv + 5;
    RedisModuleString *edgeID = _CreateGraphEntity(ctx, properties, argc-5);
    const char* strEdgeID = RedisModule_StringPtrLen(edgeID, NULL);

    // Create the actual edge
    Edge *edge = NewEdge(strEdgeID, NULL, srcNode, destNode, strEdgeType);

    // Place edge within edge store(s)
    Store *edgeStore = GetStore(ctx, STORE_EDGE, graph, NULL);
    Store_Insert(edgeStore, edgeID, edge);
    
    edgeStore = GetStore(ctx, STORE_EDGE, graph, edgeType);
    Store_Insert(edgeStore, edgeID, edge);
    ConnectNode(srcNode, destNode, edge);
    
    /* Store relation within hexastore
    * one triplet is used as key, this one contains the 
    * edge lable and id, while the other triplet
    * stored as the value does contains only IDs, no labels */
    Triplet *keyTriplet = TripletFromEdge(edge);
    Triplet *valTriplet = NewTriplet(srcNode->id, edge->id, destNode->id);
    HexaStore *hexastore = GetHexaStore(ctx, graph);
    HexaStore_InsertAllPerm(hexastore, keyTriplet->subject, keyTriplet->predicate, keyTriplet->object, valTriplet);
    FreeTriplet(keyTriplet);

    RedisModule_ReplyWithString(ctx, edgeID);
    RedisModule_FreeString(ctx, edgeID);
    return REDISMODULE_OK;
}

/* Retrives node
 * argv[1] graph name
 * argv[2] node id
 * returns node attributes plus label if exists */
int MGraph_GetNode(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 3) {
        return RedisModule_WrongArity(ctx);
    }
    
    RedisModuleString *graph;
    RedisModuleString *nodeId;
    
    RMUtil_ParseArgs(argv, argc, 1, "ss", &graph, &nodeId);
    const char *strNodeId = RedisModule_StringPtrLen(nodeId, NULL);

    Store *nodeStore = GetStore(ctx, STORE_NODE, graph, NULL);
    Node *node = Store_Get(nodeStore, strNodeId);

    if(node == NULL) {
        RedisModule_ReplyWithStringBuffer(ctx, "Missing node", strlen("Missing node"));
        return REDISMODULE_OK;
    }

    RMUtilInfo *attributes = RMUtil_HGetAll(ctx, nodeId);
    
    int responseLength = attributes->numEntries * 2;
    if(node->label != NULL) {
        responseLength += 2;
    }

    RedisModule_ReplyWithArray(ctx, responseLength);
    
    if(node->label != NULL) {
        RedisModule_ReplyWithStringBuffer(ctx, "label", 5);
        RedisModule_ReplyWithStringBuffer(ctx, node->label, strlen(node->label));
    }
    
    for(int i = 0; i < attributes->numEntries; i++) {
        RMUtilInfoEntry entry = attributes->entries[i];
        RedisModule_ReplyWithStringBuffer(ctx, entry.key, strlen(entry.key));
        RedisModule_ReplyWithStringBuffer(ctx, entry.val, strlen(entry.val));
    }
    
    return REDISMODULE_OK;
}

/* Retrives an edge
 * argv[1] graph name
 * argv[2] edge id
 * returns edge attributes plus three additional:
 * source node id, dest node id and edge label */
int MGraph_GetEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 3) {
        return RedisModule_WrongArity(ctx);
    }
    
    RedisModuleString *graph;
    RedisModuleString *edgeId;
    
    RMUtil_ParseArgs(argv, argc, 1, "ss", &graph, &edgeId);
    const char *strEdgeId = RedisModule_StringPtrLen(edgeId, NULL);

    Store *edgeStore = GetStore(ctx, STORE_EDGE, graph, NULL);
    Edge *edge = Store_Get(edgeStore, strEdgeId);

    if(edge == NULL) {
        RedisModule_ReplyWithStringBuffer(ctx, "Missing edge", strlen("Missing edge"));
        return REDISMODULE_OK;
    }

    RMUtilInfo *attributes = RMUtil_HGetAll(ctx, edgeId);
    
    RedisModule_ReplyWithArray(ctx, (attributes->numEntries*2)+8);
    
    RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
    RedisModule_ReplyWithStringBuffer(ctx, edge->id, strlen(edge->id));
    
    RedisModule_ReplyWithStringBuffer(ctx, "type", 4);
    RedisModule_ReplyWithStringBuffer(ctx, edge->relationship, strlen(edge->relationship));

    RedisModule_ReplyWithStringBuffer(ctx, "src", 3);
    RedisModule_ReplyWithStringBuffer(ctx, edge->src->id, strlen(edge->src->id));
    
    RedisModule_ReplyWithStringBuffer(ctx, "dest", 4);
    RedisModule_ReplyWithStringBuffer(ctx, edge->dest->id, strlen(edge->dest->id));

    for(int i = 0; i < attributes->numEntries; i++) {
        RMUtilInfoEntry entry = attributes->entries[i];
        RedisModule_ReplyWithStringBuffer(ctx, entry.key, strlen(entry.key));
        RedisModule_ReplyWithStringBuffer(ctx, entry.val, strlen(entry.val));
    }
    
    return REDISMODULE_OK;
}

/* Removes edge from the graph.
 * Args:
 * argv[1] graph name
 * argv[2] subject
 * argv[3] edge, predicate
 * argv[4] object
 * removes all 6 triplets representing
 * the connection (predicate) between subject and object */
int MGraph_RemoveEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 5) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *graph;
    RedisModuleString *subject;
    RedisModuleString *predicate;
    RedisModuleString *object;

    RMUtil_ParseArgs(argv, argc, 1, "ssss", &graph, &subject, &predicate, &object);

    const char *strSubject = RedisModule_StringPtrLen(subject, NULL);
    const char *strPredicate = RedisModule_StringPtrLen(predicate, NULL);
    const char *strObject = RedisModule_StringPtrLen(object, NULL);

    HexaStore *hexaStore = GetHexaStore(ctx, graph);
    HexaStore_RemoveAllPerm(hexaStore, strSubject, strPredicate, strObject);

    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/* Returns node's edges
 * Args:
 * argv[1] graph name
 * argv[2] node id
 * argv[3] edge type
 * argv[4] edge direction: OUT(0), IN(1), BOTH(2) */
int MGraph_GetNodeEdges(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 5) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *graph;
    char *nodeId;
    char *edgeType;
    long long direction;
    RMUtil_ParseArgs(argv, argc, 1, "sccl", &graph, &nodeId, &edgeType, &direction);
    
    char* prefixes[2] = {0};
    
    switch(direction) {
        case 0: // OUT
            asprintf(&prefixes[0], "SPO:%s:%s", nodeId, edgeType);
            break;
        case 1: // IN
            asprintf(&prefixes[0], "OPS:%s:%s", nodeId, edgeType);
            break;
        case 2: // BOTH
            asprintf(&prefixes[0], "SPO:%s:%s", nodeId, edgeType);
            asprintf(&prefixes[1], "OPS:%s:%s", nodeId, edgeType);
            break;
        default:
            break;
    }
    
    HexaStore *hexastore = GetHexaStore(ctx, graph);
    Triplet *triplet;
    TripletIterator *iter;
    int edgeCount = 0;
    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
    for(int i = 0; i < 2; i++) {
        if(prefixes[i] == NULL) {
            break;
        }

        iter = HexaStore_Search(hexastore, prefixes[i]);
        
        while(TripletIterator_Next(iter, &triplet) != NULL) {
            RedisModule_ReplyWithStringBuffer(ctx, triplet->predicate, strlen(triplet->predicate));
            edgeCount++;
        }

        free(prefixes[i]);
        TripletIterator_Free(iter);
    }
    
    RedisModule_ReplySetArrayLength(ctx, edgeCount);
    return REDISMODULE_OK;
}

/* Returns node's neighbours
 * Args:
 * argv[1] graph name
 * argv[2] node id
 * argv[3] edge type
 * argv[4] edge direction: OUT(0), IN(1), BOTH(2) */
int MGraph_GetNeighbours(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 5) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *graph;
    char *nodeId;
    char *edgeType;
    long long direction;
    RMUtil_ParseArgs(argv, argc, 1, "sccl", &graph, &nodeId, &edgeType, &direction);
    
    char* prefixes[2] = {0};
    
    switch(direction) {
        case 0: // OUT
            asprintf(&prefixes[0], "SPO:%s:%s", nodeId, edgeType);
            break;
        case 1: // IN
            asprintf(&prefixes[0], "OPS:%s:%s", nodeId, edgeType);
            break;
        case 2: // BOTH
            asprintf(&prefixes[0], "SPO:%s:%s", nodeId, edgeType);
            asprintf(&prefixes[1], "OPS:%s:%s", nodeId, edgeType);
            break;
        default:
            break;
    }
    
    HexaStore *hexastore = GetHexaStore(ctx, graph);
    Triplet *triplet;
    TripletIterator *iter;
    int neighboursCount = 0;
    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
    for(int i = 0; i < 2; i++) {
        if(prefixes[i] == NULL) {
            break;
        }

        iter = HexaStore_Search(hexastore, prefixes[i]);
        
        while(TripletIterator_Next(iter, &triplet) != NULL) {
            if(i == 0) /* OUT */ {
                RedisModule_ReplyWithStringBuffer(ctx, triplet->object, strlen(triplet->object));
            } else /* IN */ {
                RedisModule_ReplyWithStringBuffer(ctx, triplet->subject, strlen(triplet->subject));
            }
            neighboursCount++;
        }

        free(prefixes[i]);
        TripletIterator_Free(iter);
    }
    
    RedisModule_ReplySetArrayLength(ctx, neighboursCount);
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
    RedisModuleString *graph;
    RedisModuleString *storeId;
    RMUtil_ParseArgs(argv, argc, 1, "s", &graph);
    
    Store *store = GetStore(ctx, STORE_NODE, graph, NULL);
    StoreIterator *it = Store_Search(store, "");
    char *nodeId;

    // TODO: maybe we should delete node key as part of FreeNode?
    // Free each node, start by deleting redis hashes
    while((nodeId = StoreIterator_Next(it)) != NULL) {
        RedisModuleString* strKey = RedisModule_CreateString(ctx, nodeId, strlen(nodeId));
        key = RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);
        RedisModule_FreeString(ctx, strKey);
        RedisModule_DeleteKey(key);
        RedisModule_CloseKey(key);
    }
    StoreIterator_Free(it);
    
    storeId = Store_ID(ctx, STORE_NODE, graph, NULL);
    key = RedisModule_OpenKey(ctx, storeId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, storeId);
    // Deletes the actual store + each stored node.
    RedisModule_DeleteKey(key);
    RedisModule_CloseKey(key);

    // TODO: delete store key.
    
    store = GetStore(ctx, STORE_EDGE, graph, NULL);
    it = Store_Search(store, "");
    char *edgeId;

    // TODO: maybe we should delete edge key as part of FreeEdge?
    // Free each edge, start by deleting redis hashes
    while((edgeId = StoreIterator_Next(it)) != NULL) {
        RedisModuleString* strKey = RedisModule_CreateString(ctx, edgeId, strlen(edgeId));
        key = RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);
        RedisModule_FreeString(ctx, strKey);
        RedisModule_DeleteKey(key);
        RedisModule_CloseKey(key);
    }
    StoreIterator_Free(it);

    storeId = Store_ID(ctx, STORE_EDGE, graph, NULL);
    key = RedisModule_OpenKey(ctx, storeId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, storeId);
    // Deletes the actual store + each stored edge.
    RedisModule_DeleteKey(key);
    RedisModule_CloseKey(key);
    
    // TODO: delete store key.
    // TODO: Delete label stores...
    
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/* Queries graph
 * Args:
 * argv[1] graph name
 * argv[2] query to execute */
int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 2) return RedisModule_WrongArity(ctx);

    // Time query execution
    clock_t start = clock();
    clock_t end;

    RedisModuleString *graphName;
    RedisModuleString *query;
    RMUtil_ParseArgs(argv, argc, 1, "ss", &graphName, &query);
    
    size_t qLen;
    const char* q = RedisModule_StringPtrLen(query, &qLen);

    // Parse query, get AST.
    char *errMsg = NULL;
    QueryExpressionNode* ast = ParseQuery(q, qLen, &errMsg);
    
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    
    // Modify AST
    if(ReturnClause_ContainsCollapsedNodes(ast->returnNode) == 1) {
        // Expend collapsed nodes.
        ReturnClause_ExpandCollapsedNodes(ctx, ast, graphName);
    }

    ExecutionPlan *plan = NewExecutionPlan(ctx, graphName, ast);
    ResultSet* resultSet = ExecutionPlan_Execute(plan);
    
    // Send result-set back to client.
    ResultSet_Replay(ctx, resultSet);
    ResultSet_Free(ctx, resultSet);
    ExecutionPlanFree(plan);

    // Report execution timing
    end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double elapsedMS = elapsed * 1000; 
    char* strElapsed = (char*)malloc(sizeof(char) * strlen("Query internal execution time: milliseconds") + 8);
    sprintf(strElapsed, "Query internal execution time: %f milliseconds", elapsedMS);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);
    
    // TODO: free AST
    // FreeQueryExpressionNode(ast);
    return REDISMODULE_OK;
}

/* Builds an execution plan but does not execute it
 * reports plan back to the client
 * Args:
 * argv[1] graph name
 * argv[2] query */
int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 2) return RedisModule_WrongArity(ctx);
    
    RedisModuleString *graphName;
    RedisModuleString *query;
    RMUtil_ParseArgs(argv, argc, 1, "ss", &graphName, &query);
    
    size_t qLen;
    const char* q = RedisModule_StringPtrLen(query, &qLen);

    // Parse query, get AST.
    char *errMsg = NULL;
    QueryExpressionNode* ast = ParseQuery(q, qLen, &errMsg);
    
    if (!ast) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }
    
    ExecutionPlan *plan = NewExecutionPlan(ctx, graphName, ast);
    char* strPlan = ExecutionPlanPrint(plan);
    ExecutionPlanFree(plan);

    RedisModule_ReplyWithStringBuffer(ctx, strPlan, strlen(strPlan));
    free(strPlan);
    
    // TODO: free AST
    // FreeQueryExpressionNode(ast);
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

    if(RedisModule_CreateCommand(ctx, "graph.GETNODE", MGraph_GetNode, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.GETEDGE", MGraph_GetEdge, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.GETNODEEDGES", MGraph_GetNodeEdges, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.GETNEIGHBOURS", MGraph_GetNeighbours, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}