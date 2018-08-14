/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./op_delete.h"
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
    op_delete->nodes_to_delete = malloc(sizeof(Node**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->node_count = 0;
    op_delete->edges_to_delete = malloc(sizeof(EdgeEnds) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edge_count = 0;

    op_delete->deleted_node_cap = 1024;
    op_delete->deleted_node_count = 0;
    op_delete->deleted_nodes = malloc(sizeof(NodeID) * op_delete->deleted_node_cap);

    op_delete->deleted_edge_cap = 1024;
    op_delete->deleted_edge_count = 0;
    op_delete->deleted_edges = malloc(sizeof(struct EdgeID) * op_delete->deleted_edge_cap);

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

        // Save reference t source and destination nodes.
        Edge *e = QueryGraph_GetEdgeByAlias(qg, entity_alias);
        assert(e != NULL);

        op->edges_to_delete[op->edge_count].src = QueryGraph_GetNodeRef(op->qg, e->src);
        op->edges_to_delete[op->edge_count].dest = QueryGraph_GetNodeRef(op->qg, e->dest);
        op->edges_to_delete[op->edge_count].relation_type = e->relationship_id;
        op->edge_count++;
    }
}

void _EnqueueNodeForDeletion(OpDelete *op, Node *node) {
    // Make sure we've got enough room for node ID.
    if(op->deleted_node_count >= op->deleted_node_cap) {
        op->deleted_node_cap *= 2;
        op->deleted_nodes = realloc(op->deleted_nodes, sizeof(NodeID) * op->deleted_node_cap);
    }

    op->deleted_nodes[op->deleted_node_count++] = node->id;
}

void _EnqueueEdgeForDeletion(OpDelete *op, EdgeEnds *edge) {
    // Make sure we've got enough room for edge ID.
    if(op->deleted_edge_count >= op->deleted_edge_cap) {
        op->deleted_edge_cap *= 2;
        op->deleted_edges = realloc(op->deleted_edges, sizeof(struct EdgeID) * op->deleted_edge_cap);
    }

    op->deleted_edges[op->deleted_edge_count].src = (*edge->src)->id;
    op->deleted_edges[op->deleted_edge_count].dest = (*edge->dest)->id;
    op->deleted_edges[op->deleted_edge_count].relation_type = edge->relation_type;
    op->deleted_edge_count++;
}

void _DeleteEntities(OpDelete *op) {
    /* We must start with edge deletion as node deletion moves nodes around. */
    if(op->deleted_edge_count > 0) {
        // Sort.
        QSORT(struct EdgeID, op->deleted_edges, op->deleted_edge_count, edgeIDislt);

        for(int i = 0; i < op->deleted_edge_count; i++) {
            struct EdgeID current = op->deleted_edges[i];
            NodeID srcNodeID = current.src;
            NodeID destNodeID = current.dest;

            // Skip duplicates.
            while(i < (op->deleted_edge_count-1) && 
            srcNodeID == op->deleted_edges[i+1].src && 
            destNodeID == op->deleted_edges[i+1].dest ) {
                i++;
            }

            Graph_DeleteEdge(op->g, srcNodeID, destNodeID, current.relation_type);
        }
    }

    if(op->deleted_node_count > 0) {
        // Sort.
        QSORT(NodeID, op->deleted_nodes, op->deleted_node_count, nodeIDislt);
        
        // Remove duplicates.
        int j = 0;  // Index into dup_free_nodes_ids.
        
        // Array of unique node IDs.
        NodeID *dup_free_nodes_ids = malloc(sizeof(NodeID) * op->deleted_node_count);

        for(int i = 0; i < op->deleted_node_count; i++) {
            NodeID current = op->deleted_nodes[i];
            
            // Skip duplicates.
            while(i < (op->deleted_node_count-1) && current == op->deleted_nodes[i+1]) {
                i++;
            }
            
            dup_free_nodes_ids[j++] = current;
        }

        Graph_DeleteNodes(op->g, dup_free_nodes_ids, j);

        free(dup_free_nodes_ids);
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
        _EnqueueNodeForDeletion(op, n);
    }
    for(int i = 0; i < op->edge_count; i++) {
        EdgeEnds *e = op->edges_to_delete + i;
        _EnqueueEdgeForDeletion(op, e);
    }

    return OP_OK;
}

OpResult OpDeleteReset(OpBase *ctx) {
    return OP_OK;
}

void OpDeleteFree(OpBase *ctx) {
    OpDelete *op = (OpDelete*)ctx;

    _DeleteEntities(op);
    if(op->result_set) {
        op->result_set->stats.nodes_deleted = op->deleted_node_count;
        op->result_set->stats.relationships_deleted = op->deleted_edge_count;
    }

    free(op->nodes_to_delete);
    free(op->edges_to_delete);
    free(op->deleted_nodes);
    free(op->deleted_edges);
}
