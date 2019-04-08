/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "newast.h"
#include <assert.h>

#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

extern pthread_key_t _tlsNEWASTKey;  // Thread local storage AST key.

// TODO maybe consolidate with AR_EXP_NewVariableOperandNode
AR_ExpNode* _AR_Exp_NewIdentifier(const char *entity_alias, const cypher_astnode_t *entity, unsigned int id) {
    AR_ExpNode *node = malloc(sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = strdup(entity_alias);
    node->operand.variadic.entity_alias_idx = id;
    node->operand.variadic.entity_prop = NULL;
    node->operand.variadic.ast_ref = entity;

    return node;
}

void _mapReturnAliases(NEWAST *ast) {
    const cypher_astnode_t *return_clause = NEWAST_GetClause(ast->root, CYPHER_AST_RETURN);
    if (!return_clause) return;

    int num_return_projections = cypher_ast_return_nprojections(return_clause);
    if (num_return_projections == 0) return;

    for (unsigned int i = 0; i < num_return_projections; i ++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node == NULL) continue;

        const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);
        const char *alias = cypher_ast_identifier_get_name(alias_node);
        AR_ExpNode *ar_exp = AR_EXP_FromExpression(ast, expr);
        ast->defined_entities = array_append(ast->defined_entities, ar_exp);
        unsigned int *entityID = malloc(sizeof(unsigned int));
        *entityID = ast->identifier_map->cardinality;
        TrieMap_Add(ast->identifier_map, (char*)alias, strlen(alias), entityID, TrieMap_DONT_CARE_REPLACE);
    }
}

// Capture node and relation identifiers from
/* Populate a triemap with node and relation identifiers from MATCH, MERGE, and CREATE clauses
 * (and build anonymous identifiers when necessary). */
void _mapPatternIdentifiers(NEWAST *ast, const cypher_astnode_t *entity, unsigned int *anon_id) {
    if (!entity) return;

    cypher_astnode_type_t type = cypher_astnode_type(entity);

    char *alias = NULL;
    // If the current entity is a node or edge pattern, capture its alias
    if (type == CYPHER_AST_NODE_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(entity);
        if (alias_node) alias = (char*)cypher_ast_identifier_get_name(alias_node);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(entity);
        if (alias_node) alias = (char*)cypher_ast_identifier_get_name(alias_node);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(entity);
        for(unsigned int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
            // Recursively continue searching
            _mapPatternIdentifiers(ast, child, anon_id);
        }
        return;
    }

    if (alias) {
        // Do nothing if entry is already mapped
        if (TrieMap_Find(ast->identifier_map, alias, strlen(alias)) != TRIEMAP_NOTFOUND) return;
    } else {
        // Make identifier for unaliased entities
        // TODO once we're done with dual parsers, can build anon IDs
        // out of triemap cardinality
        asprintf(&alias, "anon_%u", *anon_id);
        (*anon_id)++;
    }
    // The identifier ID is bult from the number of identifiers in the map
    unsigned int id = ast->identifier_map->cardinality;
    // Build an arithmetic expression node representing this identifier
    AR_ExpNode *exp = _AR_Exp_NewIdentifier(alias, entity, id);

    unsigned int *entityID = malloc(sizeof(unsigned int));
    *entityID = id;
    // Store the expression node in an ID-indexed array and
    // store the ID in a triemap.
    ast->defined_entities = array_append(ast->defined_entities, exp);
    TrieMap_Add(ast->identifier_map, alias, strlen(alias), entityID, TrieMap_DONT_CARE_REPLACE);
}

bool NEWAST_ReadOnly(const cypher_astnode_t *query) {
    unsigned int num_clauses = cypher_astnode_nchildren(query);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(query, i);
        cypher_astnode_type_t type = cypher_astnode_type(child);
        if(type == CYPHER_AST_CREATE ||
           type == CYPHER_AST_MERGE ||
           type == CYPHER_AST_DELETE ||
           type == CYPHER_AST_SET ||
           type == CYPHER_AST_CREATE_NODE_PROP_INDEX ||
           type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
            return false;
        }
    }

    return true;
}


bool NEWAST_ContainsClause(const cypher_astnode_t *root, cypher_astnode_type_t clause) {
    return NEWAST_GetClause(root, clause) != NULL;
}

bool NEWAST_ContainsErrors(const cypher_parse_result_t *ast) {
    return cypher_parse_result_nerrors(ast) > 0;
}

char* NEWAST_ReportErrors(const cypher_parse_result_t *ast) {
    char *errorMsg;
    unsigned int nerrors = cypher_parse_result_nerrors(ast);
    for(unsigned int i = 0; i < nerrors; i++) {
        const cypher_parse_error_t *error = cypher_parse_result_get_error(ast, i);

        // Get the position of an error.
        struct cypher_input_position errPos = cypher_parse_error_position(error);

        // Get the error message of an error.
        const char *errMsg = cypher_parse_error_message(error);

        // Get the error context of an error.
        // This returns a pointer to a null-terminated string, which contains a
        // section of the input around where the error occurred, that is limited
        // in length and suitable for presentation to a user.
        const char *errCtx = cypher_parse_error_context(error);

        // Get the offset into the context of an error.
        // Identifies the point of the error within the context string, allowing
        // this to be reported to the user, typically with an arrow pointing to the
        // invalid character.
        size_t errCtxOffset = cypher_parse_error_context_offset(error);

        asprintf(&errorMsg, "errMsg: %s line: %u, column: %u, offset: %zu errCtx: %s errCtxOffset: %zu", errMsg, errPos.line, errPos.column, errPos.offset, errCtx, errCtxOffset);
    }
    return errorMsg;
}

int NEWAST_ReturnClause_ContainsCollapsedNodes(const cypher_astnode_t *ast) {
    const cypher_astnode_t *return_clause = NEWAST_GetClause(ast, CYPHER_AST_RETURN);

    if(!return_clause) return 0;

    unsigned int nprojections = cypher_ast_return_nprojections(return_clause);
    if(nprojections == 0) return 1;

    for (unsigned int i = 0; i < nprojections; i++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
        const cypher_astnode_t *expression = cypher_ast_projection_get_expression(projection);

        // RETURN A
        if(cypher_astnode_type(expression) == CYPHER_AST_IDENTIFIER) return 1;
    }

    return 0;
}

// Retrieve the first instance of the specified clause, if any.
const cypher_astnode_t* NEWAST_GetClause(const cypher_astnode_t *query, cypher_astnode_type_t clause_type) {
    unsigned int num_clauses = cypher_astnode_nchildren(query);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(query, i);
        if (cypher_astnode_type(child) == clause_type) return child;
    }

    return NULL;
}

/* Collect references to all clauses of the specified type in the query. Since clauses
 * cannot be nested, we only need to check the immediate children of the query node. */
unsigned int NewAST_GetTopLevelClauses(const cypher_astnode_t *query, cypher_astnode_type_t clause_type, const cypher_astnode_t **matches) {
    unsigned int num_found = 0;
    unsigned int num_clauses = cypher_astnode_nchildren(query);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(query, i);
        if (cypher_astnode_type(child) != clause_type) continue;

        matches[num_found] = child;
        num_found ++;
    }
    return num_found;
}

const cypher_astnode_t* NEWAST_GetBody(const cypher_parse_result_t *result) {
    const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
    assert(statement && cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

    return cypher_ast_statement_get_body(statement);
}

NEWAST* NEWAST_Build(cypher_parse_result_t *parse_result) {
    NEWAST *new_ast = malloc(sizeof(NEWAST));
    new_ast->root = NEWAST_GetBody(parse_result);
    assert(new_ast->root);
    new_ast->identifier_map = NULL;
    new_ast->order_expression_count = 0;
    new_ast->return_expressions = NULL;
    new_ast->order_expressions = NULL;
    return new_ast;
}

// Debug print
void _walkTriemap(TrieMap *tm) {
    TrieMapIterator *it = TrieMap_Iterate(tm, "", 0);
    char *ptr;
    tm_len_t len;
    void *value;
    while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
        printf("%*s\n", len, ptr);
    }
    TrieMapIterator_Free(it);
}

// TODO I find it so weird that this is necessary
long NEWAST_ParseIntegerNode(const cypher_astnode_t *int_node) {
    assert(int_node);

    const char *value_str = cypher_ast_integer_get_valuestr(int_node);
    return strtol(value_str, NULL, 0);
}

AST_Operator NEWAST_ConvertOperatorNode(const cypher_operator_t *op) {
    // TODO ordered by precedence, which I don't know if we're managing properly right now
    if (op == CYPHER_OP_OR) {
        return OP_OR;
    } else if (op == CYPHER_OP_XOR) {
        // TODO implement
        assert(false);
    } else if (op == CYPHER_OP_AND) {
        return OP_AND;
    } else if (op == CYPHER_OP_NOT) {
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

void NEWAST_BuildAliasMap(NEWAST *ast) {
    ast->identifier_map = NewTrieMap(); // Holds mapping between referred entities and IDs.
    ast->defined_entities = array_new(cypher_astnode_t*, 1);

    // Get graph entity identifiers from MATCH, MERGE, and CREATE clauses.
    unsigned int anon_id = 0;
    _mapPatternIdentifiers(ast, ast->root, &anon_id);

    // Get aliases defined by UNWIND and RETURN...AS clauses
    _mapReturnAliases(ast);
}

unsigned int NEWAST_GetAliasID(const NEWAST *ast, char *alias) {
    assert(ast->identifier_map);
    void *v = TrieMap_Find(ast->identifier_map, alias, strlen(alias));
    assert(v != TRIEMAP_NOTFOUND);
    unsigned int *id = v;
    return *id;
}

AR_ExpNode* NEWAST_GetEntity(const NEWAST *ast, unsigned int id) {
    return ast->defined_entities[id];
}

// TODO preferable to not have this
AR_ExpNode* NEWAST_SeekEntity(const NEWAST *ast, const cypher_astnode_t *entity) {
    unsigned int len = array_len(ast->defined_entities);
    for (unsigned int i = 0; i < len; i ++) {
        AR_ExpNode *check_elem = ast->defined_entities[i];
        if (check_elem->operand.variadic.ast_ref == entity) return ast->defined_entities[i];
    }
    return NULL;
}

size_t NEWAST_AliasCount(const NEWAST *ast) {
    assert(ast);
    return ast->identifier_map->cardinality;
}

NEWAST* NEWAST_GetFromLTS(void) {
    NEWAST *ast = pthread_getspecific(_tlsNEWASTKey);
    assert(ast);
    return ast;
}

