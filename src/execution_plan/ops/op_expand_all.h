#ifndef __OP_EXPAND_ALL_H
#define __OP_EXPAND_ALL_H

#include "op.h"
#include "../../rmutil/sds.h"
#include "../../hexastore/triplet.h"


/* ExpandAllStates 
 * Different states in which ExpandAll can be at. */
typedef enum {
    ExpandAllUninitialized, /* ExpandAll wasn't initialized it. */
    ExpandAllResetted,      /* ExpandAll was just restarted. */
    ExpandAllConsuming,     /* ExpandAll consuming data. */
} ExpandAllStates;

/* ExpandAll
 * Expands entire graph,
 * Each node within the graph will be set */
typedef struct {
    OpBase op;
    Node **src_node;        /* Source node to expand. */
    Node *_src_node;        /* Original source node to expand. */
    Node **dest_node;       /* Destination node to expand. */
    Node *_dest_node;       /* Original destination node to expand. */
    Edge **relation;        /* Edge to expand. */
    Edge *_relation;        /* Original edge to expand. */
    RedisModuleCtx *ctx;    /* Redis context. */
    HexaStore *hexastore;   /* Graph store. */
    Triplet *triplet;
    Triplet modifies;       /* Which entities does this operation modifies. */
    sds str_triplet;        /* String representation of current triplet. */
    TripletIterator *iter;  /* Graph iterator. */
    ExpandAllStates state;  /* Operation current state. */
} ExpandAll;

/* Creates a new ExpandAll operation */
OpBase* NewExpandAllOp(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                       Node **src_node, Edge **relation, Node **dest_node);

ExpandAll* NewExpandAll(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                        Node **src_node, Edge **relation, Node **dest_node);

/* ExpandAllConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult ExpandAllConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult ExpandAllReset(OpBase *ctx);

/* Frees ExpAndAll*/
void ExpandAllFree(OpBase *ctx);

#endif