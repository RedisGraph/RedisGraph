/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"

#include "../../schema/schema.h"
#include "op_merge.h"
#include <assert.h>

/* Saves every entity within the query graph into the actual graph.
 * update statistics regarding the number of entities create and properties set. */
static void _CommitNodes(OpMerge *op, Record r) {
    int labelID;
    AST_GraphEntity *ge;
    Graph *g = op->gc->g;
    AST_MergeNode *ast_merge_node = op->ast->mergeNode;
    Schema *unified_schema = GraphContext_GetUnifiedSchema(op->gc, SCHEMA_NODE);
    
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
        Schema *schema = NULL;

        // Newly created node will be placed within given record.
        Node *n = Record_GetNode(r, i);

        // Set, create label.
        if(blueprint->label == NULL) {
            labelID = GRAPH_NO_LABEL; 
        } else {
            schema = GraphContext_GetSchema(op->gc, blueprint->label, SCHEMA_NODE);
            /* This is the first time we encounter label, create its schema */
            if(schema == NULL) {
                schema = GraphContext_AddSchema(op->gc, blueprint->label, SCHEMA_NODE);
                op->result_set->stats.labels_added++;
            }
            labelID = schema->id;
        }

        Graph_CreateNode(g, labelID, n);

        if(blueprint->properties) {
            int propCount = Vector_Size(blueprint->properties);
            propCount /= 2; // Key value pairs.

            if(propCount > 0) {
                for(int prop_idx = 0; prop_idx < propCount; prop_idx++) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(blueprint->properties, prop_idx*2, &key);
                    Vector_Get(blueprint->properties, prop_idx*2+1, &value);

                    Attribute_ID prop_id = ATTRIBUTE_NOTFOUND;
                    if(schema) prop_id = Schema_AddAttribute(schema, SCHEMA_NODE, key->stringval);
                    else prop_id = Schema_AddAttribute(unified_schema, SCHEMA_NODE, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)n, prop_id, *value);
                }
                // Update tracked schema and add node to any matching indices.
                if(schema) GraphContext_AddNodeToIndices(op->gc, schema, n);
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

    for(int i = 0; i < entity_count; i++) {
        Vector_Get(ast_merge_node->graphEntities, i, &ge);
        if(ge->t != N_LINK) continue;
        edge_count++;

        AST_LinkEntity *blueprint = (AST_LinkEntity*)ge;

        // Newly created edge will be placed within given record.
        Edge *e = Record_GetEdge(r, i);
        Schema *schema = GraphContext_GetSchema(op->gc, blueprint->labels[0], SCHEMA_EDGE);
        if(!schema) schema = GraphContext_AddSchema(op->gc, blueprint->labels[0], SCHEMA_EDGE);

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

        assert(Graph_ConnectNodes(g, srcId, destId, schema->id, e));

        // Set edge properties.
        if(blueprint->ge.properties) {
            int propCount = Vector_Size(blueprint->ge.properties);
            propCount /= 2; // Key value pairs.

            if(propCount > 0) {
                for(int prop_idx = 0; prop_idx < propCount; prop_idx++) {
                    SIValue *key;
                    SIValue *value;
                    Vector_Get(blueprint->ge.properties, prop_idx*2, &key);
                    Vector_Get(blueprint->ge.properties, prop_idx*2+1, &value);

                    Attribute_ID prop_id = Schema_AddAttribute(schema, SCHEMA_EDGE, key->stringval);
                    GraphEntity_AddProperty((GraphEntity*)e, prop_id, *value);
                }
                op->result_set->stats.properties_set += propCount;
            }
        }
    }

    op->result_set->stats.relationships_created += edge_count;
}

static void _CreateEntities(OpMerge *op, Record r) {
    // Lock everything.
    Graph_AcquireWriteLock(op->gc->g);

    // Commit query graph and set resultset statistics.
    _CommitNodes(op, r);
    _CommitEdges(op, r);

    // Release lock.
    Graph_ReleaseLock(op->gc->g);
}

OpBase* NewMergeOp(GraphContext *gc, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));
    op_merge->gc = gc;
    // AST *ast = AST_GetFromTLS();
    // op_merge->ast = ast;
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
