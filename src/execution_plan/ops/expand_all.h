#ifndef __OP_EXPAND_ALL_H
#define __OP_EXPAND_ALL_H

#include "op.h"
#include "../../hexastore/triplet.h"

/* ExpandAll
 * Expands entire graph,
 * Each node within the graph will have its
 * ID set */
typedef struct {
    OpBase op;
    Node *srcNode;
    Node *destNode;
    Edge *relation;
    RedisModuleCtx *ctx;
    HexaStore *hexaStore;
    TripletIterator *iter;
} ExpandAll;

/* Creates a new ExpandAll operation */
OpBase* NewExpandAllOp(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode);
ExpandAll* NewExpandAll(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode);

/* ExpandAllConsume  next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpandAllConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult ExpandAllReset(OpBase *ctx);

/* Frees ExpAndAll*/
void ExpandAllFree(OpBase *ctx);

#endif