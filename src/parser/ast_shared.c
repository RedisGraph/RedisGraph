#include "ast_shared.h"

AST_Operator AST_ConvertOperatorNode(const cypher_operator_t *op) {
    // TODO ordered by precedence, which I don't know if we're managing properly right now
    if (op == CYPHER_OP_OR) {
        return OP_OR;
    } else if (op == CYPHER_OP_XOR) {
        // TODO implement
        assert(false);
    } else if (op == CYPHER_OP_AND) {
        return OP_AND;
    } else if (op == CYPHER_OP_NOT) { // TODO unary, maybe doesn't belong here
        return OP_NOT;
    } else if (op == CYPHER_OP_EQUAL) {
        return OP_EQUAL;
    } else if (op == CYPHER_OP_NEQUAL) {
        return OP_NEQUAL;
    } else if (op == CYPHER_OP_LT) {
        return OP_LT;
    } else if (op == CYPHER_OP_GT) {
        return OP_GT;
    } else if (op == CYPHER_OP_LTE) {
        return OP_LE;
    } else if (op == CYPHER_OP_GTE) {
        return OP_GE;
    } else if (op == CYPHER_OP_PLUS) {
        return OP_PLUS;
    } else if (op == CYPHER_OP_MINUS) {
        return OP_MINUS;
    } else if (op == CYPHER_OP_MULT) {
        return OP_MULT;
    } else if (op == CYPHER_OP_DIV) {
        return OP_DIV;
    } else if (op == CYPHER_OP_MOD) {
        return OP_MOD;
    } else if (op == CYPHER_OP_POW) {
        return OP_POW;
    }

    return -1;
}

// TODO tmp, memory leak
char* AST_ExpressionToString(const cypher_astnode_t *expr) {
    cypher_astnode_type_t type = cypher_astnode_type(expr);
    // assert (type == CYPHER_AST_EXPRESSION);
    if (type == CYPHER_AST_IDENTIFIER) {
        // Identifier referencing another AST entity
        const char *alias = cypher_ast_identifier_get_name(expr);
        return strdup(alias);
    /* Entity-property pair */
    } else if (type == CYPHER_AST_PROPERTY_OPERATOR) {
        const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(expr);
        assert(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
        const char *alias = cypher_ast_identifier_get_name(prop_expr);
        // Extract the property name
        const cypher_astnode_t *prop_name_node = cypher_ast_property_operator_get_prop_name(expr);
        const char *prop_name = cypher_ast_prop_name_get_value(prop_name_node);
        char *str = strdup(alias);
        str = strcat(str, ".");
        str = strcat(str, prop_name);
        return str;
    } else {
        assert(false);
    }

    return NULL;
}

void PropertyMap_Free(PropertyMap *map) {
    if (map == NULL) return;

    // TODO always freed elsewhere?
    for (uint i = 0; i < map->property_count; i++) {
        // SIValue_Free(&map->values[i]);
    }
    free(map->keys);
    free(map->values);
    free(map);
}

