/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include <assert.h>

#define nodeIDislt(a,b) (a<b)

#define edgeIDislt(a,b) ( ( a->src < b->src ) ||\
    ( ( a->src == b->src ) && ( a->dest < b->dest ) ) ||\
    ( ( a->src == b->src ) && ( a->dest == b->dest ) && ( a->relation_type < b->relation_type ) ) )

/* Forward declarations. */
void _LocateEntities(OpDelete *op_delete, QueryGraph *graph, AST_DeleteNode *ast_delete_node);

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->g = g;
    op_delete->qg = qg;
    op_delete->node_count = 0;
    op_delete->edge_count = 0;
    op_delete->nodes_to_delete = malloc(sizeof(char*) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edges_to_delete = malloc(sizeof(char*) * Vector_Size(ast_delete_node->graphEntities));
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

        // Current entity is a node.
        Node *n = QueryGraph_GetNodeByAlias(qg, entity_alias);
        if (n != NULL) {
            op->nodes_to_delete[op->node_count++] = entity_alias;
            continue;
        }

        // Current entity is an edge.
        op->edges_to_delete[op->edge_count++] = entity_alias;
    }
}

EntityID *_SortNRemoveDups(EntityID *entities, size_t entityCount, size_t *dupFreeCount) {
    // Sort.
    QSORT(EntityID, entities, entityCount, ENTITY_ID_ISLT);

    // Remove duplicates.
    int j = 0;  // Index into dup_free_ids.

    // Array of unique IDs.
    EntityID *dup_free_ids = malloc(sizeof(EntityID) * entityCount);

    for(int i = 0; i < entityCount; i++) {
        EntityID current = entities[i];

        // Skip duplicates.
        while(i < (entityCount-1) && current == entities[i+1]) i++;

        dup_free_ids[j++] = current;
    }

    *dupFreeCount = j;
    return dup_free_ids;
}

void _DeleteEntities(OpDelete *op) {
    /* We must start with edge deletion as node deletion moves nodes around. */
    size_t deletedEdgeCount = array_len(op->deleted_edges);
    if(deletedEdgeCount > 0) {
        // Sort and remove duplicates.
        size_t dupFreeEdgeIDsCount = 0;
        EntityID *dupFreeEdgeIDs = _SortNRemoveDups(op->deleted_edges, deletedEdgeCount, &dupFreeEdgeIDsCount);
        assert(dupFreeEdgeIDsCount > 0);
        Graph_DeleteEdges(op->g, dupFreeEdgeIDs, dupFreeEdgeIDsCount);
        if(op->result_set) op->result_set->stats.relationships_deleted = dupFreeEdgeIDsCount;
        free(dupFreeEdgeIDs);
    }

    size_t deletedNodeCount = array_len(op->deleted_nodes);
    if(deletedNodeCount > 0) {
        // Sort and remove duplicates.
        size_t dupFreeNodeIDsCount = 0;
        EntityID *dupFreeNodeIDs = _SortNRemoveDups(op->deleted_nodes, deletedNodeCount, &dupFreeNodeIDsCount);
        assert(dupFreeNodeIDsCount > 0);
        Graph_DeleteNodes(op->g, dupFreeNodeIDs, dupFreeNodeIDsCount);
        if(op->result_set) op->result_set->stats.nodes_deleted = dupFreeNodeIDsCount;
        free(dupFreeNodeIDs);
    }                
}

OpResult OpDeleteConsume(OpBase *opBase, Record *r) {
    OpDelete *op = (OpDelete*)opBase;
    OpBase *child = op->op.children[0];

    OpResult res = child->consume(child, r);
    if(res != OP_OK) return res;

    /* Enqueue entities for deletion. */
    char *alias = NULL;
    for(int i = 0; i < op->node_count; i++) {
        alias = op->nodes_to_delete[i];
        Node *n = Record_GetNode(*r, alias);
        op->deleted_nodes = array_append(op->deleted_nodes, n->id);
    }

    for(int i = 0; i < op->edge_count; i++) {
        alias = op->edges_to_delete[i];
        Edge *e = Record_GetEdge(*r, alias);
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
