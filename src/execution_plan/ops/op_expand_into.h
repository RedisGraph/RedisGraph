#ifndef __OP_EXPAND_INTO_H
#define __OP_EXPAND_INTO_H

#include "op.h"
#include "../../hexastore/hexastore.h"

/* ExpandInto checks to see if
 * there's a connection between source and destination
 * nodes, it does not changes node's ID.
 **/
typedef struct {
    OpBase op;
    Node *srcNode;
    Node *destNode;
    Edge *relation; // Type of relation we're looking for to exists between nodes.
    int refreshAfterPass;
    RedisModuleCtx *ctx;
    HexaStore *hexaStore;
} ExpandInto;

/* Creates a new ExpandInto operation */
void NewExpandIntoOp(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode, OpBase **op);
ExpandInto* NewExpandInto(RedisModuleCtx *ctx, RedisModuleString *graphId, Node *srcNode, Edge *relation, Node *destNode);

/* ExpandIntoConsume next operation 
 * Returns OP_OK if there's a connection between source and dest nodes
 * OP_DEPLETED is returned when there's no more data to work with. */
OpResult ExpandIntoConsume(OpBase *opBase, Graph* graph);

/* Simply returns OP_OK */
OpResult ExpandIntoReset(OpBase *ctx);

/* Frees ExpandInto*/
void ExpandIntoFree(OpBase *ctx);

#endif // __OP_EXPAND_INTO_H