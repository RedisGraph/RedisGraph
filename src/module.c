#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

#include "parser/ast.h"
#include "parser/grammar.h"
#include "parser/parser_common.h"

#define SCORE 0.0

// Create all 6 triplets from given subject, predicate and object.
// Returns an array of triplets, caller is responsible for freeing each triplet.
RedisModuleString **hexastoreTriplets(RedisModuleCtx *ctx, const RedisModuleString *subject, const RedisModuleString *predicate, const RedisModuleString *object) {
    RedisModuleString** triplets = RedisModule_Alloc(sizeof(RedisModuleString*) * 6);

    size_t sLen = 0;
    size_t oLen = 0;
    size_t pLen = 0;

    const char* s = RedisModule_StringPtrLen(subject, &sLen);
    const char* p = RedisModule_StringPtrLen(predicate, &pLen);
    const char* o = RedisModule_StringPtrLen(object, &oLen);
    
    size_t bufLen = 6 + sLen + pLen + oLen;

    Triplet *triplet = NewTriplet(s, p, o);
    char** permutations = GetTripletPermutations(triplet);
    
    for(int i = 0; i < 6; i++) {
        RedisModuleString *permutation = RedisModule_CreateString(ctx, permutations[i], bufLen);
        triplets[i] = permutation;
        free(permutations[i]);
    }
    
    free(permutations);
    FreeTriplet(triplet);

    return triplets;
}

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

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph, REDISMODULE_WRITE);
    int keytype = RedisModule_KeyType(key);
    
    // Expecting key to be of type empty or sorted set.
    if(keytype != REDISMODULE_KEYTYPE_ZSET && keytype != REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    // Create all 6 hexastore variations
    // SPO, SOP, PSO, POS, OSP, OPS
    RedisModuleString **triplets = hexastoreTriplets(ctx, subject, predicate, object);
    for(int i = 0; i < 6; i++) {
        RedisModuleString *triplet = triplets[i];
        RedisModule_ZsetAdd(key, SCORE, triplet, NULL);
        RedisModule_FreeString(ctx, triplet);
    }

    // Clean up
    RedisModule_Free(triplets);
    size_t newlen = RedisModule_ValueLength(key);
    RedisModule_CloseKey(key);
    RedisModule_ReplyWithLongLong(ctx, newlen);
    
    return REDISMODULE_OK;
}

// Filters given result-set using given filters
Vector* FilterResultSet(RedisModuleCtx *ctx, Vector* resultSet, FilterNode* root, char** aliases) {
    // Construct filter tree.
    QE_FilterNode* filterTree = BuildFiltersTree(root);

    Vector* filteredResultSet = NewVector(Triplet*, 0);

    int resultSetSize = Vector_Size(resultSet);    

    // Foreach result in result set
    for(int i = 0; i < Vector_Size(resultSet); i++) {
        Triplet* result;
        Vector_Get(resultSet, i, &result);

        if(applyFilters(ctx, result, aliases, filterTree)) {
            Vector_Push(filteredResultSet, result);
        } else {
            // Free filtered triplets
            FreeTriplet(result);
        }
    }

    // Free original result set.
    Vector_Free(resultSet);
    return filteredResultSet;
}

Vector* queryTriplet(RedisModuleCtx *ctx, RedisModuleString* graph, const Triplet* triplet) {

    Vector* resultSet = NewVector(Triplet*, 0);
    char* tripletStr = TripletToString(triplet);

    size_t bufLen = strlen(tripletStr) + 2;
    char* buf = (char*)malloc(bufLen);

    // [spo:antirez:is-friend-of: [spo:antirez:is-friend-of:\xff
    // min [spo:antirez:is-friend-of:
    // max [spo:antirez:is-friend-of:\xff

    // min
    sprintf(buf, "[%s", tripletStr);
    RedisModuleString *min = RedisModule_CreateString(ctx, buf, strlen(buf));

    // max
    sprintf(buf, "[%s\xff", tripletStr);
    RedisModuleString *max = RedisModule_CreateString(ctx, buf, bufLen);

    free(tripletStr);
    free(buf);

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph, REDISMODULE_READ);

    if(RedisModule_ZsetFirstInLexRange(key, min, max) == REDISMODULE_ERR) {
        RedisModule_CloseKey(key);
        RedisModule_FreeString(ctx, min);
        RedisModule_FreeString(ctx, max);
        return resultSet;
    }

    do {
        double dScore = 0.0;
        RedisModuleString* element =
            RedisModule_ZsetRangeCurrentElement(key, &dScore);

        if(element) {
            Vector_Push(resultSet, TripletFromString(RedisModule_StringPtrLen(element, 0)));
            RedisModule_FreeString(ctx, element);
        }

    } while(RedisModule_ZsetRangeNext(key));

    RedisModule_FreeString(ctx, min);
    RedisModule_FreeString(ctx, max);
    RedisModule_CloseKey(key);

    return resultSet;
}

// Construct the final response for a query
Vector* BuildQueryResponse(RedisModuleCtx *ctx, ReturnNode* returnNode, Vector* resultSet, char** aliases) {

    Vector* res = NewVector(char*, Vector_Size(resultSet));

    for(int i = 0; i < Vector_Size(resultSet); i++) {
        Triplet* item;
        Vector_Get(resultSet, i, &item);

        // Foreach specified alias:property
        Vector* returnedProps = NewVector(const char*, 0);

        size_t itemLength = 0;
        for(int j = 0; j < Vector_Size(returnNode->variables); j++) {
            VariableNode* var;
            Vector_Get(returnNode->variables, j, &var);


            // Alias maps to subject or object?
            char* elementID = item->subject;
            if(strcmp(aliases[2], var->alias) == 0) {
                elementID = item->object;
            }


            RedisModuleString* keyStr = 
            RedisModule_CreateString(ctx, elementID, strlen(elementID));

            RedisModuleKey *key =
                RedisModule_OpenKey(ctx, keyStr, REDISMODULE_READ);
            
            if(var->property != NULL) {
                RedisModuleString* elementProp =
                    RedisModule_CreateString(ctx, var->property, strlen(var->property));

                RedisModuleString* propValue;
                RedisModule_HashGet(key, REDISMODULE_HASH_NONE, elementProp, &propValue, NULL);
                
                size_t propValueLen;
                const char* prop = 
                    RedisModule_StringPtrLen(propValue, &propValueLen);

                Vector_Push(returnedProps, prop);
                itemLength += propValueLen + 1;

            } else {
                // Couldn't find an API for HGETALL.
            }

            RedisModule_CloseKey(key);
        }

        // Concat strings.

        char* strItem = (char*)malloc(sizeof(char) * itemLength);
        int offset = 0;
        for(int j = 0; j < Vector_Size(returnedProps); j++) {
            const char* prop;            
            Vector_Get(returnedProps, j, &prop);

            strcpy(strItem + offset, prop);
            offset += strlen(prop);
                    
            strItem[offset] = ',';
            offset++;
            strItem[offset] = NULL;
        }

        strItem[strlen(strItem)-1] = NULL;
        Vector_Push(res, strItem);
    }

    return res;
}

void QueryNode(RedisModuleCtx *ctx, RedisModuleString *graphName, Graph* g, Node* n, QE_FilterNode* filterTree) {
    printf("In QueryNode\n");
    printf("node: (%s:%s)\n", n->alias, n->id);
    Node* src = n;

    for(int i = 0; i < Vector_Size(src->outgoingEdges); i++) {
        Edge* edge;
        Vector_Get(src->outgoingEdges, i, &edge);
        Node* dest = edge->dest;

        printf("source node (%s:%s)-[%s]->(%s:%s)\n", src->alias, src->id, edge->relationship, dest->alias, dest->id);
        
        // Create a triplet out of edge
        Triplet* triplet = TripletFromEdge(edge);
        printf("constructed a triplet %s from edge\n", TripletToString(triplet));
        
        // Query graph using triplet, no filters are applied at this stage
        printf("About to query using triplet\n");
        Vector* resultSet = queryTriplet(ctx, graphName, triplet);
        printf("Got a result set for triplet\n");

        FreeTriplet(triplet);

        // Backup original node IDs
        char* srcOriginalID = src->id;
        char* edgeOriginalID = edge->relationship;
        char* destOriginalID = dest->id;

        // Run through result set.
        for(int j = 0; j < Vector_Size(resultSet); j++) {
            // copy result values to graph.
            Triplet* result;
            Vector_Get(resultSet, j, &result);

            printf("Got result: %s\n", (TripletToString(result)));
            
            // Override node IDs.
            src->id = result->subject;
            edge->relationship = result->predicate;
            dest->id = result->object;

            // Advance to next node
            if(Vector_Size(dest->outgoingEdges) > 0) {
                printf("Advancing to next node\n");
                QueryNode(ctx, graphName, g, dest, filterTree);                
            } else {
                printf("See if result pass filters\n");
                // We've reach the end of the graph
                // Pass through filter
                // if(parseTree->whereNode != NULL) {
                
                // Apply filters specified in where clause.
                // Vector* filterResultSet = FilterResultSet(ctx, resultSet, parseTree->whereNode->filters, aliases);
                // if(FilterResultSet(ctx, parseTree->whereNode->filters, g)) {
                    // Get requested fields. 
                    // Vector* response = BuildQueryResponse(ctx, parseTree->returnNode, resultSet, aliases);

                    // Append to final result set.
                // }
            }
        } // End of result-set loop
        
        // Restore nodes original IDs.
        src->id = srcOriginalID;
        edge->relationship = edgeOriginalID;
        dest->id = destOriginalID;
    }
}


int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 2) return RedisModule_WrongArity(ctx);

    RedisModuleString *graphName;
    RedisModuleString *query;
    RMUtil_ParseArgs(argv, argc, 1, "ss", &graphName, &query);
    
    size_t qLen;
    const char* q = RedisModule_StringPtrLen(query, &qLen);

    printf("Processing query: %s\n", q);

    // Parse query, get AST.
    QueryExpressionNode* parseTree = ParseQuery(q, qLen);

    printf("got parseTree\n");

    Graph* graph = BuildGraph(parseTree->matchNode);

    printf("Construct filter tree");
    QE_FilterNode* filterTree = NULL;
    if(parseTree->whereNode != NULL) {
        filterTree = BuildFiltersTree(parseTree->whereNode->filters);
    }
    
    Node* startNode = Graph_GetNDegreeNode(graph, 0);
    QueryNode(ctx, graphName, graph, startNode, filterTree);    

    
    // if(parseTree->whereNode != NULL) {
    //     // Apply filters specified in where clause.
    //     Vector* filterResultSet = FilterResultSet(ctx, resultSet, parseTree->whereNode->filters, aliases);

    //     // TODO: Free unfiltered result set.
    //     resultSet = filterResultSet;
    // }
    
    // // Print final result set.
    // size_t resultSetSize = Vector_Size(resultSet);
    // RedisModule_ReplyWithArray(ctx, resultSetSize);

    // // Return specified properties.    
    // Vector* response = BuildQueryResponse(ctx, parseTree->returnNode, resultSet, aliases);

    // for(int i = 0; i < Vector_Size(response); i++) {
    //     char* result;
    //     Vector_Get(response, i, &result);

    //     RedisModule_ReplyWithStringBuffer(ctx, result, strlen(result));

    //     free(result);
    // }

    // Vector_Free(response);

    // TODO: free memory.
    // RedisModule_FreeString(ctx, graph);
    // Free AST
    // FreeQueryExpressionNode(parseTree);
    // Vector_Free(resultSet);
    // FreeEdge(edge);
    // FreeNode(src);
    // FreeNode(dest);

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    RMUtil_RegisterWriteCmd(ctx, "graph.ADDEDGE", MGraph_AddEdge);
    RMUtil_RegisterWriteCmd(ctx, "graph.QUERY", MGraph_Query);

    return REDISMODULE_OK;
}