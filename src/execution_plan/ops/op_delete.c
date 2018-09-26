/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include <assert.h>

/* Forward declarations. */
void _LocateEntities(OpDelete *op_delete, QueryGraph *graph, AST_DeleteNode *ast_delete_node);

OpBase* NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, const char *graph_name, ResultSet *result_set) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->ctx = ctx;
    op_delete->g = g;
    op_delete->graph_name = graph_name;
    op_delete->qg = qg;
    op_delete->nodes_to_delete = malloc(sizeof(Node**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->node_count = 0;
    op_delete->edge_count = 0;
    op_delete->edges_to_delete = malloc(sizeof(Edge**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->deleted_nodes = array_new(NodeID, 32);
    op_delete->deleted_edges = array_new(EdgeID, 32);
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

void _LocateEntities(OpDelete *op, QueryGraph *qg, AST_DeleteNode *ast_delete_node) {
    for(int i = 0; i < Vector_Size(ast_delete_node->graphEntities); i++) {
        char *entity_alias;
        Vector_Get(ast_delete_node->graphEntities, i, &entity_alias);

        Node *n = QueryGraph_GetNodeByAlias(qg, entity_alias);
        if (n != NULL) {
            Node** node_ref = QueryGraph_GetNodeRef(qg, n);
            op->nodes_to_delete[op->node_count++] = node_ref;
            continue;
        }

        // Save reference to source and destination nodes.
        Edge *e = QueryGraph_GetEdgeByAlias(qg, entity_alias);
        assert(e != NULL);

        Edge **edge_ref = QueryGraph_GetEdgeRef(qg, e);
        op->edges_to_delete[op->edge_count++] = edge_ref;
    }
}

void _DeleteEntities(OpDelete *op) {
    /* We must start with edge deletion as node deletion moves nodes around. */
    size_t deletedEdgeCount = array_len(op->deleted_edges);
    if(deletedEdgeCount > 0) {
        // Sort and remove duplicates.
        size_t dupFreeEdgeIDsCount = SortAndUniqueEntities(op->deleted_edges, deletedEdgeCount);
        assert(dupFreeEdgeIDsCount > 0);
        Graph_DeleteEdges(op->g, op->deleted_edges, dupFreeEdgeIDsCount);
        if(op->result_set) op->result_set->stats.relationships_deleted = dupFreeEdgeIDsCount;
    }

    size_t deletedNodeCount = array_len(op->deleted_nodes);
    if(deletedNodeCount > 0) {
        // Sort and remove duplicates.
        size_t dupFreeNodeIDsCount = SortAndUniqueEntities(op->deleted_nodes, deletedNodeCount);
        assert(dupFreeNodeIDsCount > 0);
        // Remove nodes being deleted from indices
        Indices_DeleteNodes(op->ctx, op->g, op->graph_name, op->deleted_nodes, dupFreeNodeIDsCount);

        // Generate an array of IDs to be migrated
        size_t migration_count = 0;
        EntityID *migrations = Graph_EnqueueMigrations(op->g->nodes, op->deleted_nodes, dupFreeNodeIDsCount, &migration_count);

        // Update indices to assign correct IDs for migrated nodes
        Indices_UpdateNodeIDs(op->ctx, op->g, op->graph_name, migrations, op->deleted_nodes, migration_count);

        // Perform all delete operations on data blocks and matrices
        Graph_DeleteNodes(op->g, op->deleted_nodes, dupFreeNodeIDsCount);
        if(op->result_set) op->result_set->stats.nodes_deleted = dupFreeNodeIDsCount;
    }
}

OpResult OpDeleteConsume(OpBase *opBase, QueryGraph* graph) {
    OpDelete *op = (OpDelete*)opBase;
    OpBase *child = op->op.children[0];

    OpResult res = child->consume(child, graph);
    if(res != OP_OK) return res;

    /* Enqueue entities for deletion. */
    for(int i = 0; i < op->node_count; i++) {
        Node *n = *(op->nodes_to_delete[i]);
        op->deleted_nodes = array_append(op->deleted_nodes, n->id);
    }
    for(int i = 0; i < op->edge_count; i++) {
        Edge *e = *(op->edges_to_delete[i]);
        op->deleted_edges = array_append(op->deleted_edges, e->id);
    }

    return OP_OK;
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
