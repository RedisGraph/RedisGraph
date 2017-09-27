#include "../../util/prng.h"
#include "../../stores/store.h"
#include "op_create.h"
#include <assert.h>

/* Forward declarations */
void _EdgesSrcDestAliases(OpCreate *op, Graph* graph);

OpBase* NewCreateOp(RedisModuleCtx *ctx, Graph *graph, const char *graph_name, int request_refresh, ResultSet *result_set) {
    return (OpBase*)NewCreate(ctx, graph, graph_name, request_refresh, result_set);
}

OpCreate* NewCreate(RedisModuleCtx *ctx, Graph *graph, const char *graph_name, int request_refresh, ResultSet *result_set) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));

    op_create->ctx = ctx;
    op_create->graph = graph;
    op_create->graph_name = graph_name;
    op_create->request_refresh = request_refresh;
    
    op_create->nodes_to_create = NULL;
    op_create->node_count = 0;
    op_create->edges_to_create = NULL;
    op_create->edge_count = 0;
    op_create->created_nodes = NewVector(Node*, 0);
    op_create->created_edges = NewVector(Edge*, 0);
    op_create->result_set = result_set;
    
    _EdgesSrcDestAliases(op_create, graph);

    // Set our Op operations
    op_create->op.name = "Create";
    op_create->op.type = OPType_CREATE;
    op_create->op.consume = OpCreateConsume;
    op_create->op.reset = OpCreateReset;
    op_create->op.free = OpCreateFree;
    op_create->op.modifies = NULL;
    return op_create;
}

void _EdgesSrcDestAliases(OpCreate *op, Graph* graph) {
    if(graph->edge_count > 0) {
        op->edges_to_create = (EdgeCreateCtx*)malloc(sizeof(EdgeCreateCtx) * graph->edge_count);
        for(int i = 0; i < graph->edge_count; i++) {
            Edge *e = graph->edges[i];
            op->edges_to_create[i].original_edge = e;
            op->edges_to_create[i].original_edge_ref = &graph->edges[i];
            op->edges_to_create[i].src_node_alias = Graph_GetNodeAlias(graph, e->src);
            op->edges_to_create[i].dest_node_alias = Graph_GetNodeAlias(graph, e->dest);
        }
    }
}

void _SetModifiedEntities(OpCreate *op, Graph* graph) {
    /* Determin which entities are modified by create op.
     * Serach uninitialized nodes. */
    for(int i = 0; i < graph->node_count; i++) {
        Node *n = graph->nodes[i];
        if(n->id == INVALID_ENTITY_ID) {
            op->node_count++;
        }
    }

    /* Remember which nodes get modified by create operation. */
    if(op->node_count > 0) {
        op->nodes_to_create = (NodeCreateCtx*)malloc(sizeof(NodeCreateCtx) * op->node_count);

        int node_idx = 0;
        for(int i = 0; i < graph->node_count; i++) {
            Node *n = graph->nodes[i];
            if(n->id == INVALID_ENTITY_ID) {
                op->nodes_to_create[node_idx].original_node = n;
                op->nodes_to_create[node_idx].original_node_ref = &(graph->nodes[i]);
                node_idx++;
            }
        }
    }

    /* Serach for uninitialized edges. */
    for(int i = 0; i < graph->edge_count; i++) {
        Edge *e = graph->edges[i];
        if(e->id == INVALID_ENTITY_ID) {
            op->edge_count++;
        }
    }

    /* Remember which edges get modified by create operation. */
    if(op->edge_count > 0) {
        EdgeCreateCtx *edges_to_create = (EdgeCreateCtx*)malloc(sizeof(EdgeCreateCtx) * op->edge_count);
        
        int edge_idx = 0;
        for(int i = 0; i < graph->edge_count; i++) {
            Edge *e = *(op->edges_to_create[i].original_edge_ref);
            if(e->id == INVALID_ENTITY_ID) {
                edges_to_create[edge_idx].original_edge = op->edges_to_create[i].original_edge;
                edges_to_create[edge_idx].original_edge_ref = op->edges_to_create[i].original_edge_ref;
                edges_to_create[edge_idx].src_node_alias = op->edges_to_create[i].src_node_alias;
                edges_to_create[edge_idx].dest_node_alias = op->edges_to_create[i].dest_node_alias;
                edge_idx++;
            }
        }
        free(op->edges_to_create);
        op->edges_to_create = edges_to_create;
    }

    /* Create must modify atleast one entity. */
    assert((op->node_count + op->edge_count) > 0);
}

void _CreateNodes(RedisModuleCtx *ctx, OpCreate *op) {
    for(int i = 0; i < op->node_count; i++) {
        /* Get specified node to create. */        
        Node *n = op->nodes_to_create[i].original_node;

        /* Create a new node. */        
        Node *node = NewNode(get_new_id(), n->label);

        /* Add properties.*/
        /* TODO: add all properties in one go. */
        for(int prop_idx = 0; prop_idx < n->prop_count; prop_idx++) {
            EntityProperty prop = n->properties[prop_idx];
            Node_Add_Properties(node, 1, &(prop.name), &(prop.value));
        }

        /* Save node for later insertion. */
        Vector_Push(op->created_nodes, node);

        /* Update query graph with new node. */
        *(op->nodes_to_create[i].original_node_ref) = node;
    }

    /* Nodes creation should only happen once. */
    /* TODO: There's a leak here, original graph node (placeholder) is not released. */
    free(op->nodes_to_create);
    op->node_count = 0;
}

void _CreateEdges(RedisModuleCtx *ctx, OpCreate *op, Graph *graph) {
    for(int i = 0; i < op->edge_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].original_edge;

        /* Retreive source and dest nodes. */        
        Node *src_node = Graph_GetNodeByAlias(graph, op->edges_to_create[i].src_node_alias);
        Node *dest_node = Graph_GetNodeByAlias(graph, op->edges_to_create[i].dest_node_alias);

        /* Create the actual edge. */
        Edge *edge = NewEdge(get_new_id(), src_node, dest_node, e->relationship);

        /* Add properties.*/
        /* TODO: add all properties in one go. */
        for(int prop_idx = 0; prop_idx < e->prop_count; prop_idx++) {
            EntityProperty prop = e->properties[prop_idx];
            Edge_Add_Properties(edge, 1, &(prop.name), &prop.value);
        }        
        
        Node_ConnectNode(src_node, dest_node, edge);
        
        /* Save edge for later insertion. */
        Vector_Push(op->created_edges, edge);

        /* Update query graph with new node. */
        *(op->edges_to_create[i].original_edge_ref) = edge;
    }
}

/* Commit insertions. */
void _CommitNewEntities(OpCreate *op) {
    Store *label_store;
    size_t node_count = Vector_Size(op->created_nodes);
    size_t edge_count = Vector_Size(op->created_edges);
    
    if(node_count > 0) {
        Store *node_store = GetStore(op->ctx, STORE_NODE, op->graph_name, NULL);
        for(int i = 0; i < node_count; i++) {
            Node *n;
            char *node_id;
            
            Vector_Pop(op->created_nodes, &n);

            asprintf(&node_id, "%ld", n->id);

            /* Place node id within node store. */        
            Store_Insert(node_store, node_id, n);

            /* Place node id within labeled node store. */
            if(n->label) {
                /* Store node within label store. */
                label_store = GetStore(op->ctx, STORE_NODE, op->graph_name, n->label);
                Store_Insert(label_store, node_id, n);
                op->result_set->labels_added++;
            }
            op->result_set->properties_set += n->prop_count;
        }
        op->result_set->nodes_created = node_count;
    }

    if(edge_count > 0) {
        HexaStore *hexastore = GetHexaStore(op->ctx, op->graph_name);
        Store *edge_store = GetStore(op->ctx, STORE_EDGE, op->graph_name, NULL);

        for(int i = 0; i < edge_count; i++) {
            Edge *e;
            char *edge_id;

            Vector_Pop(op->created_edges, &e);
            asprintf(&edge_id, "%ld", e->id);

            /* Place edge within edge store(s). */
            Store_Insert(edge_store, edge_id, e);

            label_store = GetStore(op->ctx, STORE_EDGE, op->graph_name, e->relationship);
            Store_Insert(label_store, edge_id, e);

            /* Store relation within hexastore */
            Triplet *triplet = NewTriplet(e->src, e, e->dest);
            HexaStore_InsertAllPerm(hexastore, triplet);
            op->result_set->properties_set += e->prop_count;
        }
        op->result_set->relationships_created = edge_count;
    }
}

OpResult OpCreateConsume(OpBase *opBase, Graph* graph) {
    OpCreate *op = (OpCreate*)opBase;

    if(op->request_refresh) {
        op->request_refresh = 0;
        return OP_REFRESH;
    }

    if(op->node_count == 0 && op->edge_count == 0) {
        _SetModifiedEntities(op, graph);
    }

    /* Create each entity which is missing its ID. */
    _CreateNodes(op->ctx, op);
    _CreateEdges(op->ctx, op, op->graph);
    
    op->request_refresh = 1;
    return OP_OK;
}

OpResult OpCreateReset(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;

    // for(int i = 0; i < op->node_count; i++) {
    //     *(op->nodes_to_create[i].original_node_ref) = op->nodes_to_create[i].original_node;
    // }

    for(int i = 0; i < op->edge_count; i++) {
        *(op->edges_to_create[i].original_edge_ref) = op->edges_to_create[i].original_edge;
    }

    return OP_OK;
}

void OpCreateFree(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    _CommitNewEntities(op);

    // if(op->nodes_to_create != NULL) {
    //     free(op->nodes_to_create);
    // }

    if(op->edges_to_create != NULL) {
        free(op->edges_to_create);
    }
}
