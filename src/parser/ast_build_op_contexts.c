/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include "../../deps/xxhash/xxhash.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

static inline EdgeCreateCtx _NewEdgeCreateCtx(AST *ast, const QueryGraph *qg, const cypher_astnode_t *path, uint edge_path_offset) {
    const cypher_astnode_t *ast_edge = cypher_ast_pattern_path_get_element(path, edge_path_offset);
    const cypher_astnode_t *ast_props = cypher_ast_rel_pattern_get_properties(ast_edge);

    AR_ExpNode *exp = AST_GetEntity(ast, ast_edge);
    // Register entity for Record if necessary
    AST_RecordAccommodateExpression(ast, exp);

    // Get QueryGraph entity
    Edge *e = QueryGraph_GetEntityByASTRef(qg, ast_edge);

    uint src_idx = AST_GetEntityRecordIdx(ast, cypher_ast_pattern_path_get_element(path, edge_path_offset - 1));
    uint dest_idx = AST_GetEntityRecordIdx(ast, cypher_ast_pattern_path_get_element(path, edge_path_offset + 1));
    EdgeCreateCtx new_edge = { .edge = e,
                               .properties = AST_ConvertPropertiesMap(ast, ast_props),
                               .src_idx = src_idx,
                               .dest_idx = dest_idx,
                               .edge_idx = exp->record_idx };
    return new_edge;
}

static inline NodeCreateCtx _NewNodeCreateCtx(AST *ast, const QueryGraph *qg, const cypher_astnode_t *ast_node) {
    Node *n = QueryGraph_GetEntityByASTRef(qg, ast_node);
    const cypher_astnode_t *ast_props = cypher_ast_node_pattern_get_properties(ast_node);

    AR_ExpNode *exp = AST_GetEntity(ast, ast_node);
    // Register entity for Record if necessary
    AST_RecordAccommodateExpression(ast, exp);

    PropertyMap *properties = AST_ConvertPropertiesMap(ast, ast_props);
    NodeCreateCtx new_node = { .node = n, .properties = properties, .node_idx = exp->record_idx };

    return new_node;
}

PropertyMap* AST_ConvertPropertiesMap(const AST *ast, const cypher_astnode_t *props) {
    if (props == NULL) return NULL;
    assert(cypher_astnode_type(props) == CYPHER_AST_MAP); // TODO add parameter support

    uint prop_count = cypher_ast_map_nentries(props);

    PropertyMap *map = malloc(sizeof(PropertyMap));
    map->keys = malloc(prop_count * sizeof(char*));
    map->values = malloc(prop_count * sizeof(SIValue));
    map->property_count = prop_count;

    for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
        const cypher_astnode_t *ast_key = cypher_ast_map_get_key(props, prop_idx);
        map->keys[prop_idx] = cypher_ast_prop_name_get_value(ast_key);

        const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
        // TODO optimize
        AR_ExpNode *value_exp = AR_EXP_FromExpression(ast, ast_value);
        SIValue value = AR_EXP_Evaluate(value_exp, NULL);
        map->values[prop_idx] = value; // TODO mismapping? map->values is an SIValue*
    }
    return map;
}

AR_ExpNode** _AST_ConvertCollection(const cypher_astnode_t *collection) {
    assert(cypher_astnode_type(collection) == CYPHER_AST_COLLECTION);

    AST *ast = AST_GetFromTLS();

    uint expCount = cypher_ast_collection_length(collection);
    AR_ExpNode **expressions = array_new(AR_ExpNode*, expCount);

    for(uint i = 0; i < expCount; i ++) {
        const cypher_astnode_t *exp_node = cypher_ast_collection_get(collection, i);
        AR_ExpNode *exp = AR_EXP_FromExpression(ast, exp_node);
        expressions = array_append(expressions, exp);
    }

    return expressions;
}

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast) {
    int recordsCap = 16;    // Default.
    const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
    if (ret_clause == NULL) return recordsCap;
    // TODO should just store this number somewhere, as this logic is also in resultset
    const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);
    if (limit_clause) {
        int limit = AST_ParseIntegerNode(limit_clause);
        recordsCap = MIN(recordsCap, limit);
    }
    return recordsCap;
}

EntityUpdateEvalCtx* AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, uint *nitems_ref) {
    AST *ast = AST_GetFromTLS();
    uint nitems = cypher_ast_set_nitems(set_clause);
    EntityUpdateEvalCtx *update_expressions = rm_malloc(sizeof(EntityUpdateEvalCtx) * nitems);

    for(uint i = 0; i < nitems; i++) {
        const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
        const cypher_astnode_type_t type = cypher_astnode_type(set_item);
        // TODO Add handling for when we're setting labels (CYPHER_AST_SET_LABELS)
        // or all properties (CYPHER_AST_SET_ALL_PROPERTIES)
        assert(type == CYPHER_AST_SET_PROPERTY);

        // The SET_ITEM contains the entity alias and property key being set
        const cypher_astnode_t *key_to_set = cypher_ast_set_property_get_property(set_item); // type == CYPHER_AST_PROPERTY_OPERATOR
        // Property name
        const cypher_astnode_t *prop = cypher_ast_property_operator_get_prop_name(key_to_set); // type == CYPHER_AST_PROP_NAME
        // Entity alias
        const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(key_to_set);
        AR_ExpNode *entity = AR_EXP_FromExpression(ast, prop_expr);
        // Can this ever be anything strange? Assuming it's always just an alias wrapper right now.
        assert(entity->type == AR_EXP_OPERAND && entity->operand.type == AR_EXP_VARIADIC && entity->operand.variadic.entity_alias);

        // Updated value
        const cypher_astnode_t *val_to_set = cypher_ast_set_property_get_expression(set_item); // type == CYPHER_AST_SET_PROPERTY

        /* Track all required information to perform an update. */
        update_expressions[i].attribute = cypher_ast_prop_name_get_value(prop);
        update_expressions[i].exp = AR_EXP_FromExpression(ast, val_to_set);
        update_expressions[i].entityRecIdx = AST_GetAliasID(ast, entity->operand.variadic.entity_alias);
    }

    *nitems_ref = nitems;
    return update_expressions;
}

void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, uint **nodes_ref, uint **edges_ref) {
    AST *ast = AST_GetFromTLS();
    uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
    uint *nodes_to_delete = array_new(uint, delete_count);
    uint *edges_to_delete = array_new(uint, delete_count);

    for (uint i = 0; i < delete_count; i ++) {
        const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
        assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
        const char *alias = cypher_ast_identifier_get_name(ast_expr);
        AR_ExpNode *entity = AST_GetEntityFromAlias(ast, (char*)alias);
        assert(entity);
        uint id = entity->record_idx;
        assert(id != NOT_IN_RECORD);
        cypher_astnode_type_t type = cypher_astnode_type(entity->operand.variadic.ast_ref);
        if (type == CYPHER_AST_NODE_PATTERN) {
            nodes_to_delete = array_append(nodes_to_delete, id);
        } else if (type == CYPHER_AST_REL_PATTERN) {
            edges_to_delete = array_append(edges_to_delete, id);
        } else {
            assert(false);
        }
    }

    *nodes_ref = nodes_to_delete;
    *edges_ref = edges_to_delete;

}

AR_ExpNode** AST_PrepareSortOp(const cypher_astnode_t *order_clause, int *direction) {
    assert(order_clause);
    AST *ast = AST_GetFromTLS();

    bool ascending = true;
    unsigned int nitems = cypher_ast_order_by_nitems(order_clause);
    AR_ExpNode **order_exps = array_new(AR_ExpNode*, nitems);

    for (unsigned int i = 0; i < nitems; i ++) {
        const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
        const cypher_astnode_t *cypher_exp = cypher_ast_sort_item_get_expression(item);
        AR_ExpNode *exp;
        if (cypher_astnode_type(cypher_exp) == CYPHER_AST_IDENTIFIER) {
            // Reference to an alias in the query - associate with existing AR_ExpNode
            const char *alias = cypher_ast_identifier_get_name(cypher_exp);
            exp = AST_GetEntityFromAlias(ast, (char*)alias);
        } else {
            // Independent operator like:
            // ORDER BY COUNT(a)
            exp = AR_EXP_FromExpression(ast, cypher_exp);
        }

        // TODO rec_idx?
        order_exps = array_append(order_exps, exp);
        // TODO direction should be specifiable per order entity
        ascending = cypher_ast_sort_item_is_ascending(item);
    }

    *direction = ascending ? DIR_ASC : DIR_DESC;

    return order_exps;
}

AST_UnwindContext AST_PrepareUnwindOp(const AST *ast, const cypher_astnode_t *unwind_clause) {
    const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
    AR_ExpNode **exps = _AST_ConvertCollection(collection);
    const char *alias = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));
    uint record_len = AST_RecordLength(ast);
    uint record_idx = AST_GetAliasID(ast, (char*)alias);

    AST_UnwindContext ctx = { .exps = exps, .alias = alias, .record_len = record_len, .record_idx = record_idx };
    return ctx;
}

AST_MergeContext AST_PrepareMergeOp(AST *ast, const cypher_astnode_t *merge_clause, QueryGraph *qg) {
    const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);
    uint path_len = cypher_ast_pattern_path_nelements(path);
    uint nactions = cypher_ast_merge_nactions(merge_clause);

    uint entity_count = cypher_ast_pattern_path_nelements(path);

    NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, (entity_count / 2) + 1);
    EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, entity_count / 2);

    for(uint i = 0; i < entity_count; i ++) {
        const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
        AR_ExpNode *exp = AST_GetEntity(ast, elem);
        // Register entity for Record if necessary
        AST_RecordAccommodateExpression(ast, exp);

        if (i % 2) { // Entity is a relationship
            EdgeCreateCtx new_edge = _NewEdgeCreateCtx(ast, qg, path, i);
            edges_to_merge = array_append(edges_to_merge, new_edge);
        } else { // Entity is a node
            NodeCreateCtx new_node = _NewNodeCreateCtx(ast, qg, cypher_ast_pattern_path_get_element(path, i));
            nodes_to_merge = array_append(nodes_to_merge, new_node);
        }
    }

    uint record_len = AST_RecordLength(ast);
    AST_MergeContext ctx = { .nodes_to_merge = nodes_to_merge, .edges_to_merge = edges_to_merge, .record_len = record_len };
    return ctx;
}


//------------------------------------------------------------------------------
// CREATE operations
//------------------------------------------------------------------------------

// TODO how necessary are these functions? More generic options?
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
TrieMap* _MatchClause_DefinedEntities(const AST *ast) {
    uint match_count = AST_GetClauseCount(ast, CYPHER_AST_MATCH);
    const cypher_astnode_t *match_clauses[match_count];
    AST_GetTopLevelClauses(ast, CYPHER_AST_MATCH, match_clauses);

    uint merge_count = AST_GetClauseCount(ast, CYPHER_AST_MERGE);
    const cypher_astnode_t *merge_clauses[merge_count];
    AST_GetTopLevelClauses(ast, CYPHER_AST_MERGE, merge_clauses);

    TrieMap *map = NewTrieMap();

    for (uint i = 0; i < match_count; i ++) {
        _buildAliasTrieMap(map, match_clauses[i]);
    }

    for (uint i = 0; i < merge_count; i ++) {
        _buildAliasTrieMap(map, merge_clauses[i]);
    }

    return map;
}

AST_CreateContext AST_PrepareCreateOp(AST *ast, QueryGraph *qg) {
    uint create_clause_count = AST_GetClauseCount(ast, CYPHER_AST_CREATE);
    const cypher_astnode_t *create_clauses[create_clause_count];
    AST_GetTopLevelClauses(ast, CYPHER_AST_CREATE, create_clauses);

    /* For every entity within the CREATE clause see if it's also mentioned
     * within the MATCH clause. */
    TrieMap *match_entities = _MatchClause_DefinedEntities(ast);

    NodeCreateCtx *nodes_to_create = array_new(NodeCreateCtx, 1);
    EdgeCreateCtx *edges_to_create = array_new(EdgeCreateCtx, 1);
    uint node_count = 0;
    uint rel_count = 0;

    for (uint i = 0; i < create_clause_count; i ++) {
        const cypher_astnode_t *clause = create_clauses[i];
        const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
        uint npaths = cypher_ast_pattern_npaths(pattern);

        for (uint j = 0; j < npaths; j ++) {
            const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
            uint path_elem_count = cypher_ast_pattern_path_nelements(path);
            for (uint j = 0; j < path_elem_count; j ++) {
                /* See if current entity needs to be created:
                 * 1. current entity is NOT in MATCH clause.
                 * 2. We've yet to account for this entity. */
                const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, j);
                const cypher_astnode_t *ast_alias;
                ast_alias = (j % 2) ? cypher_ast_rel_pattern_get_identifier(elem) :
                                      cypher_ast_node_pattern_get_identifier(elem);

                if (ast_alias) {
                    // Encountered an aliased entity - verify that it is not defined
                    // in a MATCH clause or a previous CREATE pattern
                    const char *alias = cypher_ast_identifier_get_name(ast_alias);

                    // Skip entities defined in MATCH clauses or previously appearing in CREATE patterns
                    int rc = TrieMap_Add(match_entities, (char*)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
                    if (rc == 0) continue;
                }

                if (j % 2) { // Relation
                    EdgeCreateCtx new_edge = _NewEdgeCreateCtx(ast, qg, path, j);
                    edges_to_create = array_append(edges_to_create, new_edge);
                } else { // Node
                    NodeCreateCtx new_node = _NewNodeCreateCtx(ast, qg, elem);
                    nodes_to_create = array_append(nodes_to_create, new_node);
                }
            }
        }
    }

    TrieMap_Free(match_entities, TrieMap_NOP_CB);

    uint record_len = AST_RecordLength(ast);
    AST_CreateContext ctx = { .nodes_to_create = nodes_to_create, .edges_to_create = edges_to_create, .record_len = record_len };

    return ctx;
}

