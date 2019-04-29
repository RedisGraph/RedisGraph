/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"

#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Saves every entity within the query graph into the actual graph.
 * update statistics regarding the number of entities create and properties set. */
static void _CommitNodes(OpMerge *op, Record r) {
    int labelID;
    Graph *g = op->gc->g;
    Schema *unified_schema = GraphContext_GetUnifiedSchema(op->gc, SCHEMA_NODE);

    const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(op->clause);
    uint entity_count = cypher_ast_pattern_path_nelements(merge_path);

    // Determine how many nodes are specified in MERGE clause
    // Since there are always an odd number of entities in the pattern, there are count/2 + 1 nodes
    uint node_count = (entity_count / 2) + 1;

    // Start by creating nodes.
    Graph_AllocateNodes(g, node_count);

    for(uint i = 0; i < entity_count; i += 2) { // even entities only
        const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(merge_path, i);
        const cypher_astnode_t *ast_label = cypher_ast_node_pattern_get_label(ast_node, 0);
        const char *label = (ast_label) ? cypher_ast_label_get_name(ast_label) : NULL;

        Schema *schema = NULL;

        // Newly created node will be placed within given record.
        Node *n = Record_GetNode(r, i);

        // Set, create label.
        if(label == NULL) {
            labelID = GRAPH_NO_LABEL;
        } else {
            schema = GraphContext_GetSchema(op->gc, label, SCHEMA_NODE);
            /* This is the first time we encounter label, create its schema */
            if(schema == NULL) {
                schema = GraphContext_AddSchema(op->gc, label, SCHEMA_NODE);
                op->result_set->stats.labels_added++;
            }
            labelID = schema->id;
        }

        Graph_CreateNode(g, labelID, n);


        // A CYPHER_AST_MAP node, a CYPHER_AST_PARAMETER node, or null
        // _AddNodeProperties(op, schema, n, op->node_properties[i]);
        // TODO Standardize this, but why is it so different from op create?
        const cypher_astnode_t *props = cypher_ast_node_pattern_get_properties(ast_node);
        if(props) {
            cypher_astnode_type_t prop_type = cypher_astnode_type(props);
            assert(prop_type == CYPHER_AST_MAP); // TODO add parameter support
            uint prop_count = cypher_ast_map_nentries(props);
            for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
                const cypher_astnode_t *ast_key = cypher_ast_map_get_key(props, prop_idx);
                const char *key = cypher_ast_prop_name_get_value(ast_key);

                const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
                // TODO optimize
                AR_ExpNode *value_exp = AR_EXP_FromExpression(op->ast, ast_value);
                SIValue value = AR_EXP_Evaluate(value_exp, NULL);

                Attribute_ID prop_id = ATTRIBUTE_NOTFOUND;
                if(schema) {
                    prop_id = Schema_AddAttribute(schema, SCHEMA_NODE, key);
                } else {
                    prop_id = Schema_AddAttribute(unified_schema, SCHEMA_NODE, key);
                }
                GraphEntity_AddProperty((GraphEntity*)n, prop_id, value);
            }
            // Update tracked schema and add node to any matching indices.
            if(schema) GraphContext_AddNodeToIndices(op->gc, schema, n);
            op->result_set->stats.properties_set += prop_count;
        }
    }

    op->result_set->stats.nodes_created += node_count;
}

static void _CommitEdges(OpMerge *op, Record r) {
    // Create edges.
    Graph *g = op->gc->g;
    const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(op->clause);
    uint entity_count = cypher_ast_pattern_path_nelements(merge_path);

    // Determine how many nodes are specified in MERGE clause
    // Since there are always an odd number of entities in the pattern, there are count/2 nodes
    uint edge_count = entity_count / 2;

    for(uint i = 1; i < entity_count; i += 2) {
        const cypher_astnode_t *ast_rel = cypher_ast_pattern_path_get_element(merge_path, i);
        const cypher_astnode_t *ast_reltype = cypher_ast_rel_pattern_get_reltype(ast_rel, 0);
        const char *reltype = (ast_reltype) ? cypher_ast_reltype_get_name(ast_reltype) : NULL;

        // Newly created edge will be placed within given record.
        Edge *e = Record_GetEdge(r, i);
        Schema *schema = GraphContext_GetSchema(op->gc, reltype, SCHEMA_EDGE);
        if (!schema) schema = GraphContext_AddSchema(op->gc, reltype, SCHEMA_EDGE);

        enum cypher_rel_direction dir = cypher_ast_rel_pattern_get_direction(ast_rel);
        NodeID srcId;
        NodeID destId;
        // Node are already created, get them from record.
        if(dir == CYPHER_REL_OUTBOUND) {
            srcId = ENTITY_GET_ID(Record_GetNode(r, i-1));
            destId = ENTITY_GET_ID(Record_GetNode(r, i+1));
        } else {
            srcId = ENTITY_GET_ID(Record_GetNode(r, i+1));
            destId = ENTITY_GET_ID(Record_GetNode(r, i-1));
        }

        assert(Graph_ConnectNodes(g, srcId, destId, schema->id, e));

        // A CYPHER_AST_MAP node, a CYPHER_AST_PARAMETER node, or null
        const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(ast_rel);
        if(props) {
            // Set edge properties.
            cypher_astnode_type_t prop_type = cypher_astnode_type(props);
            assert(prop_type == CYPHER_AST_MAP); // TODO add parameter support
            uint prop_count = cypher_ast_map_nentries(props);
            for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
                const cypher_astnode_t *ast_key = cypher_ast_map_get_key(props, prop_idx);
                const char *key = cypher_ast_prop_name_get_value(ast_key);

                const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
                // TODO optimize
                AR_ExpNode *value_exp = AR_EXP_FromExpression(op->ast, ast_value);
                SIValue value = AR_EXP_Evaluate(value_exp, NULL);

                Attribute_ID prop_id = prop_id = Schema_AddAttribute(schema, SCHEMA_EDGE, key);
                GraphEntity_AddProperty((GraphEntity*)e, prop_id, value);
            }
            op->result_set->stats.properties_set += prop_count;
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

OpBase* NewMergeOp(GraphContext *gc, const cypher_astnode_t *clause, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));
    op_merge->gc = gc;
    op_merge->ast = AST_GetFromTLS();
    op_merge->clause = clause;
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
        r = Record_New(AST_RecordLength(op->ast));
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
