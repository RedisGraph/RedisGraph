/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "../../util/arr.h"
#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"

#include <assert.h>

void _buildAliasTrieMap(TrieMap *map, const cypher_astnode_t *entity) {
    if (!entity) return;

    cypher_astnode_type_t type = cypher_astnode_type(entity);

    char *alias = NULL;
    if (type == CYPHER_AST_NODE_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(entity);
        if (alias_node) alias = (char*)cypher_ast_identifier_get_name(alias_node);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(entity);
        if (alias_node) alias = (char*)cypher_ast_identifier_get_name(alias_node);
    } else if (type == CYPHER_AST_UNWIND) {
        // The UNWIND clause aliases an expression
        const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(entity);
        assert(alias_node);
        alias = (char*)cypher_ast_identifier_get_name(alias_node);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(entity);
        for(unsigned int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
            // Recursively continue searching
            _buildAliasTrieMap(map, child);
        }
        return;
    }

    if (alias) TrieMap_Add(map, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
}

// TODO This logic doesn't belong here, but might be entirely replaceable - investigate.
TrieMap* _MatchClause_DefinedEntities(NEWAST *ast) {
    uint clause_count = cypher_astnode_nchildren(ast->root);

    const cypher_astnode_t *match_clauses[clause_count];
    uint match_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_MATCH, match_clauses);
    const cypher_astnode_t *merge_clauses[clause_count];
    uint merge_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_MERGE, merge_clauses);

    TrieMap *map = NewTrieMap();

    for (uint i = 0; i < match_count; i ++) {
        _buildAliasTrieMap(map, match_clauses[i]);
    }
    for (uint i = 0; i < merge_count; i ++) {
        _buildAliasTrieMap(map, merge_clauses[i]);
    }

    return map;
}

void _SetModifiedEntities(OpCreate *op) {
    NEWAST *ast = op->ast;
    // TODO bit redundant
    const cypher_astnode_t *create_clause = NEWAST_GetClause(ast->root, CYPHER_AST_CREATE);


    /* For every entity within the CREATE clause see if it's also mentioned
     * within the MATCH clause. */
    TrieMap *match_entities = _MatchClause_DefinedEntities(ast);

    /* Determine which entities are modified by create op. */
    const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(create_clause);
    uint npaths = cypher_ast_pattern_npaths(pattern);
    uint node_count = 0;
    uint rel_count = 0;
    for (uint i = 0; i < npaths; i ++) {
        const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
        uint path_elem_count = cypher_ast_pattern_path_nelements(path);
        uint path_rel_count = path_elem_count / 2;
        rel_count += path_rel_count;
        node_count += path_rel_count + 1;
    }

    op->nodes_to_create = malloc(sizeof(NodeCreateCtx) * node_count);
    op->edges_to_create = malloc(sizeof(EdgeCreateCtx) * rel_count);

    int node_idx = 0;
    int edge_idx = 0;

    for (uint i = 0; i < npaths; i ++) {
        const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
        uint path_elem_count = cypher_ast_pattern_path_nelements(path);
        for (uint j = 0; j < path_elem_count; j ++) {
            /* See if current entity needs to be created:
             * 1. current entity is NOT in MATCH clause.
             * 2. We've yet to accounted for this entity. */
            const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, j);
            const cypher_astnode_t *ast_alias = NULL;
            const char *alias = NULL;
            // if (j % 2) {
                // ast_alias = cypher_ast_rel_pattern_get_identifier(elem);
                // if (!ast_alias) edge_idx ++; // Unaliased entity; must be created
                // continue;
            // } else {
                // ast_alias = cypher_ast__pattern_get_identifier(elem);
                // if (!ast_alias) node_idx ++; // Unaliased entity; must be created
                // continue;

            // }
            ast_alias = (j % 2) ? cypher_ast_rel_pattern_get_identifier(elem) :
                                  cypher_ast_node_pattern_get_identifier(elem);
            if (!ast_alias) {
                // Unaliased entity; find anonymous identifier
                // TODO seek is a slow call; improve this solution
                AR_ExpNode *exp = NEWAST_SeekEntity(ast, elem);
                assert(exp);
                alias = exp->operand.variadic.entity_alias;
            } else {
                alias = cypher_ast_identifier_get_name(ast_alias);
                assert(alias); // TODO possible? Seek again if necessary
            }
            // Skip entities defined in MATCH clauses or previously appearing in CREATE patterns
            int rc = TrieMap_Add(match_entities, (char*)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
            if (rc == 0) continue;

            if (j % 2) { // Relation
                Edge *e = QueryGraph_GetEntityByASTRef(op->qg, elem);
                // TODO revisit all rest of this
                op->edges_to_create[edge_idx].edge = e;
                op->edges_to_create[edge_idx].ast_entity = elem;
                op->edges_to_create[edge_idx].edge_rec_idx = NEWAST_GetAliasID(ast, e->alias);
                op->edges_to_create[edge_idx].src_node_rec_idx = NEWAST_GetAliasID(ast, e->src->alias);
                op->edges_to_create[edge_idx].dest_node_rec_idx = NEWAST_GetAliasID(ast, e->dest->alias);
                edge_idx ++;
            } else { // Node
                Node *n = QueryGraph_GetEntityByASTRef(op->qg, elem);
                op->nodes_to_create[node_idx].ast_entity = elem;
                op->nodes_to_create[node_idx].node = n;
                op->nodes_to_create[node_idx].node_rec_idx = NEWAST_GetAliasID(ast, n->alias);
                node_idx ++;
            }
        }
    }

    TrieMap_Free(match_entities, TrieMap_NOP_CB);

    op->node_count = node_idx;
    op->edge_count = edge_idx;
    /* Create must modify atleast one entity. */
    assert((op->node_count + op->edge_count) > 0);
}

OpBase* NewCreateOp(RedisModuleCtx *ctx, QueryGraph *qg, ResultSet *result_set) {
    OpCreate *op_create = calloc(1, sizeof(OpCreate));
    op_create->gc = GraphContext_GetFromTLS();
    op_create->ast = NEWAST_GetFromTLS();
    op_create->qg = qg;
    op_create->records = NULL;
    op_create->nodes_to_create = NULL;
    op_create->node_count = 0;
    op_create->edges_to_create = NULL;
    op_create->edge_count = 0;
    op_create->created_nodes = array_new(Node*, 0);
    op_create->created_edges = array_new(Edge*, 0);
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

void _CreateNodes(OpCreate *op, Record r) {
    for(int i = 0; i < op->node_count; i++) {
        /* Get specified node to create. */
        Node *n = op->nodes_to_create[i].node;

        /* Create a new node. */
        Node *newNode = Node_New(n->label, n->alias);

        /* Save node for later insertion. */
        op->created_nodes = array_append(op->created_nodes, newNode);

        /* Update record with new node. */
        Record_AddScalar(r, op->nodes_to_create[i].node_rec_idx, SI_PtrVal(newNode));
    }
}

void _CreateEdges(OpCreate *op, Record r) {
    for(int i = 0; i < op->edge_count; i++) {
        /* Get specified edge to create. */
        Edge *e = op->edges_to_create[i].edge;

        /* Retrieve source and dest nodes. */
        Node *src_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].src_node_rec_idx);
        Node *dest_node = (Node*)Record_GetGraphEntity(r, op->edges_to_create[i].dest_node_rec_idx);

        /* Create the actual edge. */
        Edge *newEdge = Edge_New(src_node, dest_node, e->relationship, e->alias);

        /* Save edge for later insertion. */
        op->created_edges = array_append(op->created_edges, newEdge);

        /* Update record with new edge. */
        Record_AddScalar(r, op->edges_to_create[i].edge_rec_idx, SI_PtrVal(newEdge));
    }
}

/* Commit insertions. */
static void _CommitNodes(OpCreate *op) {
    Node *n;
    int labelID;
    Graph *g = op->gc->g;
    Schema *unified_schema = GraphContext_GetUnifiedSchema(op->gc, SCHEMA_NODE);
    NEWAST *ast = NEWAST_GetFromTLS();

    uint node_count = array_len(op->created_nodes);
    Graph_AllocateNodes(op->gc->g, node_count);


    for(uint i = 0; i < node_count; i++) {
        n = op->created_nodes[i];
        Schema *schema = NULL;

        // Get label ID.
        if(n->label == NULL) {
            labelID = GRAPH_NO_LABEL;
            schema = unified_schema;
        } else {
            schema = GraphContext_GetSchema(op->gc, n->label, SCHEMA_NODE);
            if(schema == NULL) {
                schema = GraphContext_AddSchema(op->gc, n->label, SCHEMA_NODE);
                op->result_set->stats.labels_added++;
            }
            labelID = schema->id;
        }

        // Introduce node into graph.
        Graph_CreateNode(g, labelID, n);

        // Retrieve AST reference for node
        uint id = NEWAST_GetAliasID(ast, n->alias);
        AR_ExpNode *exp = NEWAST_GetEntity(ast, id);
        const cypher_astnode_t *ast_entity = exp->operand.variadic.ast_ref;
        const cypher_astnode_t *props = cypher_ast_node_pattern_get_properties(ast_entity);
        if (!props) continue; // No properties specified

        // TODO duplicated logic from op_merge and elsewhere; consolidate
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
            if(n->label) GraphContext_AddNodeToIndices(op->gc, schema, n);
            op->result_set->stats.properties_set += prop_count;
        }
    }

}

static void _CommitEdges(OpCreate *op) {
    Edge *e;
    Graph *g = op->gc->g;
    int relationships_created = 0;

    NEWAST *ast = NEWAST_GetFromTLS();
    uint edge_count = array_len(op->created_edges);
    for(uint i = 0; i < edge_count; i++) {
        e = op->created_edges[i];
        NodeID srcNodeID;

        // Nodes which already existed prior to this query would
        // have their ID set under e->srcNodeID and e->destNodeID
        // Nodes which are created as part of this query would be
        // saved under edge src/dest pointer.
        if(e->srcNodeID != INVALID_ENTITY_ID) srcNodeID = e->srcNodeID;
        else srcNodeID = ENTITY_GET_ID(Edge_GetSrcNode(e));

        NodeID destNodeID;
        if(e->destNodeID != INVALID_ENTITY_ID) destNodeID = e->destNodeID;
        else destNodeID = ENTITY_GET_ID(Edge_GetDestNode(e));

        Schema *schema = GraphContext_GetSchema(op->gc, e->relationship, SCHEMA_EDGE);
        if(!schema) schema = GraphContext_AddSchema(op->gc, e->relationship, SCHEMA_EDGE);
        int relation_id = schema->id;

        if(!Graph_ConnectNodes(g, srcNodeID, destNodeID, relation_id, e)) continue;
        relationships_created++;

        // Retrieve AST reference for edge
        uint id = NEWAST_GetAliasID(ast, e->alias);
        AR_ExpNode *exp = NEWAST_GetEntity(ast, id);
        const cypher_astnode_t *ast_entity = exp->operand.variadic.ast_ref;
        const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(ast_entity);
        if (!props) continue; // No properties specified

        // TODO duplicated logic from op_merge and elsewhere; consolidate
        cypher_astnode_type_t prop_type = cypher_astnode_type(props);
        assert(prop_type == CYPHER_AST_MAP); // TODO add parameter support
        uint prop_count = cypher_ast_map_nentries(props);
        // Set edge properties.
        for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
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

    op->result_set->stats.relationships_created += relationships_created;
}

static void _CommitNewEntities(OpCreate *op) {
    size_t node_count = array_len(op->created_nodes);
    size_t edge_count = array_len(op->created_edges);

    // Lock everything.
    Graph_AcquireWriteLock(op->gc->g);

    if(node_count > 0) {
        _CommitNodes(op);
        op->result_set->stats.nodes_created += node_count;
    }

    if(edge_count > 0) _CommitEdges(op);

    // Release lock.
    Graph_ReleaseLock(op->gc->g);
}

static Record _handoff(OpCreate *op) {
    Record r = NULL;
    if(array_len(op->records)) r = array_pop(op->records);
    return r;
}

Record OpCreateConsume(OpBase *opBase) {
    OpCreate *op = (OpCreate*)opBase;
    Record r;

    // Return mode, all data was consumed.
    if(op->records) return _handoff(op);

    // Consume mode.
    op->records = array_new(Record, 32);

    // No child operation to call.
    if(!op->op.childCount) {
        NEWAST *ast = op->ast;
        r = Record_New(NEWAST_AliasCount(ast));
        /* Create entities. */
        _CreateNodes(op, r);
        _CreateEdges(op, r);

        // Save record for later use.
        op->records = array_append(op->records, r);
    } else {
        // Pull data until child is depleted.
        OpBase *child = op->op.children[0];
        while((r = child->consume(child))) {
            /* Create entities. */
            _CreateNodes(op, r);
            _CreateEdges(op, r);

            // Save record for later use.
            op->records = array_append(op->records, r);
        }
    }

    // Create entities.
    _CommitNewEntities(op);

    // Return record.
    return _handoff(op);
}

OpResult OpCreateReset(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    return OP_OK;
}

void OpCreateFree(OpBase *ctx) {
    OpCreate *op = (OpCreate*)ctx;
    if(op->records) {
        assert(array_len(op->records) == 0);
        array_free(op->records);
    }
    if(op->nodes_to_create) free(op->nodes_to_create);
    if(op->edges_to_create) free(op->edges_to_create);

    if (op->created_nodes) {
        size_t nodeCount = array_len(op->created_nodes);
        for(uint i = 0; i < nodeCount; i++) {
            // It's safe to free node, as its internal GraphEntity wouldn't be free.
            Node_Free(op->created_nodes[i]);
        }
        array_free(op->created_nodes);
    }

    if (op->created_edges) {
        size_t edgeCount = array_len(op->created_edges);
        for(uint i = 0; i < edgeCount; i++) {
            // It's safe to free edge, as its internal GraphEntity wouldn't be free.
            Edge_Free(op->created_edges[i]);
        }
        array_free(op->created_edges);
    }
}
