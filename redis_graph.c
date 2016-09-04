#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "./redismodule.h"
#include "triplet.h"
#include "edge.h"
#include "node.h"
#include "rmutil/util.h"
#include "rmutil/vector.h"
#include "rmutil/test_util.h"

#define SCORE 0.0

// Adds a new node to the graph.
// Args:
// argv[1] - Graph name
// argv[2] - Node name
int Graph_AddNode(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if(argc != 3) {
        return RedisModule_WrongArity(ctx);
    }
    
    RedisModuleString *graph = argv[1];
    RedisModuleString *nodeName = argv[2];
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph, REDISMODULE_WRITE);
    
    int keytype = RedisModule_KeyType(key);
    
    // Expecting key to be of type empty or sorted set.
    if(keytype != REDISMODULE_KEYTYPE_ZSET && keytype != REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    RedisModule_ZsetAdd(key, SCORE, nodeName, NULL);
    size_t newlen = RedisModule_ValueLength(key);
    RedisModule_CloseKey(key);
    RedisModule_ReplyWithLongLong(ctx, newlen);
    return REDISMODULE_OK;
}

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
int Graph_AddEdge(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
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

int applyFilter(RedisModuleCtx *ctx, const Filter* filter, const char* elementID, const char* property) {        
    RedisModuleString* keyStr =
        RedisModule_CreateString(ctx, elementID, strlen(elementID));

    RedisModuleString* elementProp =
        RedisModule_CreateString(ctx, property, strlen(property));

    RedisModuleKey *key = RedisModule_OpenKey(ctx, keyStr, REDISMODULE_READ);
    
    if(key == NULL) {
        printf("key %s does not exists\n", elementID);
        RedisModule_FreeString(ctx, keyStr);
        RedisModule_FreeString(ctx, elementProp);
        return 0;
    }

    RedisModuleString* propValue;

    RedisModule_HashGet(key, REDISMODULE_HASH_NONE, elementProp, &propValue, NULL);

    RedisModule_CloseKey(key);
    RedisModule_FreeString(ctx, keyStr);
    RedisModule_FreeString(ctx, elementProp);

    // Cast property value to all 3 filter supported types (int, char* and double)
    const char* strPropValue = RedisModule_StringPtrLen(propValue, 0);
    int nPropValue = atoi(strPropValue);
    double dPropValue = atof(strPropValue);
    RedisModule_FreeString(ctx, propValue);

    // Determine filter type and apply.
    // Eq, Gt, Lt, Between
    switch(filter->op) {
        case EQ:
            switch(filter->eq.v.t) {
                case String:
                    return(strcmp(strPropValue, filter->eq.v.c) == 0);

                case Integer:
                    return (nPropValue == filter->eq.v.n);

                case Float:
                    return (dPropValue == filter->eq.v.f);
            }
            break;

        case GT:
            switch(filter->rng.min.t) {
                case Integer:
                    return (nPropValue > filter->rng.min.n);

                case Float:
                    return (dPropValue > filter->rng.min.f);
            }
            break;

        case GE:
            switch(filter->rng.min.t) {
                case Integer:
                    return (nPropValue >= filter->rng.min.n);

                case Float:
                    return (dPropValue >= filter->rng.min.f);
            }
            break;

        case LT:
            switch(filter->rng.min.t) {
                case Integer:
                    return (nPropValue < filter->rng.max.n);

                case Float:
                    return (dPropValue < filter->rng.max.f);
            }
            break;

        case LE:
            switch(filter->rng.min.t) {
                case Integer:
                    return (nPropValue <= filter->rng.max.n);

                case Float:
                    return (dPropValue <= filter->rng.max.f);
            }
            break;
    } // switch(filter->op)

    return 0;
}

// Filters given result-set using given filters
// only the part (SPO) specified by part is inspected.
Vector* filterResultSet(RedisModuleCtx *ctx, Vector* resultSet, Vector* filters, TripletKind part) {
    Vector* filteredResultSet = NewVector(Triplet*, 0);

    // Foreach result in result set
    for(int i = 0; i < Vector_Size(resultSet); i++) {        
        Triplet* result;
        Vector_Get(resultSet, i, &result);
        int bPassFilters = 1;

        // Applay filter to subject.
        const char* elementID;
        switch(part) {
            case S:
                elementID = result->subject;
                break;
            case P:
                elementID = result->predicate;
                break;
            case O:
                elementID = result->object;
                break;
            default:
                // ERROR
                break;
        }

        // Apply filter to current result
        for (int i = 0; i < Vector_Size(filters); i++) {

            PropertyFilter* propertyFilter;
            Vector_Get(filters, i, &propertyFilter);
            if(!applyFilter(ctx, propertyFilter->filter, elementID, propertyFilter->property)) {
                printf("%s failed to pass filter %s\n", elementID, propertyFilter->property);
                bPassFilters = 0;
                break;
            }
        }

        // Check if current result pass all filters.
        if(bPassFilters) {
            printf("%s passed all filters\n", elementID);
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

int Graph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModuleString *graph = RedisModule_CreateString(ctx, "test_graph", 10);

    // TEST DATA
    Node* src = NewNode("Roi");
    Node* dest = NewNode("");
    Edge* edge = NewEdge(src, dest, "listen");
    NodeAddFilter(src, "age", GreaterThanIntergerFilter(15, 0));
    NodeAddFilter(dest, "genre", EqualStringFilter("Alternative rock"));
    // END OF TEST DATA

    // Create a triplet out of edge
    Triplet* triplet = TripletFromEdge(edge);

    // Query graph using triplet
    Vector* resultSet = queryTriplet(ctx, graph, triplet);

    FreeTriplet(triplet);
    RedisModule_FreeString(ctx, graph);

    resultSet = filterResultSet(ctx, resultSet, src->filters, S);
    resultSet = filterResultSet(ctx, resultSet, dest->filters, O);
    
    // Print final result set.
    for(int i = 0; i < Vector_Size(resultSet); i++) {
        Triplet* result;
        Vector_Get(resultSet, i, &result);
        char* resultStr = TripletToString(result);

        FreeTriplet(result);
        printf("Retrived: %s\n", resultStr);
        free(resultStr);
    }

    printf("result set size: %d\n", Vector_Size(resultSet));
    RedisModule_ReplyWithDouble(ctx, Vector_Size(resultSet));

    Vector_Free(resultSet);
    FreeEdge(edge);
    FreeNode(src);
    FreeNode(dest);

    return REDISMODULE_OK;
}

// Test the the AddNode command
int testAddNode(RedisModuleCtx *ctx) {
    RedisModuleCallReply *r = RedisModule_Call(ctx, "graph.ADDNODE", "cc", "peers", "Fanning");
    RMUtil_Assert(RedisModule_CallReplyType(r) == REDISMODULE_REPLY_INTEGER);
    RMUtil_AssertReplyEquals(r, "1");

    return 0;
}

// Test the the AddNode command
int testAddEdge(RedisModuleCtx *ctx) {

    // Create edge
    RedisModuleCallReply *r = RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", "peers", "Fanning", "cofounder", "Parker");
    RMUtil_Assert(RedisModule_CallReplyType(r) == REDISMODULE_REPLY_INTEGER);
    RMUtil_AssertReplyEquals(r, "6");

    // Check all 6 permutations were created.
    r = RedisModule_Call(ctx, "ZCOUNT", "ccc", "peers", "0", "0");
    RMUtil_AssertReplyEquals(r, "6");

    return 0;
}

int testQuery(RedisModuleCtx *ctx) {
    // Create users
    RedisModule_Call(ctx, "HSET", "ccc", "Roi", "age", "32");
    RedisModule_Call(ctx, "HSET", "ccc", "Hila", "age", "18");

    // Create music bands
    RedisModule_Call(ctx, "HSET", "ccc", "Gorillaz", "genre", "Alternative rock");
    RedisModule_Call(ctx, "HSET", "ccc", "Tool", "genre", "Progressive metal");
    RedisModule_Call(ctx, "HSET", "ccc", "A_perfect_circle", "genre", "Alternative rock");
    RedisModule_Call(ctx, "HSET", "ccc", "Deftones", "genre", "Alternative metal");
    RedisModule_Call(ctx, "HSET", "ccc", "Florance_and_the_machine", "genre", "Indie rock");

    // Populate graph
    const char* graph = "test_graph";

    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "listen", "A_perfect_circle");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "listen", "Tool");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "listen", "Deftones");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "listen", "Gorillaz");

    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Hila", "listen", "Gorillaz");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Hila", "listen", "Florance_and_the_machine");

    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "visit", "Tokyo");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "visit", "California");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "visit", "Tanzania");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Roi", "visit", "Germany");

    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Hila", "visit", "Germany");
    RedisModule_Call(ctx, "graph.ADDEDGE", "cccc", graph, "Hila", "visit", "Tokyo");

    RedisModuleCallReply *r = RedisModule_Call(ctx, "graph.QUERY", "");
    RMUtil_AssertReplyEquals(r, "2");

    return 0;
}

// Unit test entry point for the module
int TestModule(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);

    // RMUtil_Test(testAddNode);
    RMUtil_Test(testAddEdge);
    RMUtil_Test(testQuery);
    
    RedisModule_ReplyWithSimpleString(ctx, "PASS");
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    RMUtil_RegisterWriteCmd(ctx, "graph.ADDNODE", Graph_AddNode);
    RMUtil_RegisterWriteCmd(ctx, "graph.ADDEDGE", Graph_AddEdge);
    RMUtil_RegisterWriteCmd(ctx, "graph.QUERY", Graph_Query);

    // register the unit test
    RMUtil_RegisterWriteCmd(ctx, "graph.TEST", TestModule);

    return REDISMODULE_OK;
}