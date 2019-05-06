/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

static void _AST_GetIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if(!node) return;
    assert(identifiers);

    if(cypher_astnode_type(node) == CYPHER_AST_IDENTIFIER) {
        const char *identifier = cypher_ast_identifier_get_name(node);
        TrieMap_Add(identifiers, (char*)identifier, strlen(identifier), NULL, TrieMap_DONT_CARE_REPLACE);
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
        _AST_GetIdentifiers(child, identifiers);
    }
}

// UNWIND and WITH also form aliases, but don't need special handling for us yet.
static void _AST_GetReturnAliases(const cypher_astnode_t *node, TrieMap *aliases) {
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
        TrieMap_Add(aliases, (char*)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }
}

/* Compares a triemap of user-specified functions with the registered functions we provide. */
static AST_Validation _AST_ValidateReferredFunctions(TrieMap *referred_functions, char **reason, bool include_aggregates) {
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

static AST_Validation _Validate_MATCH_Clause(const AST *ast, char **reason) {
    // Check to see if all mentioned inlined, outlined functions exists.
    // Inlined functions appear within entity definition ({a:v})
    // Outlined functions appear within the WHERE clause.
    // libcypher_parser doesn't have a WHERE node, all of the filters
    // are specified within the MATCH node sub-tree.
    // Iterate over all top-level query children (clauses)
    uint match_count = AST_GetClauseCount(ast, CYPHER_AST_MATCH);
    if (match_count == 0) return AST_VALID;

    const cypher_astnode_t *match_clauses[match_count];
    AST_GetTopLevelClauses(ast, CYPHER_AST_MATCH, match_clauses);

    TrieMap *referred_funcs = NewTrieMap();
    TrieMap *identifiers = NewTrieMap();
    TrieMap *reused_entities = NewTrieMap();
    AST_Validation res;

    for (unsigned int i = 0; i < match_count; i ++) {
        const cypher_astnode_t *match_clause = match_clauses[i];
        // Collect function references
        AST_ReferredFunctions(match_clause, referred_funcs);

        // Validate relations contained in clause
        res = _Validate_MATCH_Clause_Relations(match_clause, identifiers, reason);
        if (res == AST_INVALID) goto cleanup;

        res = _Validate_MATCH_Clause_IndependentPaths(match_clause, reused_entities, reason);
        if (res == AST_INVALID) goto cleanup;
    }

    // Verify that referred functions exist.
    bool include_aggregates = false;
    res = _AST_ValidateReferredFunctions(referred_funcs, reason, include_aggregates);

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

static AST_Validation _Validate_CREATE_Clause(const AST *ast, char **reason) {
    const cypher_astnode_t *create_clause = AST_GetClause(ast, CYPHER_AST_CREATE);
    if (!create_clause) return AST_VALID;

    if (_Validate_CREATE_Clause_TypedRelations(create_clause) == AST_INVALID) {
        asprintf(reason, "Exactly one relationship type must be specified for CREATE");
        return AST_INVALID;
    }

    return AST_VALID;
}

static AST_Validation _Validate_DELETE_Clause(const AST *ast, char **reason) {
    const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
    if (!delete_clause) return AST_VALID;

    const cypher_astnode_t *match_clause = AST_GetClause(ast, CYPHER_AST_MATCH);
    if (!match_clause) return AST_INVALID;

    return AST_VALID;
}

static AST_Validation _Validate_RETURN_Clause(const AST *ast, char **reason) {
    const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
    if (!return_clause) return AST_VALID;

    // Retrieve all user-specified functions in RETURN clause.
    TrieMap *referred_funcs = NewTrieMap();
    AST_ReferredFunctions(return_clause, referred_funcs);

    // Verify that referred functions exist.
    bool include_aggregates = true;
    AST_Validation res = _AST_ValidateReferredFunctions(referred_funcs, reason, include_aggregates);
    TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

    return res;
}

static void _AST_GetDefinedIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if (!node) return;
    cypher_astnode_type_t type = cypher_astnode_type(node);
    if (type == CYPHER_AST_RETURN) {
        // Only collect aliases (which may be referenced in an ORDER BY)
        // from the RETURN clause, rather than all identifiers
        _AST_GetReturnAliases(node, identifiers);
    } else if (type == CYPHER_AST_MERGE || type == CYPHER_AST_UNWIND || type == CYPHER_AST_MATCH || type == CYPHER_AST_CREATE) {
        _AST_GetIdentifiers(node, identifiers);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(node);
        for(int c = 0; c < child_count; c ++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
            _AST_GetDefinedIdentifiers(child, identifiers);
        }
    }
}

static void _AST_GetReferredIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
    if (!node) return;
    cypher_astnode_type_t type = cypher_astnode_type(node);
    if (type == CYPHER_AST_SET || type == CYPHER_AST_RETURN || type == CYPHER_AST_DELETE || type == CYPHER_AST_UNWIND) {
        _AST_GetIdentifiers(node, identifiers);
    } else {
        unsigned int child_count = cypher_astnode_nchildren(node);
        for(int c = 0; c < child_count; c ++) {
            const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
            _AST_GetReferredIdentifiers(child, identifiers);
        }
    }
}

/* Check that all referred identifiers been defined. */
static AST_Validation _AST_Aliases_Defined(const AST *ast, char **undefined_alias) {
    AST_Validation res = AST_VALID;

    // Get defined identifiers.
    TrieMap *defined_aliases = NewTrieMap();
    _AST_GetDefinedIdentifiers(ast->root, defined_aliases);

    // Get referred identifiers.
    TrieMap *referred_identifiers = NewTrieMap();
    _AST_GetReferredIdentifiers(ast->root, referred_identifiers);

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

AST_Validation AST_Validate(const AST *ast, char **reason) {
    // Occurs on statements like CREATE INDEX
    if (cypher_astnode_type(ast->root) != CYPHER_AST_QUERY) return AST_VALID;

    if (_Validate_MATCH_Clause(ast, reason) == AST_INVALID) {
        return AST_INVALID;
    }

    if (_Validate_CREATE_Clause(ast, reason) == AST_INVALID) {
        return AST_INVALID;
    }

    if (_Validate_DELETE_Clause(ast, reason) == AST_INVALID) {
        return AST_INVALID;
    }

    if (_Validate_RETURN_Clause(ast, reason) == AST_INVALID) {
        return AST_INVALID;
    }

    if(_AST_Aliases_Defined(ast, reason) == AST_INVALID) {
        return AST_INVALID;
    }

    return AST_VALID;
}

//==============================================================================
//=== MATCH CLAUSE =============================================================
//==============================================================================
void AST_MatchClause_DefinedEntities(const AST *ast, TrieMap *definedEntities) {
    const cypher_astnode_t *match_clause = AST_GetClause(ast, CYPHER_AST_MATCH);

    _AST_GetIdentifiers(match_clause, definedEntities);
}

//==============================================================================
//=== WHERE CLAUSE =============================================================
//==============================================================================
void _AST_WhereClause_ReferredFunctions(const cypher_astnode_t *match_clause, TrieMap *referred_funcs) {
    if (!match_clause) return;
    AST_ReferredFunctions(match_clause, referred_funcs);
}

// void AST_WhereClause_ReferredFunctions(const cypher_parse_result_t *ast, TrieMap *referred_funcs) {
//     const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
//     const cypher_astnode_t *match = AST_GetClause(root, CYPHER_AST_MATCH);
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
