#ifndef __OP_EXPAND_INTO_H
#define __OP_EXPAND_INTO_H

#include "op.h"
#include "../../rmutil/sds.h"
#include "../../hexastore/hexastore.h"

/* ExpandInto checks to see if
 * there's a connection between source and destination
 * nodes, it does not changes node's ID. */
typedef struct {
    OpBase op;
    Node **src_node;
    Node **dest_node;
    Edge **relation; /* Type of relation we're looking for to exists between nodes. */
    Edge *_relation;
    sds str_triplet;
    int refreshAfterPass;
    RedisModuleCtx *ctx;
    HexaStore *hexastore;
    TripletIterator *iter;  /* Graph iterator. */
} ExpandInto;

/* Creates a new ExpandInto operation */
void NewExpandIntoOp(RedisModuleCtx *ctx, Graph *g, const char *graph_name, Node **src_node,
                     Edge **relation, Node **dest_node, OpBase **op);

ExpandInto* NewExpandInto(RedisModuleCtx *ctx, Graph *g, const char *graph_name, Node **src_node,
                          Edge **relation, Node **dest_node);

/* ExpandIntoConsume next operation 
 * Returns OP_OK if there's a connection between source and dest nodes
 * OP_DEPLETED is returned when there's no more data to work with. */
OpResult ExpandIntoConsume(OpBase *opBase, Graph* graph);

/* Simply returns OP_OK */
OpResult ExpandIntoReset(OpBase *ctx);

/* Frees ExpandInto*/
void ExpandIntoFree(OpBase *ctx);

#endif // __OP_EXPAND_INTO_H