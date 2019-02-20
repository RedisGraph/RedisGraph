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

static void _NEWAST_GetIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if(!node) return;
    assert(identifiers);

    if(cypher_astnode_type(node) == CYPHER_AST_IDENTIFIER) {
        const char *identifier = cypher_ast_identifier_get_name(node);
        TrieMap_Add(identifiers, (char*)identifier, strlen(identifier), NULL, TrieMap_NOP_REPLACE);
    }

    unsigned int child_count = cypher_astnode_nchildren(node);

    /* In case current node is of type projection
     * inspect first child only,
     * @10  20..26  > > > projection           expression=@11, alias=@14
     * @11  20..26  > > > > apply              @12(@13)
     * @12  20..23  > > > > > function name    `max`
     * @13  24..25  > > > > > identifier       `z`
     * @14  20..26  > > > > identifier         `max(z)` */
    if(cypher_astnode_type(node) == CYPHER_AST_PROJECTION) child_count = 1;

    for(int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        _NEWAST_GetIdentifiers(child, identifiers);
    }
}

// UNWIND and WITH also form aliases, but don't need special handling for us yet.
static void _NEWAST_GetReturnAliases(const cypher_astnode_t *node, TrieMap *aliases) {
    if (!node) return;
    if(cypher_astnode_type(node) != CYPHER_AST_RETURN) return;
    assert(aliases);

    int num_return_projections = cypher_ast_return_nprojections(node);
    if (num_return_projections == 0) return;

    for (unsigned int i = 0; i < num_return_projections; i ++) {
        const cypher_astnode_t *child = cypher_ast_return_get_projection(node, i);
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(child);
        if (alias_node == NULL) continue;
        const char *alias = cypher_ast_identifier_get_name(alias_node);
        TrieMap_Add(aliases, (char*)alias, strlen(alias), NULL, TrieMap_NOP_REPLACE);
    }
}

/* Compares a triemap of user-specified functions with the registered functions we provide. */
static AST_Validation _NEWAST_ValidateReferredFunctions(TrieMap *referred_functions, char **reason, bool include_aggregates) {
    int res = AST_VALID;
    void *value;
    tm_len_t len;
    char *funcName;
    TrieMapIterator *it = TrieMap_Iterate(referred_functions, "", 0);
    *reason = NULL;

    // TODO: return RAX.
    while(TrieMapIterator_Next(it, &funcName, &len, &value)) {
        funcName[len] = 0;
        // No functions have a name longer than 32 characters
        if(len >= 32) {
            res = AST_INVALID;
            break;
        }

        if(AR_FuncExists(funcName)) continue;

        if(Agg_FuncExists(funcName)) {
            if(include_aggregates) {
                continue;
            } else {
                // Provide a unique error for using aggregate functions from inappropriate contexts
                asprintf(reason, "Invalid use of aggregating function '%s'", funcName);
                res = AST_INVALID;
                break;
            }
        }

        // If we reach this point, the function was not found
        res = AST_INVALID;
        break;
    }

    // If the function was not found, provide a reason if one is not set
    if (res == AST_INVALID && *reason == NULL) {
        asprintf(reason, "Unknown function '%s'", funcName);
    }

    TrieMapIterator_Free(it);
    return res;
}

// Note each function call within given expression
// Example: given the expression: "abs(max(min(a), abs(k)))"
// referred_funcs will include: "abs", "max" and "min".
static void _consume_function_call_expression(const cypher_astnode_t *expression, TrieMap *referred_funcs) {
    // Value is an apply operator
    const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(expression);
    const char *func_name = cypher_ast_function_name_get_value(func);
    TrieMap_Add(referred_funcs, (char*)func_name, strlen(func_name), NULL, TrieMap_NOP_REPLACE);

    unsigned int narguments = cypher_ast_apply_operator_narguments(expression);
    for(int i = 0; i < narguments; i++) {
        const cypher_astnode_t *child_exp = cypher_ast_apply_operator_get_argument(expression, i);
        cypher_astnode_type_t child_exp_type = cypher_astnode_type(child_exp);
        if(child_exp_type != CYPHER_AST_APPLY_OPERATOR) continue;
        _consume_function_call_expression(child_exp, referred_funcs);
    }
}

static AST_Validation _MATCH_Clause_Validate_Range(const cypher_astnode_t *node, char **reason) {
    // Defaults
    int start = 1;
    int end = INT_MAX-2;

    const cypher_astnode_t *range_start = cypher_ast_range_get_start(node);
    if(range_start) {
        const char *start_str = cypher_ast_integer_get_valuestr(range_start);
        start = atoi(start_str);
    }

    const cypher_astnode_t *range_end = cypher_ast_range_get_end(node);
    if(range_end) {
        const char *end_str = cypher_ast_integer_get_valuestr(range_end);
        end = atoi(end_str);
    }

    if(start > end) {
        asprintf(reason, "Variable length path, maximum number of hops must be greater or equal to minimum number of hops.");
        return AST_INVALID;
    }

    return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_ReusedEdges(const cypher_astnode_t *node, TrieMap *identifiers, char **reason) {
    unsigned int child_count = cypher_astnode_nchildren(node);
    for(int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        cypher_astnode_type_t type = cypher_astnode_type(child);

        if(type == CYPHER_AST_IDENTIFIER) {
            const char *identifier = cypher_ast_identifier_get_name(child);
            int new = TrieMap_Add(identifiers, (char*)identifier, strlen(identifier), NULL, TrieMap_DONT_CARE_REPLACE);
            if(!new) {
                asprintf(reason, "Cannot use the same relationship variable '%s' for multiple patterns.", identifier);
                return AST_INVALID;
            }
        }
    }

    return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_Relations(const cypher_astnode_t *node, TrieMap *identifiers, char **reason) {
    cypher_astnode_type_t node_type = cypher_astnode_type(node);

    // Make sure relation identifier is unique.
    if(node_type == CYPHER_AST_REL_PATTERN) {
        if(_Validate_MATCH_Clause_ReusedEdges(node, identifiers, reason) == AST_INVALID) return AST_INVALID;
    }
    if(node_type == CYPHER_AST_RANGE) {
        return _MATCH_Clause_Validate_Range(node, reason);
    }

    // Keep scanning for relation pattern nodes.
    unsigned int child_count = cypher_astnode_nchildren(node);
    for(int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        if(_Validate_MATCH_Clause_Relations(child, identifiers, reason) == AST_INVALID) return AST_INVALID;
    }

    return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_ReusedIdentifiers(const cypher_astnode_t *node,
                                                const cypher_astnode_t *current_path,
                                                TrieMap *identifiers, char **reason) {
    if (!node) return AST_VALID;

    if (cypher_astnode_type(node) == CYPHER_AST_IDENTIFIER) {
        const char *alias = cypher_ast_identifier_get_name(node);
        void *val = TrieMap_Find(identifiers, (char*)alias, strlen(alias));
        // If this alias has been used in a previous path, emit an error.
        if (val != TRIEMAP_NOTFOUND && val != current_path) {
            asprintf(reason, "Alias '%s' reused. Entities with the same alias may not be referenced in multiple patterns.", alias);
            return AST_INVALID;
        }
        // Use the path pointer to differentiate aliases reused within a path from aliases used in separate paths.
        TrieMap_Add(identifiers, (char*)alias, strlen(alias), (void*)current_path, TrieMap_DONT_CARE_REPLACE);
    }

    unsigned int child_count = cypher_astnode_nchildren(node);
    for (unsigned int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        if (_Validate_MATCH_Clause_ReusedIdentifiers(child, current_path, identifiers, reason) != AST_VALID) {
            return AST_INVALID;
        }
    }
    return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_IndependentPaths(const cypher_astnode_t *match, TrieMap *reused_entities, char **reason) {
  /* Verify that no alias appears in multiple independent patterns.
   * TODO We should introduce support for this when possible. */
    AST_Validation res = AST_VALID;

    const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match);
    // A pattern is composed of 1 or more paths that currently must be independent for us.
    unsigned int child_count = cypher_astnode_nchildren(pattern);
    for (unsigned int i = 0; i < child_count; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(pattern, i);
        if (cypher_astnode_type(child) == CYPHER_AST_PATTERN_PATH) {
            res = _Validate_MATCH_Clause_ReusedIdentifiers(child, child, reused_entities, reason);
            if (res != AST_VALID) break;
        }
    }

    return res;
}

static AST_Validation _Validate_MATCH_Clause(const cypher_astnode_t *query, char **reason) {
    // Check to see if all mentioned inlined, outlined functions exists.
    // Inlined functions appear within entity definition ({a:v})
    // Outlined functions appear within the WHERE clause.
    // libcypher_parser doesn't have a WHERE node, all of the filters
    // are specified within the MATCH node sub-tree.
    // Iterate over all top-level query children (clauses)
    unsigned int clause_count = cypher_astnode_nchildren(query);
    const cypher_astnode_t *match_clauses[clause_count];
    unsigned int match_count = NewAST_GetTopLevelClauses(query, CYPHER_AST_MATCH, match_clauses);
    if (match_count == 0) return AST_VALID;

    TrieMap *referred_funcs = NewTrieMap();
    TrieMap *identifiers = NewTrieMap();
    TrieMap *reused_entities = NewTrieMap();
    AST_Validation res;

    for (unsigned int i = 0; i < match_count; i ++) {
        const cypher_astnode_t *match_clause = match_clauses[i];
        // Collect function references
        NEWAST_ReferredFunctions(match_clause, referred_funcs);

        // Validate relations contained in clause
        res = _Validate_MATCH_Clause_Relations(match_clause, identifiers, reason);
        if (res == AST_INVALID) goto cleanup;

        res = _Validate_MATCH_Clause_IndependentPaths(match_clause, reused_entities, reason);
        if (res == AST_INVALID) goto cleanup;
    }

    // Verify that referred functions exist.
    bool include_aggregates = false;
    res = _NEWAST_ValidateReferredFunctions(referred_funcs, reason, include_aggregates);

    if(res == AST_INVALID) goto cleanup;

cleanup:
    TrieMap_Free(referred_funcs, TrieMap_NOP_CB);
    TrieMap_Free(identifiers, TrieMap_NOP_CB);
    TrieMap_Free(reused_entities, TrieMap_NOP_CB);
    return res;
}

// Validate that each relation pattern is typed.
static AST_Validation _Validate_CREATE_Clause_TypedRelations(const cypher_astnode_t *node) {
    unsigned int child_count = cypher_astnode_nchildren(node);

    // Make sure relation pattern has a single relation type.
    if(cypher_astnode_type(node) == CYPHER_AST_REL_PATTERN) {
        unsigned int relation_type_count = 0;
        for(int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
            if(cypher_astnode_type(child) == CYPHER_AST_RELTYPE) relation_type_count++;
        }
        if(relation_type_count != 1) return AST_INVALID;
    } else {
        // Keep scanning for relation pattern nodes.
        for(int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
            if(_Validate_CREATE_Clause_TypedRelations(child) == AST_INVALID) return AST_INVALID;
        }
    }

    return AST_VALID;
}

static AST_Validation _Validate_CREATE_Clause(const cypher_astnode_t *ast, char **reason) {
    const cypher_astnode_t *create_clause = NEWAST_GetClause(ast, CYPHER_AST_CREATE);
    if (!create_clause) return AST_VALID;

    if (_Validate_CREATE_Clause_TypedRelations(create_clause) == AST_INVALID) {
        asprintf(reason, "Exactly one relationship type must be specified for CREATE");
        return AST_INVALID;
    }

    return AST_VALID;
}

static AST_Validation _Validate_DELETE_Clause(const cypher_astnode_t *ast, char **reason) {
    const cypher_astnode_t *delete_clause = NEWAST_GetClause(ast, CYPHER_AST_DELETE);
    if (!delete_clause) return AST_VALID;

    const cypher_astnode_t *match_clause = NEWAST_GetClause(ast, CYPHER_AST_MATCH);
    if (!match_clause) return AST_INVALID;

    return AST_VALID;
}

static AST_Validation _Validate_RETURN_Clause(const cypher_astnode_t *ast, char **reason) {
    const cypher_astnode_t *return_clause = NEWAST_GetClause(ast, CYPHER_AST_RETURN);
    if (!return_clause) return AST_VALID;

    // Retrieve all user-specified functions in RETURN clause.
    TrieMap *referred_funcs = NewTrieMap();
    NEWAST_ReferredFunctions(return_clause, referred_funcs);

    // Verify that referred functions exist.
    bool include_aggregates = true;
    AST_Validation res = _NEWAST_ValidateReferredFunctions(referred_funcs, reason, include_aggregates);
    TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

    return res;
}

static void _NEWAST_GetDefinedIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if (!node) return;
    cypher_astnode_type_t type = cypher_astnode_type(node);
    if (type == CYPHER_AST_RETURN) {
        // Only collect aliases (which may be referenced in an ORDER BY)
        // from the RETURN clause, rather than all identifiers
        _NEWAST_GetReturnAliases(node, identifiers);
    } else if (type == CYPHER_AST_MERGE || type == CYPHER_AST_UNWIND || type == CYPHER_AST_MATCH) {
        _NEWAST_GetIdentifiers(node, identifiers);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(node);
        for(int c = 0; c < child_count; c ++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
            _NEWAST_GetDefinedIdentifiers(child, identifiers);
        }
    }
}

static void _NEWAST_GetReferredIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if (!node) return;
    cypher_astnode_type_t type = cypher_astnode_type(node);
    if (type == CYPHER_AST_SET || type == CYPHER_AST_RETURN || type == CYPHER_AST_DELETE || type == CYPHER_AST_UNWIND) {
        _NEWAST_GetIdentifiers(node, identifiers);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(node);
        for(int c = 0; c < child_count; c ++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
            _NEWAST_GetReferredIdentifiers(child, identifiers);
        }
    }
}

// TODO No need to do this separately from ID mapping
/* Check that all referred identifiers been defined. */
static AST_Validation _NEWAST_Aliases_Defined(const cypher_astnode_t *ast, char **undefined_alias) {
    AST_Validation res = AST_VALID;

    // Get defined identifiers.
    TrieMap *defined_aliases = NewTrieMap();
    _NEWAST_GetDefinedIdentifiers(ast, defined_aliases);

    // Get referred identifiers.
    TrieMap *referred_identifiers = NewTrieMap();
    _NEWAST_GetReferredIdentifiers(ast, referred_identifiers);

    char *alias;
    tm_len_t len;
    void *value;
    TrieMapIterator *it = TrieMap_Iterate(referred_identifiers, "", 0);

    // See that each referred identifier is defined.
    while(TrieMapIterator_Next(it, &alias, &len, &value)) {
        if(TrieMap_Find(defined_aliases, alias, len) == TRIEMAP_NOTFOUND) {
            asprintf(undefined_alias, "%s not defined", alias);
            res = AST_INVALID;
            break;
        }
    }

    // Clean up:
    TrieMapIterator_Free(it);
    TrieMap_Free(defined_aliases, TrieMap_NOP_CB);
    TrieMap_Free(referred_identifiers, TrieMap_NOP_CB);
    return res;
}

AST_Validation NEWAST_Validate(const cypher_astnode_t *query, char **reason) {
    // Occurs on statements like CREATE INDEX
    if (cypher_astnode_type(query) != CYPHER_AST_QUERY) return AST_VALID;

    if (_Validate_MATCH_Clause(query, reason) == AST_INVALID) {
        printf("_Validate_MATCH_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_CREATE_Clause(query, reason) == AST_INVALID) {
        printf("_Validate_CREATE_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_DELETE_Clause(query, reason) == AST_INVALID) {
        printf("_Validate_DELETE_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_RETURN_Clause(query, reason) == AST_INVALID) {
        printf("_Validate_RETURN_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if(_NEWAST_Aliases_Defined(query, reason) == AST_INVALID) {
        printf("_NEWAST_Aliases_Defined, AST_INVALID\n");
        return AST_INVALID;
    }

    return AST_VALID;
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

// Scan through the entire AST collect each function call
// we encounter.
void NEWAST_ReferredFunctions(const cypher_astnode_t *root, TrieMap *referred_funcs) {
    cypher_astnode_type_t root_type = cypher_astnode_type(root);
    if(root_type == CYPHER_AST_APPLY_OPERATOR) {
        _consume_function_call_expression(root, referred_funcs);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(root);
        for(int i = 0; i < child_count; i++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
            NEWAST_ReferredFunctions(child, referred_funcs);
        }
    }
}

//==============================================================================
//=== MATCH CLAUSE =============================================================
//==============================================================================
void NEWAST_MatchClause_DefinedEntities(const cypher_astnode_t *ast, TrieMap *definedEntities) {
    const cypher_astnode_t *match_clause = NEWAST_GetClause(ast, CYPHER_AST_MATCH);

    _NEWAST_GetIdentifiers(match_clause, definedEntities);
}

//==============================================================================
//=== RETURN CLAUSE ============================================================
//==============================================================================
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

//==============================================================================
//=== WHERE CLAUSE =============================================================
//==============================================================================
void NEWAST_WhereClause_ReferredFunctions(const cypher_astnode_t *match_clause, TrieMap *referred_funcs) {
    if (!match_clause) return;
    NEWAST_ReferredFunctions(match_clause, referred_funcs);
}

// void NEWAST_WhereClause_ReferredFunctions(const cypher_parse_result_t *ast, TrieMap *referred_funcs) {
//     const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
//     const cypher_astnode_t *match = NEWAST_GetClause(root, CYPHER_AST_MATCH);
//     if (!match) return;

//     // Inspect inlined filters ({a:v}).
//     const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match);
//     unsigned int npaths = cypher_ast_pattern_npaths(pattern);

//     // For each path in pattern
//     for(unsigned int i = 0; i < npaths; i++) {
//         const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
//         unsigned int nelements = cypher_ast_pattern_path_nelements(path);

//         // For each element in current path.
//         for(unsigned int j = 0; j < nelements; j++) {
//             const cypher_astnode_t *element = cypher_ast_pattern_path_get_element(path, j);
//             const cypher_astnode_t *properties;

//             // Element is a node if j is odd, otherwise element is a relation.
//             if((j % 2) == 0) properties = cypher_ast_node_pattern_get_properties(element);
//             else properties = cypher_ast_rel_pattern_get_properties(element);
//             if(!properties) continue;

//             unsigned int nentries = cypher_ast_map_nentries(properties);
//             // For each key/value pair in current element.
//             for(unsigned int k = 0; k < nentries; k++) {
//                 const cypher_astnode_t *value = cypher_ast_map_get_value(properties, k);
//                 cypher_astnode_type_t value_type = cypher_astnode_type(value);
//                 if(value_type != CYPHER_AST_APPLY_OPERATOR) continue;
//                 _consume_function_call_expression(value, referred_funcs);
//             }
//         }
//     }
// }

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

// Retrieve the first instance of the specified clause, if any.
const cypher_astnode_t* NEWAST_GetClause(const cypher_astnode_t *query, cypher_astnode_type_t clause_type) {
    unsigned int num_clauses = cypher_astnode_nchildren(query);
    for (unsigned int i = 0; i < num_clauses; i ++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(query, i);
        if (cypher_astnode_type(child) == clause_type) return child;
    }

    return NULL;
}

const cypher_astnode_t* NEWAST_GetBody(const cypher_parse_result_t *result) {
    const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
    assert(statement && cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

    return cypher_ast_statement_get_body(statement);
}

NEWAST_GraphEntity* New_GraphEntity(const cypher_astnode_t *entity, const cypher_astnode_type_t type) {
    NEWAST_GraphEntity *ge = calloc(1, sizeof(NEWAST_GraphEntity));
    ge->ast_ref = entity;
    if (type == CYPHER_AST_NODE_PATTERN) {
        ge->t = N_ENTITY;
        const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(entity);
        if (alias_node) ge->alias = strdup(cypher_ast_identifier_get_name(alias_node));
        if (cypher_ast_node_pattern_nlabels(entity) > 0) {
            const cypher_astnode_t *label_node = cypher_ast_node_pattern_get_label(entity, 0);
            ge->label = strdup(cypher_ast_label_get_name(label_node));
        }
    } else if (type == CYPHER_AST_REL_PATTERN) {
        ge->t = N_LINK;
        const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(entity);
        if (alias_node) ge->alias = strdup(cypher_ast_identifier_get_name(alias_node));
        if (cypher_ast_rel_pattern_nreltypes(entity) > 0) {
            // TODO collect all reltypes or update logic elesewhere
            const cypher_astnode_t *reltype_node = cypher_ast_rel_pattern_get_reltype(entity, 0);
            ge->label = strdup(cypher_ast_reltype_get_name(reltype_node));
        }
    // } else { // type == CYPHER_AST_PROJECTION
        // ge->t = N_PROJECTION;
        // const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(entity);
        // assert(alias_node);
        // ge->alias = strdup(cypher_ast_identifier_get_name(alias_node));
    // }

    // // TODO tmp
    // AST_Entity *ret = New_AST_Entity(ge->alias, A_ENTITY, ge);
    // return ret;
    }
    return ge;
}

// AST_Entity* New_AST_Entity(const char *alias, AST_EntityType t, void *ptr) {
    // AST_Entity *ret = malloc(sizeof(AST_Entity));
    // ret->alias = alias;
    // ret->t = t;
    // if (t == A_ENTITY) {
        // ret->ge = ptr;
    // } else {
        // ret->exp = ptr;
    // }
    // return ret;
// }

void _mapReturnAliases(NEWAST *ast, unsigned int *id) {
    const cypher_astnode_t *return_clause = NEWAST_GetClause(ast->root, CYPHER_AST_RETURN);
    if (!return_clause) return;

    int num_return_projections = cypher_ast_return_nprojections(return_clause);
    if (num_return_projections == 0) return;

    for (unsigned int i = 0; i < num_return_projections; i ++) {
        const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
        if (alias_node == NULL) continue;
        const char *alias = cypher_ast_identifier_get_name(alias_node);
        unsigned int *entityID = NULL;
        const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);
        AR_ExpNode *ar_exp = AR_EXP_FromExpression(ast, expr);
        if (ar_exp->type == AR_EXP_OPERAND && ar_exp->operand.type == AR_EXP_VARIADIC) {
            // TODO lookup and point to same entity if it exists?
        } else {
            unsigned int *entityID = malloc(sizeof(unsigned int));
            *entityID = (*id)++;
            // ast->defined_entities = array_append(ast->defined_entities, alias);
            TrieMap_Add(ast->identifier_map, (char*)alias, strlen(alias), entityID, TrieMap_NOP_REPLACE);
        }
        // if(cypher_astnode_type(expr) != CYPHER_AST_PROPERTY_OPERATOR) {
            // const char *identifier = cypher_ast_identifier_get_name(expr);
            // void *v = TrieMap_Find(ast->identifier_map, (char*)identifier, strlen(identifier));
            // assert(v != TRIEMAP_NOTFOUND);
            // entityID = v;
            // TrieMap_Add(ast->identifier_map, (char*)alias, strlen(alias), entityID, TrieMap_NOP_REPLACE);
        // }
    }
}

// Capture node and relation identifiers from MATCH, MERGE, and CREATE clauses
void _mapPatternIdentifiers(NEWAST *ast, const cypher_astnode_t *node, unsigned int *id) {
    if (!node) return;

    cypher_astnode_type_t type = cypher_astnode_type(node);

    // TODO improve logic
    NEWAST_GraphEntity *ge = NULL;
    if (type == CYPHER_AST_NODE_PATTERN ||
        type == CYPHER_AST_REL_PATTERN) { // ||
        // type == CYPHER_AST_PROJECTION) {
        ge = New_GraphEntity(node, type);
    /*
    } else if (type == CYPHER_AST_PROJECTION) {
        // UNWIND, RETURN AS
        // ge = New_GraphEntity(node, type);
        const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(node);
        if (alias_node) {
            // If this projection constructs an alias, add it to the triemap with the
            // ID of the referenced entity
            const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(node);
            const cypher_astnode_t *expr = cypher_ast_projection_get_expression(node);

            // TODO still untrue for things like RETURN 5 AS e
            const char *alias = cypher_ast_identifier_get_name(alias_node);
            unsigned int *entityID = NULL;
            if(cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
                const char *identifier = cypher_ast_identifier_get_name(expr);
                void *v = TrieMap_Find(ast->identifier_map, (char*)identifier, strlen(identifier));
                assert(v != TRIEMAP_NOTFOUND);
                entityID = v;
            } else {
                entityID = malloc(sizeof(unsigned int));
                *entityID = (*id)++;
            }
            TrieMap_Add(ast->identifier_map, (char*)alias, strlen(alias), entityID, TrieMap_NOP_REPLACE);
        }
        */

    }
    // Add identifier if we have found one.
    if (ge && ge->alias) {
        void *val = TrieMap_Find(ast->identifier_map, ge->alias, strlen(ge->alias));
        // Do nothing if entry already exists
        if (val != TRIEMAP_NOTFOUND) return;
        unsigned int *entityID = malloc(sizeof(unsigned int));
        *entityID = (*id)++;
        ast->defined_entities = array_append(ast->defined_entities, ge);
        TrieMap_Add(ast->identifier_map, ge->alias, strlen(ge->alias), entityID, TrieMap_NOP_REPLACE);
        return;
    }

    unsigned int child_count = cypher_astnode_nchildren(node);
    for(unsigned int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        _mapPatternIdentifiers(ast, child, id);
    }
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

void NEWAST_BuildAliasMap(NEWAST *ast) {
  unsigned int id = 0;
  ast->identifier_map = NewTrieMap(); // Holds mapping between referred entities and IDs.
  ast->defined_entities = array_new(cypher_astnode_t*, 1);
  // Get graph entity identifiers from MATCH, MERGE, and CREATE clauses.
  _mapPatternIdentifiers(ast, ast->root, &id);
  // Get aliases defined by UNWIND and RETURN...AS clauses

  _mapReturnAliases(ast, &id);
  // _walkTriemap(ast->identifier_map);
  /*
    unsigned int clause_count = cypher_astnode_nchildren(ast->root);
    const cypher_astnode_t *match_clauses[clause_count];
    unsigned int match_count = NewAST_GetTopLevelClauses(ast->root, CYPHER_AST_MATCH, match_clauses);
    for (unsigned int i = 0; i < match_count; i ++) {
        const cypher_astnode_t *match = match_clauses[i];
    }
    */
}

unsigned int NEWAST_GetAliasID(const NEWAST *ast, char *alias) {
  assert(ast->identifier_map);
  void *v = TrieMap_Find(ast->identifier_map, alias, strlen(alias));
  assert(v != TRIEMAP_NOTFOUND);
  unsigned int *id = v;
  return *id;
}

NEWAST_GraphEntity* NEWAST_GetEntity(const NEWAST *ast, unsigned int id) {
    return ast->defined_entities[id];
}

NEWAST* NEWAST_GetFromLTS(void) {
  NEWAST *ast = pthread_getspecific(_tlsNEWASTKey);
  assert(ast);
  return ast;
}

