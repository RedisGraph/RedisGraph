/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <assert.h>

#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

// Note each function call within given expression
// Example: given the expression: "abs(max(min(a), abs(k)))"
// referred_funcs will include: "abs", "max" and "min".
static void _consume_function_call_expression(const cypher_astnode_t *expression, TrieMap *referred_funcs) {
    // Value is an apply operator
    const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(expression);
    const char *func_name = cypher_ast_function_name_get_value(func);
    TrieMap_Add(referred_funcs, (char*)func_name, strlen(func_name), NULL, TrieMap_DONT_CARE_REPLACE);

    unsigned int narguments = cypher_ast_apply_operator_narguments(expression);
    for(int i = 0; i < narguments; i++) {
        const cypher_astnode_t *child_exp = cypher_ast_apply_operator_get_argument(expression, i);
        cypher_astnode_type_t child_exp_type = cypher_astnode_type(child_exp);
        if(child_exp_type != CYPHER_AST_APPLY_OPERATOR) continue;
        _consume_function_call_expression(child_exp, referred_funcs);
    }
}


// TODO maybe consolidate with AR_EXP_NewVariableOperandNode
AR_ExpNode* _AR_Exp_NewIdentifier(const char *entity_alias, const cypher_astnode_t *entity, unsigned int id) {
    AR_ExpNode *node = malloc(sizeof(AR_ExpNode));
    node->type = AR_EXP_OPERAND;
    node->collapsed = true;
    node->record_idx = id;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = entity_alias ? strdup(entity_alias) : NULL;
    node->operand.variadic.entity_alias_idx = id;
    node->operand.variadic.entity_prop = NULL;
    node->operand.variadic.ast_ref = entity;

    return node;
}

void _mapWith(AST *ast, const cypher_astnode_t *with_clause) {
    // TODO logic duplicated from AST_BuildWithExpressions
    unsigned int count = cypher_ast_with_nprojections(with_clause);
    AR_ExpNode **with_expressions = array_new(AR_ExpNode*, count);
    for (unsigned int i = 0; i < count; i++) {
        const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
        const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);

        AR_ExpNode *exp = NULL;
        char *identifier = NULL;

        if (cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
            // Retrieve "a" from "with a" or "with a AS e"
            identifier = (char*)cypher_ast_identifier_get_name(expr);
            exp = AST_GetEntityFromAlias(ast, (char*)identifier);
        }

        if (exp == NULL) {
            // Identifier did not appear in previous clauses.
            // It may be a constant or a function call (or other?)
            // Create a new entity to represent it.
            exp = AR_EXP_FromExpression(ast, expr);

            // Make space for entity in record
            unsigned int id = AST_AddRecordEntry(ast);
            AR_EXP_AssignRecordIndex(exp, id);
            // Add entity to the set of entities to be populated
            ast->defined_entities = array_append(ast->defined_entities, exp);
            AST_MapEntity(ast, projection, exp);
        }

        // If the projection is aliased, add the alias to mappings and Record
        char *alias = NULL;
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node) {
            // TODO ?
            // The projection either has an alias (AS) or is a function call.
            alias = (char*)cypher_ast_identifier_get_name(alias_node);
            // TODO can the alias have appeared in an earlier clause?

            // Associate alias with the expression
            AST_MapAlias(ast, alias, exp);
            exp->alias = alias;
            with_expressions = array_append(with_expressions, exp);
        } else {
            exp->alias = identifier;
            with_expressions = array_append(with_expressions, exp);
        }
    }
}

void _mapUnwind(AST *ast, const cypher_astnode_t *unwind_clause) {
    const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(unwind_clause);
    char *alias = (char*)cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

    const cypher_astnode_t *ast_exp = cypher_ast_unwind_get_expression(unwind_clause);
    // AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(ast, ast_exp, alias, NULL);
    // exp->record_idx = AST_AddRecordEntry(ast);
    // exp->collapsed = false;
    // exp->alias = (char*)alias;
    // AST_MapEntity(ast, ast_exp, exp);
    // AST_MapAlias(ast, (char*)alias, exp);
    // ast->defined_entities = array_append(ast->defined_entities, exp);
    uint id = AST_AddRecordEntry(ast);
    AR_ExpNode *exp = AR_EXP_NewReferenceNode(alias, id, false);
    ast->defined_entities = array_append(ast->defined_entities, exp);
    AST_MapAlias(ast, alias, exp);

}

bool AST_ReadOnly(const AST *ast) {
    unsigned int num_clauses = cypher_astnode_nchildren(ast->root);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(ast->root, i);
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

bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause) {
    return AST_GetClause(ast, clause) != NULL;
}

bool AST_ContainsErrors(const cypher_parse_result_t *result) {
    return cypher_parse_result_nerrors(result) > 0;
}

char* AST_ReportErrors(const cypher_parse_result_t *result) {
    char *errorMsg;
    unsigned int nerrors = cypher_parse_result_nerrors(result);
    for(unsigned int i = 0; i < nerrors; i++) {
        const cypher_parse_error_t *error = cypher_parse_result_get_error(result, i);

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

// Recursively collect the names of all function calls beneath a node
void AST_ReferredFunctions(const cypher_astnode_t *root, TrieMap *referred_funcs) {
    cypher_astnode_type_t root_type = cypher_astnode_type(root);
    if(root_type == CYPHER_AST_APPLY_OPERATOR) {
        _consume_function_call_expression(root, referred_funcs);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(root);
        for(int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
            AST_ReferredFunctions(child, referred_funcs);
        }
    }
}


// Retrieve the first instance of the specified clause in range, if any.
const cypher_astnode_t* AST_GetClause(const AST *ast, cypher_astnode_type_t clause_type) {
    for (unsigned int i = ast->start_offset; i < ast->end_offset; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(ast->root, i);
        if (cypher_astnode_type(child) == clause_type) return child;
    }

    return NULL;
}

/* Collect references to all clauses of the specified type in the query. Since clauses
 * cannot be nested, we only need to check the immediate children of the query node. */
unsigned int AST_GetTopLevelClauses(const AST *ast, cypher_astnode_type_t clause_type, const cypher_astnode_t **matches) {
    unsigned int num_found = 0;
    unsigned int num_clauses = cypher_astnode_nchildren(ast->root);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(ast->root, i);
        if (cypher_astnode_type(child) != clause_type) continue;

        matches[num_found] = child;
        num_found ++;
    }
    return num_found;
}

uint* AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type) {
    uint *clause_indices = array_new(uint, 0);
    unsigned int num_clauses = cypher_astnode_nchildren(ast->root);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        if (cypher_astnode_type(cypher_astnode_get_child(ast->root, i)) == clause_type) {
            clause_indices = array_append(clause_indices, i);
        }
    }
    return clause_indices;
}

uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type) {
    unsigned int num_clauses = cypher_astnode_nchildren(ast->root);
    unsigned int num_found = 0;
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(ast->root, i);
        if (cypher_astnode_type(child) == clause_type) num_found ++;

    }
    return num_found;
}

uint AST_NumClauses(const AST *ast) {
    return cypher_astnode_nchildren(ast->root);
}

// TODO should just use this format or the TopLevel format
const cypher_astnode_t** AST_CollectReferencesInRange(const AST *ast, cypher_astnode_type_t type) {
    const cypher_astnode_t **found = array_new(const cypher_astnode_t *, 0);

    for (uint i = ast->start_offset; i < ast->end_offset; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(ast->root, i);
        if (cypher_astnode_type(child) != type) continue;

        found = array_append(found, child);
    }

    return found;
}

const cypher_astnode_t* AST_GetBody(const cypher_parse_result_t *result) {
    const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
    assert(statement && cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

    return cypher_ast_statement_get_body(statement);
}

AST* AST_Build(cypher_parse_result_t *parse_result) {
    AST *ast = malloc(sizeof(AST));
    ast->root = AST_GetBody(parse_result);
    assert(ast->root);
    ast->record_length = 0;
    ast->entity_map = NULL;
    ast->defined_entities = NULL;
    ast->start_offset = 0;
    ast->end_offset = cypher_astnode_nchildren(ast->root);

    return ast;
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
long AST_ParseIntegerNode(const cypher_astnode_t *int_node) {
    assert(int_node);

    const char *value_str = cypher_ast_integer_get_valuestr(int_node);
    return strtol(value_str, NULL, 0);
}

/* TODO CYPHER_OP_IS_NULL and CYPHER_OP_IS_NOT_NULL are also unaries (though postfix).
 * Investigate this, also check CIPs because I think null handling may be different in
 * Cypher now than it is in the parser. */
/*
AST_Operator AST_ConvertUnaryOperatorNode(const cypher_astnode_t *expr) {
    // TODO All in AR_EXP_FromExpression right now
}
*/

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause) {
    assert(clause);

    bool aggregated = false;

    // Retrieve all user-specified functions in clause.
    TrieMap *referred_funcs = NewTrieMap();
    AST_ReferredFunctions(clause, referred_funcs);

    void *value;
    tm_len_t len;
    char *funcName;
    TrieMapIterator *it = TrieMap_Iterate(referred_funcs, "", 0);
    while(TrieMapIterator_Next(it, &funcName, &len, &value)) {
        if(Agg_FuncExists(funcName)) {
            aggregated = true;
            break;
        }
    }
    TrieMapIterator_Free(it);
    TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

    return aggregated;
}

void _AST_MapPath(AST *ast, const cypher_astnode_t *path, bool map_anonymous) {
    uint nelems = cypher_ast_pattern_path_nelements(path);
    for (uint i = 0; i < nelems; i ++) {
        const cypher_astnode_t *ast_alias = NULL;
        const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, i);
        if (i % 2) {
            ast_alias = cypher_ast_rel_pattern_get_identifier(entity);
        } else {
            ast_alias = cypher_ast_node_pattern_get_identifier(entity);
        }

        // If the entity is aliased: (a:person)
        // it should be mapped. We may have already constructed a mapping
        // on a previous encounter: MATCH (a)-[]->(a)
        if (ast_alias) {
            const char *alias = cypher_ast_identifier_get_name(ast_alias);
            AR_ExpNode *exp = AST_GetEntityFromAlias(ast, (char*)alias);
            // Alias was not previously encountered.
            if (exp == NULL) {
                exp = AR_EXP_NewVariableOperandNode(ast, entity, alias, NULL);
                exp->record_idx = AST_AddRecordEntry(ast);
                exp->operand.variadic.entity_alias_idx = exp->record_idx; // TODO bad logic
            }
            AST_MapEntity(ast, entity, exp);
            AST_MapAlias(ast, (char*)alias, exp);
            ast->defined_entities = array_append(ast->defined_entities, exp);
        } else if (map_anonymous) {
            uint id = AST_AddRecordEntry(ast);
            AR_ExpNode *exp = _AR_Exp_NewIdentifier(NULL, entity, id);
            ast->defined_entities = array_append(ast->defined_entities, exp);
            AST_MapEntity(ast, entity, exp);
        }
    }
}

void _AST_MapPattern(AST *ast, const cypher_astnode_t *pattern, bool map_anonymous) {
    uint npaths = cypher_ast_pattern_npaths(pattern);
    for (uint i = 0; i < npaths; i ++) {
        const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
        _AST_MapPath(ast, path, map_anonymous);
    }
}

void AST_BuildAliasMap(AST *ast) {
    if (ast->entity_map == NULL) ast->entity_map = NewTrieMap();
    if (ast->defined_entities == NULL) ast->defined_entities = array_new(AR_ExpNode*, 1);

    // Check every clause in the given range
    for (uint i = ast->start_offset; i < ast->end_offset; i ++) {
        const cypher_astnode_t *clause = cypher_astnode_get_child(ast->root, i);
        cypher_astnode_type_t type = cypher_astnode_type(clause);
        // MATCH, MERGE, and CREATE operations may define node and edge patterns
        // as well as aliases, all of which should be mapped
        if (type == CYPHER_AST_MATCH) {
            // TODO mapping all pattern entities for Record - should add specific required anonymous
            // entities later.
            _AST_MapPattern(ast, cypher_ast_match_get_pattern(clause), true);
        } else if (type == CYPHER_AST_MERGE) {
            _AST_MapPath(ast, cypher_ast_merge_get_pattern_path(clause), true);
        } else if (type == CYPHER_AST_CREATE) {
            _AST_MapPattern(ast, cypher_ast_create_get_pattern(clause), true);
            // UNWIND and WITH create aliases separately from node/edge patterns
        } else if (type == CYPHER_AST_UNWIND) {
            _mapUnwind(ast, clause);
        } else if (type == CYPHER_AST_WITH) {
            _mapWith(ast, clause);
        }
    }
}

AST* AST_GetFromTLS(void) {
    AST *ast = pthread_getspecific(_tlsASTKey);
    assert(ast);
    return ast;
}

