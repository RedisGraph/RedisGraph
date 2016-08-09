#include "./redismodule.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define SCORE 0.0

// Adds a new node to the graph.
// Args:
// argv[1] - Graph name
// argv[2] - Node name
int Graph_AddNode(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if(argc != 3) {
        return RedisModule_WrongArity(ctx);
    }
    // RedisModule_AutoMemory(ctx);

    RedisModuleString *graph = argv[1];
    RedisModuleString *nodeName = argv[2];
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph, REDISMODULE_WRITE);
    
    int keytype = RedisModule_KeyType(key);
    
    // Expecting key to be of type empty or sorted set.
    if(keytype != REDISMODULE_KEYTYPE_ZSET && keytype != REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    RM_ZsetAdd(key, SCORE, nodeName, NULL);
    size_t newlen = RedisModule_ValueLength(key);
    RedisModule_CloseKey(key);
    RedisModule_ReplyWithLongLong(ctx, newlen);
    return REDISMODULE_OK;
}

// Create all 6 triplets from given subject, predicate and object.
// Returns an array of triplets, caller is responsible for freeing each triplet.
RedisModuleString **hexastoreTriplets(RedisModuleCtx *ctx, const RedisModuleString *subject, const RedisModuleString *predicate, const RedisModuleString *object) {
    
    size_t sLen = 0;
    size_t oLen = 0;
    size_t pLen = 0;
    
    const char* s = RedisModule_StringPtrLen(subject, &sLen);
    const char* p = RedisModule_StringPtrLen(predicate, &pLen);
    const char* o = RedisModule_StringPtrLen(object, &oLen);
    
    size_t bufLen = 6 + sLen + pLen + oLen;
    char *buf = RedisModule_Alloc(bufLen);
    
    RedisModuleString** triplets = RedisModule_Alloc(sizeof(RedisModuleString*) * 6);
    
    // 1. SPO
    strcat(buf, "SPO:");
    strcat(buf, s);
    strcat(buf, ":");
    strcat(buf, p);
    strcat(buf, ":");
    strcat(buf, o);
    RedisModuleString *spo = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[0] = spo;
    memset(buf, 0, bufLen);
    
    // 2. SOP
    strcat(buf, "SOP:");
    strcat(buf, s);
    strcat(buf, ":");
    strcat(buf, o);
    strcat(buf, ":");
    strcat(buf, p);
    RedisModuleString *sop = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[1] = sop;
    memset(buf, 0, bufLen);
    
    // 3. OSP
    strcat(buf, "OSP:");
    strcat(buf, o);
    strcat(buf, ":");
    strcat(buf, s);
    strcat(buf, ":");
    strcat(buf, p);
    RedisModuleString *osp = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[2] = osp;
    memset(buf, 0, bufLen);
    
    // 4. OPS
    strcat(buf, "OPS:");
    strcat(buf, o);
    strcat(buf, ":");
    strcat(buf, p);
    strcat(buf, ":");
    strcat(buf, s);
    RedisModuleString *ops = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[3] = ops;
    memset(buf, 0, bufLen);
    
    // 5. POS
    strcat(buf, "POS:");
    strcat(buf, p);
    strcat(buf, ":");
    strcat(buf, o);
    strcat(buf, ":");
    strcat(buf, s);
    RedisModuleString *pos = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[4] = pos;
    memset(buf, 0, bufLen);
    
    // 6. PSO
    strcat(buf, "PSO:");
    strcat(buf, p);
    strcat(buf, ":");
    strcat(buf, s);
    strcat(buf, ":");
    strcat(buf, o);
    RedisModuleString *pso = RedisModule_CreateString(ctx, buf, bufLen);
    triplets[5] = pso;
    memset(buf, 0, bufLen);
    
    RedisModule_Free(buf);
    
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
    // RedisModule_AutoMemory(ctx);

    RedisModuleString *graph = argv[1];
    RedisModuleString *subject = argv[2];
    RedisModuleString *predicate = argv[3];
    RedisModuleString *object = argv[4];

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
        RM_ZsetAdd(key, SCORE, triplet, NULL);
        RedisModule_FreeString(ctx, triplet);
    }

    // Clean up
    RedisModule_Free(triplets);
    size_t newlen = RedisModule_ValueLength(key);
    RedisModule_CloseKey(key);
    RedisModule_ReplyWithLongLong(ctx, newlen);
    
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    
    if (RedisModule_Init(ctx, "graph", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }
    
    if (RedisModule_CreateCommand(ctx, "graph.ADDNODE",
                                  Graph_AddNode, "write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    
    if (RedisModule_CreateCommand(ctx, "graph.ADDEDGE",
                                  Graph_AddEdge, "write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    
    return REDISMODULE_OK;
}
