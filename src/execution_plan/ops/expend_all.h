#ifndef __OP_EXPEND_ALL_H
#define __OP_EXPEND_ALL_H

#include "op.h"

/* ExpendAll
 * Expends entire graph,
 * Each node within the graph will have its
 * ID set */
typedef struct {
    OpBase op;
    Node *srcNode;
    Node *destNode;
    Edge *edge;
    Triplet* triplet;
    HexaStore *hexastore;
    TripletIterator *iter;
} ExpendAll;

/* Creates a new ExpendAll operation */
OpBase* NewExpendAllOp(RedisModuleCtx *ctx, RedisModuleString *graphId, const Graph *graph);
ExpendAll* NewExpendAll(RedisModuleCtx *ctx, RedisModuleString *graphId, const Graph *graph);

/* ExpendAllConsume  next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpendAllConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult ExpendAllReset(OpBase *ctx);

/* Frees ExpendAll*/
void ExpendAllFree(ExpendAll *ctx);

#endif