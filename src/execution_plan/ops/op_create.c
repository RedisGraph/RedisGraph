/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../../stores/store.h"
#include "op_create.h"
#include <assert.h>

void _SetModifiedEntities(OpCreate *op) {
    /* Determin which entities are modified by create op. */
    size_t create_entity_count = Vector_Size(op->ast->createNode->graphEntities);
    op->nodes_to_create = malloc(sizeof(NodeCreateCtx) * create_entity_count);
    op->edges_to_create = malloc(sizeof(EdgeCreateCtx) * create_entity_count);

    int node_idx = 0;
    int edge_idx = 0;

    /* For every entity within the CREATE clause see if it's also mentioned 
     * within the MATCH clause. */
    TrieMap *matchEntities = NewTrieMap();
    MatchClause_ReferredEntities(op->ast->matchNode, matchEntities);

    for(int i = 0; i < create_entity_count; i++) {
        AST_GraphEntity *create_ge;
        Vector_Get(op->ast->createNode->graphEntities, i, &create_ge);
        char *alias = create_ge->alias;

        /* See if current entity needs to be created:
         * 1. current entity is NOT in MATCH clause.
         * 2. We've yet to accounted for this entity. */
        if(TrieMap_Find(matchEntities, alias, strlen(alias)) != TRIEMAP_NOTFOUND) continue;

        // Remember this entity to avoid duplications.
        TrieMap_Add(matchEntities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
        if(create_ge->t == N_ENTITY) {
            // Node.
            Node *n = QueryGraph_GetNodeByAlias(op->qg, create_ge->alias);
            Node **ppn = QueryGraph_GetNodeRef(op->qg, n);
            op->nodes_to_create[node_idx].original_node = n;
            op->nodes_to_create[node_idx].original_node_ref = ppn;
            node_idx++;
        } else {
            // Edge.
            Edge *e = QueryGraph_GetEdgeByAlias(op->qg, create_ge->alias);            
            op->edges_to_create[edge_idx].original_edge = e;
            op->edges_to_create[edge_idx].original_edge_ref = QueryGraph_GetEdgeRef(op->qg, e);
            assert(QueryGraph_ContainsNode(op->qg, e->src));
            assert(QueryGraph_ContainsNode(op->qg, e->dest));
            op->edges_to_create[edge_idx].src_node_alias = e->src->alias;
            op->edges_to_create[edge_idx].dest_node_alias = e->dest->alias;
            edge_idx++;
        }        
    }

    TrieMap_Free(matchEntities, TrieMap_NOP_CB);

    op->node_count = node_idx;
    op->edge_count = edge_idx;
    /* Create must modify atleast one entity. */
    assert((op->node_count + op->edge_count) > 0);
}

OpBase* NewCreateOp(RedisModuleCtx *ctx, GraphContext *gc, AST_Query *ast, QueryGraph *qg, ResultSet *result_set) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = gc;
    op_create->ast = ast;
    op_create->qg = qg;
    
    op_create->nodes_to_create = NULL;
    op_create->node_count = 0;
    op_create->edges_to_create = NULL;
    op_create->edge_count = 0;
    op_create->created_nodes = NewVector(Node*, 0);
    op_create->created_edges = NewVector(Edge*, 0);
    op_create->result_set = result_set;    

    _SetModifiedEntities(op_create);

    // Set our Op operations
    OpBase_Init(&op_create->op);
    op_create->op.name = "Create";
    op_create->op.type = OPType_CREATE;
    op_create->op.consume = OpCreateConsume;
    op_create->op.reset = OpCreateReset;
    op_create->op.free = OpCreateFree;

    return (OpBase*)op_create;
}

void _CreateNodes(OpCreate *op, Record *r) {
    Graph *g = op->gc->g;
    for(int i = 0; i < op->node_count; i++) {
        /* Get specified node to create. */
        Node *n = op->nodes_to_create[i].original_node;

        /* Create a new node. */
        Node *newNode = Node_New(n->label, n->alias);

        /* Save node for later insertion. */
        Vector_Push(op->created_nodes, newNode);

        /* Update record with new node. */
        Record_AddEntry(r, newNode->alias, SI_PtrVal(newNode));
    }
}

void _CreateEdges(OpCreate *op, Record *r) {
    for(int i = 0; i < op->edge_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].original_edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = Record_GetNode(*r, op->edges_to_create[i].src_node_alias);
        Node *dest_node = Record_GetNode(*r, op->edges_to_create[i].dest_node_alias);

        /* Create the actual edge. */
        Edge *newEdge = Edge_New(src_node, dest_node, e->relationship, e->alias);

        /* Save edge for later insertion. */
        Vector_Push(op->created_edges, newEdge);

        /* Update query graph with new edge. */
        Record_AddEntry(r, newEdge->alias, SI_PtrVal(newEdge));
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op) {
    Node *n;
    int labelID;
    Graph *g = op->gc->g;
    size_t node_count = Vector_Size(op->created_nodes);
    TrieMap *createEntities = NewTrieMap();
    LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_NODE);
    
    Graph_AllocateNodes(op->gc->g, node_count);
    CreateClause_ReferredEntities(op->ast->createNode, createEntities);
    
    while(Vector_Pop(op->created_nodes, &n)) {
        LabelStore *store = NULL;

        // Get label ID.
        if(n->label == NULL) {
            labelID = GRAPH_NO_LABEL;
        } else {
            store = GraphContext_GetStore(op->gc, n->label, STORE_NODE);
            if(store == NULL) {
                store = GraphContext_AddLabel(op->gc, n->label);
                op->result_set->stats.labels_added++;
            }
            labelID = store->id;
        }

        // Introduce node into graph.
        Graph_CreateNode(g, labelID, n);
        
        // Set node properties.
        AST_GraphEntity *entity = TrieMap_Find(createEntities, n->alias, strlen(n->alias));
        assert(entity != NULL && entity != TRIEMAP_NOTFOUND);

        if(entity->properties) {
            int propCount = Vector_Size(entity->properties);
            if(propCount > 0) {    
                char *keys[propCount];
                SIValue values[propCount];

                for(int prop_idx = 0; prop_idx < propCount; prop_idx+=2) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(entity->properties, prop_idx, &key);
                    Vector_Get(entity->properties, prop_idx+1, &value);

                    values[prop_idx/2] = *value;
                    keys[prop_idx/2] = key->stringval;
                }

                GraphEntity_Add_Properties((GraphEntity*)n, propCount/2, keys, values);
                if(store) LabelStore_UpdateSchema(store, propCount/2, keys);
                LabelStore_UpdateSchema(allStore, propCount/2, keys);
                op->result_set->stats.properties_set += propCount/2;
            }
        }
    }

    TrieMap_Free(createEntities, TrieMap_NOP_CB);
}

static void _CommitEdges(OpCreate *op) {
    Edge *e;
    int labelID;
    Graph *g = op->gc->g;
    TrieMap *createEntities = NewTrieMap();
    LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_EDGE);
    CreateClause_ReferredEntities(op->ast->createNode, createEntities);

    while(Vector_Pop(op->created_edges, &e)) {
        int relation_id;
        NodeID srcNodeID;

        // Nodes which already existed prior to this query would
        // have their ID set under e->srcNodeID and e->destNodeID
        // Nodes which are created as part of this query would be
        // saved under edge src/dest pointer.
        if(e->srcNodeID != INVALID_ENTITY_ID) srcNodeID = e->srcNodeID;
        else srcNodeID = ENTITY_GET_ID(Edge_GetSrcNode(e));
        
        NodeID destNodeID;
        if(e->srcNodeID != INVALID_ENTITY_ID) destNodeID = e->destNodeID;
        else destNodeID = ENTITY_GET_ID(Edge_GetDestNode(e));

        LabelStore *store = GraphContext_GetStore(op->gc, e->relationship, STORE_EDGE);
        if(!store) store = GraphContext_AddRelationType(op->gc, e->relationship);
        relation_id = store->id;

        Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e);

        // Set edge properties.
        AST_GraphEntity *entity = TrieMap_Find(createEntities, e->alias, strlen(e->alias));
        assert(entity != NULL && entity != TRIEMAP_NOTFOUND);

        if(entity->properties) {
            int propCount = Vector_Size(entity->properties);
            if(propCount > 0) {
                char *keys[propCount];
                SIValue values[propCount];

                for(int prop_idx = 0; prop_idx < propCount; prop_idx+=2) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(entity->properties, prop_idx, &key);
                    Vector_Get(entity->properties, prop_idx+1, &value);

                    values[prop_idx/2] = *value;
                    keys[prop_idx/2] = key->stringval;
                }

                GraphEntity_Add_Properties((GraphEntity*)e, propCount/2, keys, values);
                LabelStore_UpdateSchema(store, propCount/2, keys);
                LabelStore_UpdateSchema(allStore, propCount/2, keys);
                op->result_set->stats.properties_set += propCount/2;
            }
        }
    }

    TrieMap_Free(createEntities, TrieMap_NOP_CB);
}

void _CommitNewEntities(OpCreate *op) {
    size_t node_count = Vector_Size(op->created_nodes);
    size_t edge_count = Vector_Size(op->created_edges);
    LabelStore *allStore;
    
    if(node_count > 0) {
        _CommitNodes(op);
        op->result_set->stats.nodes_created = node_count;
    }
    if(edge_count > 0) {
        _CommitEdges(op);
        op->result_set->stats.relationships_created = edge_count;
    }
}

OpResult OpCreateConsume(OpBase *opBase, Record *r) {
    OpResult res = OP_OK;
    OpCreate *op = (OpCreate*)opBase;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        res = child->consume(child, r);
    }

    if(res == OP_OK) {
        /* Create entities. */
        _CreateNodes(op, r);
        _CreateEdges(op, r);
    }

    if(op->op.childCount == 0) return OP_DEPLETED;
    else return res;
}

OpResult OpCreateReset(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;

    for(int i = 0; i < op->node_count; i++) {
        *(op->nodes_to_create[i].original_node_ref) = op->nodes_to_create[i].original_node;
    }

    for(int i = 0; i < op->edge_count; i++) {
        *(op->edges_to_create[i].original_edge_ref) = op->edges_to_create[i].original_edge;
    }

    return OP_OK;
}

void OpCreateFree(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    _CommitNewEntities(op);

    if(op->nodes_to_create) free(op->nodes_to_create);
    if(op->edges_to_create) free(op->edges_to_create);
    if (op->created_nodes) Vector_Free(op->created_nodes);
    if (op->created_edges) Vector_Free(op->created_edges);
}
