/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_merge.h"

#include "../../stores/store.h"
#include "op_merge.h"
#include <assert.h>

/* As query graph entities are subject to change during execution of a query 
 * we must maintain a reference to the original entities before execution 
 * such that in the case of a no match on the queried pattern
 * we'll be able to create the original entities. */
void _SaveQueryGraphEntities(const QueryGraph *qg, OpMerge *op) {
    if(qg->node_count) {
        op->nodes = malloc(sizeof(Node*) * qg->node_count);
        memcpy(op->nodes, qg->nodes, sizeof(Node*) * qg->node_count);
    } else {
        op->nodes = NULL;
    }

    if(qg->edge_count) {
        op->edges = malloc(sizeof(Edge*) * qg->edge_count);
        memcpy(op->edges, qg->edges, sizeof(Node*) * qg->edge_count);
    } else {
        op->edges = NULL;
    }
}

OpBase* NewMergeOp(RedisModuleCtx *ctx, AST_Query *ast, Graph *g, QueryGraph *qg, const char *graph_name, ResultSet *result_set) {
    return (OpBase*)NewMerge(ctx, ast, g, qg, graph_name, result_set);
}

OpMerge* NewMerge(RedisModuleCtx *ctx, AST_Query *ast, Graph *g, QueryGraph *qg, const char *graph_name, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));

    op_merge->ctx = ctx;
    op_merge->ast = ast;
    op_merge->g = g;
    op_merge->qg = qg;
    op_merge->result_set = result_set;
    op_merge->request_refresh = true;
    op_merge->graph_name = graph_name;
    op_merge->matched = false;

    _SaveQueryGraphEntities(qg, op_merge);

    // Set our Op operations
    op_merge->op.name = "Merge";
    op_merge->op.type = OPType_MERGE;
    op_merge->op.consume = OpMergeConsume;
    op_merge->op.reset = OpMergeReset;
    op_merge->op.free = OpMergeFree;
    op_merge->op.modifies = NULL;

    return op_merge;
}

OpResult OpMergeConsume(OpBase *opBase, QueryGraph* graph) {
    OpMerge *op = (OpMerge*)opBase;

    if(op->request_refresh) {
        op->request_refresh = false;
        return OP_REFRESH;
    }

    /* If we're here that means pattern was matched! 
     * in that case there's no need to create any graph entity,
     * we can simply return. */
    op->matched = true;
    return OP_DEPLETED;
}

OpResult OpMergeReset(OpBase *ctx) {
    // Merge doesn't modify anything.
    return OP_OK;
}

void _CreateEntities(OpMerge *op) {
    // Restore QueryGraph original entities.
    QueryGraph *qg = op->qg;
    for(int i = 0; i < qg->node_count; i++)
        qg->nodes[i] = op->nodes[i];

    for(int i = 0; i < qg->edge_count; i++)
        qg->edges[i] = op->edges[i];

    // Commit query graph and set resultset statistics.
    op->result_set->stats = CommitGraph(op->ctx, op->qg, op->g, op->graph_name);    
}

void OpMergeFree(OpBase *ctx) {
    OpMerge *op = (OpMerge*)ctx;
    
    if(!op->matched) {
        /* Pattern was not matched, 
         * create every single entity within the pattern. */
        _CreateEntities(op);
    }

    if(op->nodes) free(op->nodes);
    if(op->edges) free(op->edges);
    free(op);
}
