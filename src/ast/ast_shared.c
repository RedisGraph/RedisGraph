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

