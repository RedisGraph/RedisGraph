/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include <assert.h>

#include "../../deps/xxhash/xxhash.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

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

    bool ascending;
    unsigned int nitems = cypher_ast_order_by_nitems(order_clause);
    AR_ExpNode **order_exps = array_new(AR_ExpNode*, nitems);

    for (unsigned int i = 0; i < nitems; i ++) {
        const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
        const cypher_astnode_t *cypher_exp = cypher_ast_sort_item_get_expression(item);
        AR_ExpNode *ar_exp = AR_EXP_FromExpression(ast, cypher_exp);
        // TODO rec_idx?
        order_exps = array_append(order_exps, ar_exp);
        // TODO direction should be specifiable per order entity
        ascending = cypher_ast_sort_item_is_ascending(item);
    }

    *direction = ascending ? DIR_ASC : DIR_DESC;

    return order_exps;
}

AR_ExpNode** AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause) {
    const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
    AR_ExpNode **exps = _AST_ConvertCollection(collection);

    return exps;
}

