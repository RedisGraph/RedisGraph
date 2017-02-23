#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "graph/edge.h"
#include "graph/node.h"
#include "graph/graph.h"

#include "value.h"
#include "triplet.h"
#include "value_cmp.h"
#include "redismodule.h"
#include "query_executor.h"

#include "rmutil/util.h"
#include "rmutil/vector.h"
#include "rmutil/test_util.h"
#include "util/triemap/triemap_type.h"

#include "parser/ast.h"
#include "parser/grammar.h"
#include "parser/parser_common.h"

#include "aggregate/functions.h"
#include "grouping/group.h"
#include "hexastore/hexastore.h"

#include "resultset/record.h"
#include "resultset/resultset.h"

// Adds a new edge to the graph.
// Args:
// argv[1] graph name
// argv[2] subject
// argv[3] edge, predicate
// argv[4] object
// connect subject to object with a bi directional edge.
// Assuming both subject and object exists.
int MGraph_AddEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
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

    // Create all 6 hexastore variations
    // SPO, SOP, PSO, POS, OSP, OPS
    HexaStore *hexaStore = GetHexaStore(ctx, graph);
    HexaStore_InsertAllPerm(hexaStore, strSubject, strPredicate, strObject);
    
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

// Removes edge from the graph.
// Args:
// argv[1] graph name
// argv[2] subject
// argv[3] edge, predicate
// argv[4] object
// removes all 6 triplets representing
// the connection (predicate) between subject and object
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

// Removes given graph.
// Args:
// argv[1] graph name
// sorted set holding graph name is deleted
int MGraph_DeleteGraph(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleString *graph;
    RMUtil_ParseArgs(argv, argc, 1, "s", &graph);

    RedisModule_DeleteKey(graph);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

TripletIterator* queryTriplet(RedisModuleCtx *ctx, RedisModuleString* graph, const Triplet* triplet) {
    HexaStore *hexaStore = GetHexaStore(ctx, graph);

    char *prefix = TripletToString(triplet);
    TripletIterator *iter = HexaStore_Search(hexaStore, prefix);
    free(prefix);

    return iter;
}

// Concats rmStrings using given delimiter.
// rmStrings is a vector of RedisModuleStrings pointers.
char* _concatRMStrings(const Vector* rmStrings, const char* delimiter, char** concat) {
    size_t length = 0;

    // Compute length
    for(int i = 0; i < Vector_Size(rmStrings); i++) {
        RedisModuleString* string;
        Vector_Get(rmStrings, i, &string);

        size_t len;
        RedisModule_StringPtrLen(string, &len);
        length += len;
    }

    length += strlen(delimiter) * Vector_Size(rmStrings) + 1;
    *concat = calloc(length, sizeof(char));

    for(int i = 0; i < Vector_Size(rmStrings); i++) {
        RedisModuleString* rmString;
        Vector_Get(rmStrings, i, &rmString);

        const char* string = RedisModule_StringPtrLen(rmString, NULL);

        strcat(*concat, string);
        strcat(*concat, delimiter);
    }

    // Discard last delimiter.
    (*concat)[strlen(*concat) - strlen(delimiter)] = NULL;
}

void _aggregateRecord(RedisModuleCtx *ctx, const ReturnNode* returnTree, const Graph* g) {
    // Get group
    Vector* groupKeys = ReturnClause_RetrieveGroupKeys(ctx, returnTree, g);
    char* groupKey = NULL;
    _concatRMStrings(groupKeys, ",", &groupKey);
    // TODO: free groupKeys

    Group* group = NULL;
    CacheGroupGet(groupKey, &group);

    if(group == NULL) {
        // Create a new group
        // Get aggregation functions
        group = NewGroup(groupKeys, ReturnClause_GetAggFuncs(ctx, returnTree));
        CacheGroupAdd(groupKey, group);

        // TODO: no need to get inserted group
        CacheGroupGet(groupKey, &group);
    }

    // TODO: why can't we free groupKey?
    // free(groupKey);

    Vector* valsToAgg = ReturnClause_RetrieveGroupAggVals(ctx, returnTree, g);

    // Run each value through its coresponding function.
    for(int i = 0; i < Vector_Size(valsToAgg); i++) {
        RedisModuleString* value = NULL;
        Vector_Get(valsToAgg, i, &value);
        size_t len;
        const char* strValue = RedisModule_StringPtrLen(value, &len);

        // Convert to double SIValue.
        SIValue numValue;
        numValue.type = T_DOUBLE;
        SI_ParseValue(&numValue, strValue, len);

        AggCtx* funcCtx = NULL;
        Vector_Get(group->aggregationFunctions, i, &funcCtx);
        Agg_Step(funcCtx, &numValue, 1);
    }

    // TODO: free valsToAgg
}

/*
* To speed things up we'll try to skip triplets as early as possible
* even though we might not have a complete graph (some IDs are still missing)
* fastSkipTriplets returns the first triplet which pass the filter-tree
* triplets which fail to pass are discarded.
*/
Triplet* _fastSkipTriplets(RedisModuleCtx *ctx, const QE_FilterNode *filterTree, TripletIterator *iterator, Graph *g, Node *src, Node *dest) {
    // TODO: if triplet doesn't pass filter below we've got a leak.
    Triplet* triplet = TripletIterator_Next(iterator);
    if(triplet == NULL) {
        return NULL;
    }

    // No filters
    if(filterTree == NULL) {
        return triplet;
    }

    src->id = triplet->subject;
    dest->id = NULL;

    // Try to fast skip current src id
    while(applyFilters(ctx, g, filterTree) != 1) {
        // Skip source id
        while((triplet = TripletIterator_Next(iterator), triplet != NULL) && strcmp(triplet->subject, src->id) == 0) {
            FreeTriplet(triplet);
        }

        if(triplet != NULL) {
            // Try new source id
            src->id = triplet->subject;
        } else {
            // Consumed iterator
            return NULL;
        }
    }

    dest->id = triplet->object;

    while(applyFilters(ctx, g, filterTree) != 1) {
        // Skip dest id
        while((triplet = TripletIterator_Next(iterator), triplet != NULL) && strcmp(triplet->object, dest->id) == 0) {
            FreeTriplet(triplet);
        }

        if(triplet != NULL) {
            // Try new dest id
            dest->id = triplet->object;
        } else {
            // Consumed iterator
            return NULL;
        }
    }

    return triplet;
}

void QueryNode(RedisModuleCtx *ctx, RedisModuleString *graphName, Graph* g, Node* n, Vector* entryPoints, const QE_FilterNode* filterTree, const QueryExpressionNode* ast, ResultSet* returnedSet) {
    Node* src = n;
    for(int i = 0; i < Vector_Size(src->outgoingEdges) && !ResultSet_Full(returnedSet); i++) {
        Edge* edge;
        Vector_Get(src->outgoingEdges, i, &edge);
        Node* dest = edge->dest;
        
        // Create a triplet out of edge
        Triplet* triplet = TripletFromEdge(edge);
        
        // Query graph using triplet, no filters are applied at this stage
        TripletIterator* cursor = queryTriplet(ctx, graphName, triplet);
        FreeTriplet(triplet);

        // Backup original node IDs
        char* srcOriginalID = src->id;
        char* edgeOriginalID = edge->relationship;
        char* destOriginalID = dest->id;

        // Run through cursor.
        while(!ResultSet_Full(returnedSet)) {
            // Fast skip triplets which do not pass filters
            triplet = _fastSkipTriplets(ctx, filterTree, cursor, g, src, dest);
            if(triplet == NULL) {
                // Couldn't find a triplet which agrees with filters
                break;
            } else {
                // Set nodes
                src->id = triplet->subject;
                edge->relationship = triplet->predicate;
                dest->id = triplet->object;
            }

            // Advance to next node
            if(Vector_Size(dest->outgoingEdges) > 0) {
                QueryNode(ctx, graphName, g, dest, entryPoints, filterTree, ast, returnedSet);
            } else if(Vector_Size(entryPoints) > 0) {
                // Additional entry point
                Node* entryPoint = NULL;
                Vector_Pop(entryPoints, &entryPoint);
                QueryNode(ctx, graphName, g, entryPoint, entryPoints, filterTree, ast, returnedSet);
                // Restore state, for next iteration.
                Vector_Push(entryPoints, entryPoint);
            } else {
                // We've reach the end of the graph
                if(returnedSet->aggregated) {
                    _aggregateRecord(ctx, ast->returnNode, g);
                } else {
                    // Append to final result set.
                    Record *r = Record_FromGraph(ctx, ast, g);
                    if(r != NULL) {
                        ResultSet_AddRecord(returnedSet, r);
                    }
                }
            }
        } // End of cursor loop
        TripletIterator_Free(cursor);
        
        // Restore nodes original IDs.
        src->id = srcOriginalID;
        edge->relationship = edgeOriginalID;
        dest->id = destOriginalID;
    }
}

// Queries graph
// Args:
// argv[1] graph name
// argv[2] query to execute
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
    QueryExpressionNode* parseTree = Query_Parse(q, qLen, &errMsg);
    if (!parseTree) {
        RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
        RedisModule_ReplyWithError(ctx, errMsg);
        free(errMsg);
        return REDISMODULE_OK;
    }

    Graph* graph = BuildGraph(parseTree->matchNode);

    QE_FilterNode* filterTree = NULL;
    if(parseTree->whereNode != NULL) {
        filterTree = BuildFiltersTree(parseTree->whereNode->filters);
    }

    Vector* entryPoints = Graph_GetNDegreeNodes(graph, 0);
    Node* startNode = NULL;
    Vector_Pop(entryPoints, &startNode);

    CacheGroupClear(); // Clear group cache before query execution.
    
    // Construct a new empty result-set.
    ResultSet* resultSet = NewResultSet(parseTree);
    QueryNode(ctx, graphName, graph, startNode, entryPoints, filterTree, parseTree, resultSet);

    // Send result-set back to client.
    ResultSet_Replay(ctx, resultSet);

    // Report execution timing
    end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double elapsedMS = elapsed * 1000; 
    char* strElapsed = (char*)malloc(sizeof(char) * strlen("Query internal execution time: milliseconds") + 8);
    sprintf(strElapsed, "Query internal execution time: %f milliseconds", elapsedMS);
    RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
    free(strElapsed);

    // TODO: free filterTree

    // Free AST
    FreeQueryExpressionNode(parseTree);
    ResultSet_Free(ctx, resultSet);

    Vector_Free(entryPoints);
    Graph_Free(graph);
    CacheGroupClear();

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    Agg_RegisterFuncs();

    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(TrieMapType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register triemaptype\n");
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

    return REDISMODULE_OK;
}