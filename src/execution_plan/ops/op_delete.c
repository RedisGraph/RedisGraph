/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include <assert.h>

void _LocateEntities(OpDelete *op, QueryGraph *qg, AST_DeleteNode *ast_delete_node) {
    AST *ast = op->ast;
    for(int i = 0; i < Vector_Size(ast_delete_node->graphEntities); i++) {
        char *entity_alias;
        Vector_Get(ast_delete_node->graphEntities, i, &entity_alias);

        // Current entity is a node.
        int entityRecIdx = AST_GetAliasID(ast, entity_alias);
        Node *n = QueryGraph_GetNodeByAlias(qg, entity_alias);
        if (n != NULL) {
            op->nodes_to_delete[op->node_count++] = entityRecIdx;
        } else {
            // Current entity is an edge.
            op->edges_to_delete[op->edge_count++] = entityRecIdx;
        }
    }
}

void _DeleteEntities(OpDelete *op) {
    /* Lock everything. */
    Graph_AcquireWriteLock(op->gc->g);

    /* We must start with edge deletion as node deletion moves nodes around. */
    size_t deletedEdgeCount = array_len(op->deleted_edges);
    for(int i = 0; i < deletedEdgeCount; i++) {
        Edge *e = op->deleted_edges + i;
        if(Graph_DeleteEdge(op->gc->g, e))
            if(op->result_set) op->result_set->stats.relationships_deleted++;
    }

    size_t deletedNodeCount = array_len(op->deleted_nodes);
    for(int i = 0; i < deletedNodeCount; i++) {
        Node *n = op->deleted_nodes + i;
        GraphContext_DeleteNodeFromIndices(op->gc, n);
        Graph_DeleteNode(op->gc->g, n);
        if(op->result_set) op->result_set->stats.nodes_deleted++;
    }

    /* Release lock. */
    Graph_ReleaseLock(op->gc->g);
}

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, GraphContext *gc, ResultSet *result_set) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->gc = gc;
    // op_delete->ast = ast;
    op_delete->node_count = 0;
    op_delete->edge_count = 0;
    op_delete->nodes_to_delete = malloc(sizeof(int) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edges_to_delete = malloc(sizeof(int) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->deleted_nodes = array_new(Node, 32);
    op_delete->deleted_edges = array_new(Edge, 32);
    op_delete->result_set = result_set;
    
    _LocateEntities(op_delete, qg, ast_delete_node);

    // Set our Op operations
    OpBase_Init(&op_delete->op);
    op_delete->op.name = "Delete";
    op_delete->op.type = OPType_DELETE;
    op_delete->op.consume = OpDeleteConsume;
    op_delete->op.reset = OpDeleteReset;
    op_delete->op.free = OpDeleteFree;

    return (OpBase*)op_delete;
}

Record OpDeleteConsume(OpBase *opBase) {
    OpDelete *op = (OpDelete*)opBase;
    OpBase *child = op->op.children[0];

    Record r = child->consume(child);
    if(!r) return NULL;

    /* Enqueue entities for deletion. */
    for(int i = 0; i < op->node_count; i++) {
        Node *n = Record_GetNode(r, op->nodes_to_delete[i]);
        op->deleted_nodes = array_append(op->deleted_nodes, *n);
    }

    for(int i = 0; i < op->edge_count; i++) {
        Edge *e = Record_GetEdge(r, op->edges_to_delete[i]);
        op->deleted_edges = array_append(op->deleted_edges, *e);
    }

    return r;
}

OpResult OpDeleteReset(OpBase *ctx) {
    return OP_OK;
}

void OpDeleteFree(OpBase *ctx) {
    OpDelete *op = (OpDelete*)ctx;

    _DeleteEntities(op);

    free(op->nodes_to_delete);
    free(op->edges_to_delete);
    array_free(op->deleted_nodes);
    array_free(op->deleted_edges);
}
