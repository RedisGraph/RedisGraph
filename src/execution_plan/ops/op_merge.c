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

/* Saves every entity within the query graph into the actual graph.
 * return statistics regarding the number of entities create and properties set. */
static void _CommitNodes(OpMerge *op) {
    const QueryGraph *qg = op->qg;
    Graph *g = op->gc->g;
    size_t node_count = qg->node_count;

    // Start by creating nodes.
    if(node_count > 0) {
        Graph_AllocateNodes(g, node_count);
        
        TrieMap *referred_entities = NewTrieMap();
        MergeClause_ReferredEntities(op->ast->mergeNode, referred_entities);

        int labelID;
        LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_NODE);

        for(int i = 0; i < node_count; i++) {
            Node *n = qg->nodes[i];
            LabelStore *store = NULL;

            // Set, create label.
            if(n->label == NULL) {
               labelID = GRAPH_NO_LABEL; 
            } else {
                store = GraphContext_GetStore(op->gc, n->label, STORE_NODE);
                /* This is the first time we encounter label, create its store */
                if(store == NULL) {
                    store = GraphContext_AddLabel(op->gc, n->label);
                    op->result_set->stats.labels_added++;
                }
                labelID = store->id;
            }

            Graph_CreateNode(g, labelID, n);
            
            AST_NodeEntity *entity = TrieMap_Find(referred_entities, n->alias, strlen(n->alias));
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
                    // Update tracked schema.
                    if(store) LabelStore_UpdateSchema(store, propCount/2, keys);
                    LabelStore_UpdateSchema(allStore, propCount/2, keys);
                    op->result_set->stats.properties_set += propCount/2;
                }
            }
        }

        TrieMap_Free(referred_entities, TrieMap_NOP_CB);
        op->result_set->stats.nodes_created = node_count;
    }
}

static void _CommitEdges(OpMerge *op) {
    // Create edges.
    const QueryGraph *qg = op->qg;
    Graph *g = op->gc->g;
    int relationships_created = 0;
    size_t edge_count = qg->edge_count;

    if(edge_count > 0) {
        TrieMap *referred_entities = NewTrieMap();
        MergeClause_ReferredEntities(op->ast->mergeNode, referred_entities);

        LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_EDGE);
        for(int i = 0; i < edge_count; i++) {
            Edge *e = qg->edges[i];
            NodeID srcId = ENTITY_GET_ID(e->src);
            NodeID destId = ENTITY_GET_ID(e->dest);
            
            LabelStore *store = GraphContext_GetStore(op->gc, e->relationship, STORE_EDGE);
            if(store == NULL) {
                store = GraphContext_AddRelationType(op->gc, e->relationship);
            }
            int r = store->id;
            if(!Graph_ConnectNodes(g, srcId, destId, r, e)) continue;
            
            AST_NodeEntity *entity = TrieMap_Find(referred_entities, e->alias, strlen(e->alias));
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
                    // Update tracked schema.
                    if(store) LabelStore_UpdateSchema(store, propCount/2, keys);
                    LabelStore_UpdateSchema(allStore, propCount/2, keys);
                    op->result_set->stats.properties_set += propCount/2;
                }
            }
            relationships_created++;
        }
        TrieMap_Free(referred_entities, TrieMap_NOP_CB);
        op->result_set->stats.relationships_created = relationships_created;
    }
}

static void _CreateEntities(OpMerge *op) {
    // Commit query graph and set resultset statistics.
    _CommitNodes(op);
    _CommitEdges(op);
}

OpBase* NewMergeOp(GraphContext *gc, AST_Query *ast, QueryGraph *qg, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));
    op_merge->gc = gc;
    op_merge->ast = ast;
    op_merge->qg = qg;
    op_merge->result_set = result_set;
    op_merge->matched = false;

    // Set our Op operations
    OpBase_Init(&op_merge->op);
    op_merge->op.name = "Merge";
    op_merge->op.type = OPType_MERGE;
    op_merge->op.consume = OpMergeConsume;
    op_merge->op.reset = OpMergeReset;
    op_merge->op.free = OpMergeFree;

    return (OpBase*)op_merge;
}

OpResult OpMergeConsume(OpBase *opBase, Record *r) {
    OpMerge *op = (OpMerge*)opBase;

    OpBase *child = op->op.children[0];
    OpResult res = child->consume(child, r);
    if(res == OP_OK) {
        /* If we're here that means pattern was matched! 
        * in that case there's no need to create any graph entity,
        * we can simply return. */
        op->matched = true;
        return OP_DEPLETED;
    }
    return res;
}

OpResult OpMergeReset(OpBase *ctx) {
    // Merge doesn't modify anything.
    return OP_OK;
}

void OpMergeFree(OpBase *ctx) {
    OpMerge *op = (OpMerge*)ctx;
    
    if(!op->matched) {
        /* Pattern was not matched, 
         * create every single entity within the pattern. */
        _CreateEntities(op);
    }
}
