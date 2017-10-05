#include "./op_delete.h"

/* Forward declarations. */
void _LocateEntities(OpDelete *op_delete, Graph *graph, AST_DeleteNode *ast_delete_node);
void _DeleteEdge(OpDelete *op, Edge *e);

OpBase* NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, Graph *graph,
                    const char *graph_name, ResultSet *result_set) {
    return (OpBase*)_NewDeleteOp(ctx, ast_delete_node, graph, graph_name, result_set);
}

OpDelete* _NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, Graph *graph,
                       const char *graph_name, ResultSet *result_set) {
    OpDelete *op_delete = (OpDelete*) calloc(1, sizeof(OpDelete));

    op_delete->ctx = ctx;
    op_delete->request_refresh = 1;
    op_delete->graph_name = graph_name;    
    op_delete->nodes_to_delete = (Node***)malloc(sizeof(Node**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->node_count = 0;
    op_delete->edges_to_delete = (Edge***)malloc(sizeof(Edge**) * Vector_Size(ast_delete_node->graphEntities));
    op_delete->edge_count = 0;
    op_delete->deleted_nodes = NewTrieMap();
    op_delete->deleted_edges = NewTrieMap();
    op_delete->result_set = result_set;
    
    _LocateEntities(op_delete, graph, ast_delete_node);

    // Set our Op operations
    op_delete->op.name = "Delete";
    op_delete->op.type = OPType_DELETE;
    op_delete->op.consume = OpDeleteConsume;
    op_delete->op.reset = OpDeleteReset;
    op_delete->op.free = OpDeleteFree;
    op_delete->op.modifies = NULL;
    return op_delete;
}

void _LocateEntities(OpDelete *op, Graph *g, AST_DeleteNode *ast_delete_node) {
    for(int i = 0; i < Vector_Size(ast_delete_node->graphEntities); i++) {
        char *entity_alias;
        Vector_Get(ast_delete_node->graphEntities, i, &entity_alias);

        Node *n = Graph_GetNodeByAlias(g, entity_alias);
        if (n != NULL) {
            Node** node_ref = Graph_GetNodeRef(g, n);
            op->nodes_to_delete[op->node_count++] = node_ref;
            continue;
        }
        Edge *e = Graph_GetEdgeByAlias(g, entity_alias);
        if(e != NULL) {
            Edge **edge_ref = Graph_GetEdgeRef(g, e);
            op->edges_to_delete[op->edge_count++] = edge_ref;
        }
    }
}

/* Fake free callback which does absolutly nothing, this is done 
 * so items removed from either node / edge stores do not get freed
 * upon removal. */
void _NOP_CB(void *p) {
}

/* When adding either nodes or edges to deletion trie
 * incase we've already encountered a graph entity, simply use the one
 * already in the trie. */
void* _NO_REPLACE_CB(void *oldval, void *newval) {
    return oldval;
}

void _EnqueueEntityForDeletion(TrieMap *queue, GraphEntity *entity) {
    char entity_id[256];
    size_t id_len = sprintf(entity_id, "%ld", entity->id);
    TrieMap_Add(queue, entity_id, id_len, entity, _NO_REPLACE_CB);
}

void _DeleteNode(OpDelete *op, Node *n) {
    char node_id[256];

    sprintf(node_id, "%ld", n->id);
    RedisModuleCtx *ctx = op->ctx;
    const char *graph_name = op->graph_name;

    /* Remove node from all-node store. */
    Store *all_node_store = GetStore(ctx, STORE_NODE, graph_name, NULL);
    Store_Remove(all_node_store, node_id, _NOP_CB);

    /* Remove node from label store. */
    if(n->label != NULL) {
        Store *all_node_store = GetStore(ctx, STORE_NODE, graph_name, n->label);
        Store_Remove(all_node_store, node_id, _NOP_CB);
    }

    /* Remove node from hexastore. */
    HexaStore *hexa_store = GetHexaStore(ctx, graph_name);
    Triplet t;

    for(int i = 0; i < Vector_Size(n->incoming_edges); i++) {
        Edge *incoming_edge;
        Vector_Get(n->incoming_edges, i, &incoming_edge);
        TripletFromEdge(incoming_edge, &t);
        HexaStore_RemoveAllPerm(hexa_store, &t);
    }
    for(int i = 0; i < Vector_Size(n->outgoing_edges); i++) {
        Edge *outgoing_edge;
        Vector_Get(n->outgoing_edges, i, &outgoing_edge);
        TripletFromEdge(outgoing_edge, &t);
        HexaStore_RemoveAllPerm(hexa_store, &t);
    }

    /* Remove node outgoing edges. */
    for(int i = 0; i < Vector_Size(n->outgoing_edges); i++) {
        Edge *outgoing_edge;
        Vector_Get(n->outgoing_edges, i, &outgoing_edge);
        
        /* Set edge source node to NULL, to prevent _DeleteEdge from 
         * modifying edge's source node. */
        outgoing_edge->src = NULL;
        _EnqueueEntityForDeletion(op->deleted_edges, (GraphEntity*)outgoing_edge);
    }

    /* Remove node incoming edges. */
    for(int i = 0; i < Vector_Size(n->incoming_edges); i++) {
        Edge *incoming_edge;
        Vector_Get(n->incoming_edges, i, &incoming_edge);
        
        /* Set edge destination node to NULL, to prevent _DeleteEdge from 
         * modifying edge's dest node. */
        incoming_edge->dest = NULL;
        _EnqueueEntityForDeletion(op->deleted_edges, (GraphEntity*)incoming_edge);
    }

    /* Free node memory. */
    FreeNode(n);
    op->result_set->nodes_deleted++;
}

void _DeleteEdge(OpDelete *op, Edge *e) {
    char edge_id[256];
    sprintf(edge_id, "%ld", e->id);
    RedisModuleCtx *ctx = op->ctx;
    const char *graph_name = op->graph_name;

    /* Remove edge from all-edge store. */ 
    Store *all_edge_store = GetStore(ctx, STORE_EDGE, graph_name, NULL);
    Store_Remove(all_edge_store, edge_id, _NOP_CB);

    /* Remove edge from relation store. */ 
    Store *edge_store = GetStore(ctx, STORE_EDGE, graph_name, e->relationship);
    Store_Remove(edge_store, edge_id, _NOP_CB);

    /* Remove edge from its source and destination nodes. */
    /* Remove edge from its source node. */
    Node *src_node = e->src;
    if(src_node != NULL) {
        Vector *new_outgoing_edges = NewVector(Edge*, Vector_Size(src_node->outgoing_edges)-1);
        while(Vector_Size(src_node->outgoing_edges) > 0) {
            Edge *outgoing_edge;
            Vector_Pop(src_node->outgoing_edges, &outgoing_edge);
            if(e != outgoing_edge) {
                Vector_Push(new_outgoing_edges, outgoing_edge);
            }
        }
        Vector_Free(src_node->outgoing_edges);
        src_node->outgoing_edges = new_outgoing_edges;
    }
    
    /* Remove edge from its destination node. */
    Node *dest_node = e->dest;
    if(dest_node != NULL) {
        Vector *new_incoming_edges = NewVector(Edge*, Vector_Size(dest_node->incoming_edges)-1);
        while(Vector_Size(dest_node->incoming_edges) > 0) {
            Edge *incoming_edge;
            Vector_Pop(dest_node->incoming_edges, &incoming_edge);
            if(e != incoming_edge) {
                Vector_Push(new_incoming_edges, incoming_edge);
            }
        }
        Vector_Free(dest_node->incoming_edges);
        dest_node->incoming_edges = new_incoming_edges;
    }
    
    /* Remove edge from hexastore. */
    if(e->src && e->dest) {
        Triplet t;
        TripletFromEdge(e, &t);
        HexaStore *hexa_store = GetHexaStore(ctx, graph_name);
        HexaStore_RemoveAllPerm(hexa_store, &t);
    }

    /* Free edge memory. */
    FreeEdge(e);
    op->result_set->relationships_deleted++;
}

void _DeleteEntities(OpDelete *op) {
    char *id;
    tm_len_t id_len;
    Node *n;
    Edge *e;

    TrieMapIterator *it = TrieMap_Iterate(op->deleted_nodes, "", 0);
    while(TrieMapIterator_Next(it, &id, &id_len, (void**)&n)) {
        _DeleteNode(op, n);
    }
    TrieMapIterator_Free(it);

    it = TrieMap_Iterate(op->deleted_edges, "", 0);
    while(TrieMapIterator_Next(it, &id, &id_len, (void**)&e)) {
        _DeleteEdge(op, e);
    }
    TrieMapIterator_Free(it);
}

OpResult OpDeleteConsume(OpBase *opBase, Graph* graph) {
    OpDelete *op = (OpDelete*)opBase;

    if(op->request_refresh) {
        op->request_refresh = 0;
        return OP_REFRESH;
    }

    /* Enqueue entities for deletion. */
    char entity_id[256];
    size_t id_len;
    Node *n;
    Edge *e;

    for(int i = 0; i < op->node_count; i++) {
        n = *(op->nodes_to_delete[i]);
        _EnqueueEntityForDeletion(op->deleted_nodes, (GraphEntity*)n);
    }
    for(int i = 0; i < op->edge_count; i++) {
        e = *(op->edges_to_delete[i]);
        _EnqueueEntityForDeletion(op->deleted_edges, (GraphEntity*)e);
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
    free(op->nodes_to_delete);
    free(op->edges_to_delete);
    TrieMap_Free(op->deleted_nodes, _NOP_CB);
    TrieMap_Free(op->deleted_edges, _NOP_CB);
}