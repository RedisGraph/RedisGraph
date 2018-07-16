/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./op_delete.h"

/* Forward declarations. */
void _LocateEntities(OpDelete *op_delete, QueryGraph *graph, AST_DeleteNode *ast_delete_node);

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set) {
    return (OpBase*)_NewDeleteOp(ast_delete_node, qg, g, result_set);
}

OpDelete* _NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set) {
    OpDelete *op_delete = malloc(sizeof(OpDelete));

    op_delete->g = g;
    op_delete->qg = qg;
    op_delete->request_refresh = 1;
    op_delete->nodes_to_delete = malloc(sizeof(Node**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->node_count = 0;
    op_delete->edges_to_delete = malloc(sizeof(EdgeEnds) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edge_count = 0;
    op_delete->deleted_nodes = NewTrieMap();
    op_delete->deleted_edges = NewTrieMap();
    op_delete->result_set = result_set;
    
    _LocateEntities(op_delete, qg, ast_delete_node);

    // Set our Op operations
    op_delete->op.name = "Delete";
    op_delete->op.type = OPType_DELETE;
    op_delete->op.consume = OpDeleteConsume;
    op_delete->op.reset = OpDeleteReset;
    op_delete->op.free = OpDeleteFree;
    op_delete->op.modifies = NULL;
    return op_delete;
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

        // FOR NOW, SAVE REF TO SRC AND DEST NODES.
        Edge *e = QueryGraph_GetEdgeByAlias(qg, entity_alias);
        if(e != NULL) {
            Edge **edge_ref = QueryGraph_GetEdgeRef(qg, e);
            op->edges_to_delete[op->edge_count].src = QueryGraph_GetNodeRef(op->qg, e->src);
            op->edges_to_delete[op->edge_count].dest = QueryGraph_GetNodeRef(op->qg, e->dest);
            op->edge_count++;
        }
    }
}

void _EnqueueNodeForDeletion(TrieMap *queue, Node *node) {
    char entity_id[128];
    size_t id_len = sprintf(entity_id, "%ld", node->id);
    TrieMap_Add(queue, entity_id, id_len, NULL, TrieMap_DONT_CARE_REPLACE);
}

void _EnqueueEdgeForDeletion(TrieMap *queue, EdgeEnds *edge) {
    char entity_id[256];
    size_t id_len = sprintf(entity_id, "%ld:%ld", (*edge->src)->id, (*edge->dest)->id);
    TrieMap_Add(queue, entity_id, id_len, NULL, TrieMap_DONT_CARE_REPLACE);
}

void _DeleteEntities(OpDelete *op) {
    char ID[128];
    char *prefix;
    tm_len_t prefixLen;
    TrieMapIterator *it;
    void *v;

    /* We must start with edge deletion as node deletion moves nodes around. */    
    it = TrieMap_Iterate(op->deleted_edges, "", 0);
    
    while(TrieMapIterator_Next(it, &prefix, &prefixLen, &v)) {
        memcpy(ID, prefix, prefixLen);
        ID[prefixLen] = 0;
        // Extract src node id and dest node id from id.
        int srcNodeID = atoi(strtok(ID, ":"));
        int destNodeID = atoi(strtok(NULL, ":"));
        Graph_DeleteEdge(op->g, srcNodeID, destNodeID);
    }
    TrieMapIterator_Free(it);    

    if(op->deleted_nodes->cardinality > 0) {
        int *nodesToDelete = malloc(sizeof(int) * op->deleted_nodes->cardinality);
        int i = 0;
        it = TrieMap_Iterate(op->deleted_nodes, "", 0);
        while(TrieMapIterator_Next(it, &prefix, &prefixLen, &v)) {
            nodesToDelete[i++] = atoi(prefix);
        }
        Graph_DeleteNodes(op->g, nodesToDelete, i);
        free(nodesToDelete);
        TrieMapIterator_Free(it);
    }
}

OpResult OpDeleteConsume(OpBase *opBase, QueryGraph* graph) {
    OpDelete *op = (OpDelete*)opBase;

    if(op->request_refresh) {
        op->request_refresh = 0;
        return OP_REFRESH;
    }

    /* Enqueue entities for deletion. */
    char entity_id[256];
    size_t id_len;
    Node *n;
    EdgeEnds *e;

    for(int i = 0; i < op->node_count; i++) {
        n = *(op->nodes_to_delete[i]);        
        _EnqueueNodeForDeletion(op->deleted_nodes, n);
    }
    for(int i = 0; i < op->edge_count; i++) {
        e = op->edges_to_delete + i;
        _EnqueueEdgeForDeletion(op->deleted_edges, e);
    }

    op->request_refresh = 1;
    return OP_OK;
}

OpResult OpDeleteReset(OpBase *ctx) {
    return OP_OK;
}

void OpDeleteFree(OpBase *ctx) {
    OpDelete *op = (OpDelete*)ctx;

    _DeleteEntities(op);
    op->result_set->stats.nodes_deleted = op->deleted_nodes->cardinality;
    op->result_set->stats.relationships_deleted = op->deleted_edges->cardinality;

    free(op->nodes_to_delete);
    free(op->edges_to_delete);
    TrieMap_Free(op->deleted_nodes, TrieMap_NOP_CB);
    TrieMap_Free(op->deleted_edges, TrieMap_NOP_CB);
}
