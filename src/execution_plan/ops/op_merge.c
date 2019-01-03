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
static void _CommitNodes(OpMerge *op, Record r) {
    int labelID;
    AST_GraphEntity *ge;
    Graph *g = op->gc->g;    
    AST_MergeNode *ast_merge_node = op->ast->mergeNode;
    LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_NODE);
    
    size_t node_count = 0;
    size_t entity_count = Vector_Size(ast_merge_node->graphEntities);

    // Determine how many nodes are specified in MERGE clause.
    for(int i = 0; i < entity_count; i++) {
        Vector_Get(ast_merge_node->graphEntities, i, &ge);
        if(ge->t == N_ENTITY) node_count++;
    }

    // Start by creating nodes.
    Graph_AllocateNodes(g, node_count);

    for(int i = 0; i < entity_count; i++) {
        Vector_Get(ast_merge_node->graphEntities, i, &ge);
        if(ge->t != N_ENTITY) continue;

        AST_NodeEntity *blueprint = ge;
        LabelStore *store = NULL;

        // Newly created node will be placed within given record.
        Node *n = Record_GetNode(r, i);

        // Set, create label.
        if(blueprint->label == NULL) {
            labelID = GRAPH_NO_LABEL; 
        } else {
            store = GraphContext_GetStore(op->gc, blueprint->label, STORE_NODE);
            /* This is the first time we encounter label, create its store */
            if(store == NULL) {
                store = GraphContext_AddLabel(op->gc, blueprint->label);
                op->result_set->stats.labels_added++;
            }
            labelID = store->id;
        }

        Graph_CreateNode(g, labelID, n);

        if(blueprint->properties) {
            int propCount = Vector_Size(blueprint->properties);
            propCount /= 2; // Key value pairs.

            if(propCount > 0) {
                char *keys[propCount];
                SIValue values[propCount];

                for(int prop_idx = 0; prop_idx < propCount; prop_idx++) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(blueprint->properties, prop_idx*2, &key);
                    Vector_Get(blueprint->properties, prop_idx*2+1, &value);

                    keys[prop_idx] = key->stringval;
                    values[prop_idx] = *value;
                }

                GraphEntity_Add_Properties((GraphEntity*)n, propCount, keys, values);
                // Update tracked schema.
                if(store) LabelStore_UpdateSchema(store, propCount, keys);
                LabelStore_UpdateSchema(allStore, propCount, keys);
                op->result_set->stats.properties_set += propCount;
            }
        }
    }

    op->result_set->stats.nodes_created += node_count;
}

static void _CommitEdges(OpMerge *op, Record r) {
    // Create edges.
    Graph *g = op->gc->g;
    AST_GraphEntity *ge;
    AST_MergeNode *ast_merge_node = op->ast->mergeNode;
    size_t edge_count = 0;
    size_t entity_count = Vector_Size(ast_merge_node->graphEntities);

    LabelStore *allStore = GraphContext_AllStore(op->gc, STORE_EDGE);
    for(int i = 0; i < entity_count; i++) {
        Vector_Get(ast_merge_node->graphEntities, i, &ge);
        if(ge->t != N_LINK) continue;
        edge_count++;

        AST_LinkEntity *blueprint = (AST_LinkEntity*)ge;

        // Newly created edge will be placed within given record.
        Edge *e = Record_GetEdge(r, i);

        // TODO: For the timebeing if edge has multiple relationship types, use the first one.
        // ()-[:A|B]->()
        LabelStore *store = GraphContext_GetStore(op->gc, blueprint->labels[0], STORE_EDGE);
        if(store == NULL) {
            store = GraphContext_AddRelationType(op->gc, blueprint->labels[0]);
        }

        NodeID srcId;
        NodeID destId;
        // Node are already created, get them from record.
        if(blueprint->direction == N_LEFT_TO_RIGHT) {
            srcId = ENTITY_GET_ID(Record_GetNode(r, i-1));
            destId = ENTITY_GET_ID(Record_GetNode(r, i+1));
        } else {
            srcId = ENTITY_GET_ID(Record_GetNode(r, i+1));
            destId = ENTITY_GET_ID(Record_GetNode(r, i-1));
        }

        assert(Graph_ConnectNodes(g, srcId, destId, store->id, e));

        // Set edge properties.
        if(blueprint->ge.properties) {
            int propCount = Vector_Size(blueprint->ge.properties);
            propCount /= 2; // Key value pairs.

            if(propCount > 0) {
                char *keys[propCount];
                SIValue values[propCount];

                for(int prop_idx = 0; prop_idx < propCount; prop_idx++) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(blueprint->ge.properties, prop_idx*2, &key);
                    Vector_Get(blueprint->ge.properties, prop_idx*2+1, &value);

                    keys[prop_idx] = key->stringval;
                    values[prop_idx] = *value;
                }

                GraphEntity_Add_Properties((GraphEntity*)e, propCount, keys, values);
                // Update tracked schema.
                LabelStore_UpdateSchema(store, propCount, keys);
                LabelStore_UpdateSchema(allStore, propCount, keys);
                op->result_set->stats.properties_set += propCount;
            }
        }
    }

    op->result_set->stats.relationships_created += edge_count;
}

static void _CreateEntities(OpMerge *op, Record r) {
    // Commit query graph and set resultset statistics.
    _CommitNodes(op, r);
    _CommitEdges(op, r);
}

OpBase* NewMergeOp(GraphContext *gc, AST *ast, QueryGraph *qg, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));
    op_merge->gc = gc;
    op_merge->ast = ast;
    op_merge->qg = qg;
    op_merge->result_set = result_set;
    op_merge->matched = false;
    op_merge->created = false;

    // Set our Op operations
    OpBase_Init(&op_merge->op);
    op_merge->op.name = "Merge";
    op_merge->op.type = OPType_MERGE;
    op_merge->op.consume = OpMergeConsume;
    op_merge->op.reset = OpMergeReset;
    op_merge->op.free = OpMergeFree;

    return (OpBase*)op_merge;
}

Record OpMergeConsume(OpBase *opBase) {
    OpMerge *op = (OpMerge*)opBase;

    /* Pattern was created in the previous call
     * Execution plan is already depleted. */
    if(op->created) return NULL;

    OpBase *child = op->op.children[0];
    Record r = child->consume(child);
    if(r) {
        /* If we're here that means pattern was matched! 
        * in that case there's no need to create any graph entity,
        * we can simply return. */
        op->matched = true;
    } else {
        /* In case there a previous match, execution plan
         * is simply depleted, no need to create the pattern. */
        if(op->matched) return r;

        // No previous match, create MERGE pattern.
        r = Record_New(AST_AliasCount(op->ast));
        _CreateEntities(op, r);
        op->created = true;
    }

    return r;
}

OpResult OpMergeReset(OpBase *ctx) {
    // Merge doesn't modify anything.
    return OP_OK;
}

void OpMergeFree(OpBase *ctx) {
}
