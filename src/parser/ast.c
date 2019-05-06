/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <assert.h>

#include "../../deps/xxhash/xxhash.h"
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
    node->record_idx = id;
    node->operand.type = AR_EXP_VARIADIC;
    node->operand.variadic.entity_alias = entity_alias ? strdup(entity_alias) : NULL;
    node->operand.variadic.entity_alias_idx = id;
    node->operand.variadic.entity_prop = NULL;
    node->operand.variadic.ast_ref = entity;

    return node;
}

/*
 * If we have an alias that is associated with a previously-constructed expression:
 * Map the current entity to the existing expression.
 *
 * If we have an alias, but no previous expression:
 * Make and map a new entity, and give it space in the Record.
 *
 * If we have no alias:
 * Make and map a new entity.
 */
void _AddOrConnectEntity(AST *ast, const cypher_astnode_t *entity, char *alias, bool update_record) {

    // Hash the AST node into a unique identifier
    AST_IDENTIFIER identifier = AST_EntityHash(entity);

    // Do nothing if entity is already mapped
    if (AST_GetEntityFromHash(ast, identifier) != NULL) return;

    AR_ExpNode *exp = NULL;

    // TODO slightly overkill, as aliased but non-referenced entities don't need to be in record
    // If this entity is aliased, we may have encountered it from a different AST node previously.
    if (alias) {
        exp = AST_GetEntityFromAlias(ast, alias);
        if (exp) {
            // Alias was previously encountered
            // Associate the current hash and alias with the previously-built entity
            AST_MapEntityHash(ast, identifier, exp);
            AST_MapAlias(ast, alias, exp);
            return;
        }
    }

    // TODO also must add unaliased entities that have inlined filters - (:label {prop: 5})
    unsigned int id = NOT_IN_RECORD;
    if (alias || update_record) {
        // The entity must be created and added to the record, give it an index
        id = AST_AddRecordEntry(ast);
    }
    // Build an arithmetic expression node representing this identifier
    exp = _AR_Exp_NewIdentifier(alias, entity, id);
    // AR_EXP_AssignRecordIndex(exp, id);

    // Store the expression node in an ID-indexed array if necessary
    if (id != NOT_IN_RECORD) {
        ast->defined_entities = array_append(ast->defined_entities, exp);
        if (alias) AST_MapAlias(ast, alias, exp);
    }

    // TODO Adds some entities we don't need (anonymous unfiltered nodes, etc)
    AST_MapEntityHash(ast, identifier, exp);

}

bool _shouldRecordChildren(const cypher_astnode_type_t type) {
    if (type == CYPHER_AST_CREATE) {
        // All CREATE entities must be represented in the Record
        return true;
    } else if (type == CYPHER_AST_APPLY_OPERATOR) {
        // Function calls
        return true;
    } else {
        // TODO ?
    }

    return false;
}

/* Populate a triemap with node and relation identifiers from MATCH, MERGE, and CREATE clauses. */
void _mapPatternIdentifiers(AST *ast, const cypher_astnode_t *entity, bool record_children) {
    if (!entity) return;

    cypher_astnode_type_t type = cypher_astnode_type(entity);

    const char *alias = NULL;
    // If the current entity is a node or edge pattern, capture its alias
    if (type == CYPHER_AST_NODE_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(entity);
        if (alias_node) alias = cypher_ast_identifier_get_name(alias_node);
    } else if (type == CYPHER_AST_REL_PATTERN) {
        const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(entity);
        if (alias_node) alias = cypher_ast_identifier_get_name(alias_node);
    } else if (type == CYPHER_AST_UNWIND) {
        // The UNWIND clause aliases an expression
        const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(entity);
        assert(alias_node);
        alias = cypher_ast_identifier_get_name(alias_node);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(entity);
        for(unsigned int i = 0; i < child_count; i++) {
            if (record_children == false) record_children = _shouldRecordChildren(type);
            const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
            // Recursively continue searching
            _mapPatternIdentifiers(ast, child, record_children);
        }
        return;
    }

    _AddOrConnectEntity(ast, entity, (char*)alias, record_children);
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


int AST_ReturnClause_ContainsCollapsedNodes(const AST *ast) {
    const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);

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
const cypher_astnode_t* AST_GetClause(const AST *ast, cypher_astnode_type_t clause_type) {
    unsigned int num_clauses = cypher_astnode_nchildren(ast->root);
    for (unsigned int i = 0; i < num_clauses; i ++) {
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
        if (cypher_astnode_type(cypher_astnode_get_child(ast->root, i)) != clause_type) {
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

const cypher_astnode_t* AST_GetBody(const cypher_parse_result_t *result) {
    const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
    assert(statement && cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

    return cypher_ast_statement_get_body(statement);
}

AST* AST_Build(cypher_parse_result_t *parse_result) {
    AST *new_ast = malloc(sizeof(AST));
    new_ast->root = AST_GetBody(parse_result);
    assert(new_ast->root);
    new_ast->order_expression_count = 0;
    new_ast->record_length = 0;
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

void AST_BuildAliasMap(AST *ast) {
    ast->entity_map = NewTrieMap();
    ast->defined_entities = array_new(cypher_astnode_t*, 1);

    // Get graph entity identifiers from MATCH, MERGE, and CREATE clauses.
    _mapPatternIdentifiers(ast, ast->root, false);

    // Get aliases defined by UNWIND and RETURN...AS clauses
    // _mapReturnAliases(ast);
}

AST* AST_GetFromTLS(void) {
    AST *ast = pthread_getspecific(_tlsASTKey);
    assert(ast);
    return ast;
}

