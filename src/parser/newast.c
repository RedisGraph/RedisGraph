/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "newast.h"
#include <assert.h>

#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

static bool _NEWAST_ReadOnly(const cypher_astnode_t *root) {
    cypher_astnode_type_t root_type = cypher_astnode_type(root);
    
    if(root_type == CYPHER_AST_CREATE || 
        root_type == CYPHER_AST_MERGE ||
        root_type == CYPHER_AST_DELETE ||
        root_type == CYPHER_AST_SET ||
        root_type == CYPHER_AST_CREATE_NODE_PROP_INDEX ||
        root_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
        return false;
    }

    unsigned int child_count = cypher_astnode_nchildren(root);
    for(int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
        bool readonly = _NEWAST_ReadOnly(child);
        if(!readonly) return false;
    }

    return true;
}

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

static const cypher_astnode_t *_NEWAST_GetClause(const cypher_astnode_t *node, cypher_astnode_type_t clause) {
    if(cypher_astnode_type(node) == clause) return node;

    unsigned int child_count = cypher_astnode_nchildren(node);
    for(int i = 0; i < child_count; i++) {
        const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
        const cypher_astnode_t *required_clause = _NEWAST_GetClause(child, clause);
        if(required_clause) return required_clause;
    }

    return NULL;
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

static AST_Validation _Validate_MATCH_Clause(const cypher_astnode_t *ast, char **reason) {
    // Check to see if all mentioned inlined, outlined functions exists.
    // Inlined functions appear within entity definition ({a:v})
    // Outlined functions appear within the WHERE clause.
    // libcypher_parser doesn't have a WHERE node, all of the filters
    // are specified within the MATCH node sub-tree.
    const cypher_astnode_t *match_clause = _NEWAST_GetClause(ast, CYPHER_AST_MATCH);
    if (!match_clause) return AST_VALID;

    TrieMap *referred_funcs = NewTrieMap();
    NEWAST_ReferredFunctions(match_clause, referred_funcs);  

    // Verify that referred functions exist.
    bool include_aggregates = false;
    AST_Validation res = _NEWAST_ValidateReferredFunctions(referred_funcs, reason, include_aggregates);
    TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

    if(res == AST_INVALID) return res;

    TrieMap *identifiers = NewTrieMap();
    res = _Validate_MATCH_Clause_Relations(match_clause, identifiers, reason);
    TrieMap_Free(identifiers, TrieMap_NOP_CB);
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
    const cypher_astnode_t *create_clause = _NEWAST_GetClause(ast, CYPHER_AST_CREATE);
    if (!create_clause) return AST_VALID;

    if (_Validate_CREATE_Clause_TypedRelations(create_clause) == AST_INVALID) {
        asprintf(reason, "Exactly one relationship type must be specified for CREATE");
        return AST_INVALID;
    }
    
    return AST_VALID;
}

static AST_Validation _Validate_DELETE_Clause(const cypher_astnode_t *ast, char **reason) {
    const cypher_astnode_t *delete_clause = _NEWAST_GetClause(ast, CYPHER_AST_DELETE);
    if (!delete_clause) return AST_VALID;
    
    const cypher_astnode_t *match_clause = _NEWAST_GetClause(ast, CYPHER_AST_MATCH);
    if (!match_clause) return AST_INVALID;

    return AST_VALID;
}

static AST_Validation _Validate_RETURN_Clause(const cypher_astnode_t *ast, char **reason) {
    const cypher_astnode_t *return_clause = _NEWAST_GetClause(ast, CYPHER_AST_RETURN);
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

/* Check that all refereed identifiers been defined. */
static AST_Validation _NEWAST_Aliases_Defined(const cypher_astnode_t *ast, char **undefined_alias) {
    AST_Validation res = AST_VALID;
    const cypher_astnode_t *node;
    TrieMap *defined_aliases = NewTrieMap();
    TrieMap *refereed_identifiers = NewTrieMap();

    const cypher_astnode_t *set_clause = _NEWAST_GetClause(ast, CYPHER_AST_SET);
    const cypher_astnode_t *match_clause = _NEWAST_GetClause(ast, CYPHER_AST_MATCH);
    const cypher_astnode_t *merge_clause = _NEWAST_GetClause(ast, CYPHER_AST_MERGE);
    const cypher_astnode_t *unwind_clause = _NEWAST_GetClause(ast, CYPHER_AST_UNWIND);
    const cypher_astnode_t *return_clause = _NEWAST_GetClause(ast, CYPHER_AST_RETURN);
    const cypher_astnode_t *delete_clause = _NEWAST_GetClause(ast, CYPHER_AST_DELETE);

    // Get defined identifiers.
    _NEWAST_GetIdentifiers(merge_clause, defined_aliases);
    _NEWAST_GetIdentifiers(match_clause, defined_aliases);
    _NEWAST_GetIdentifiers(unwind_clause, defined_aliases);

    // Get refereed identifiers.
    _NEWAST_GetIdentifiers(set_clause, refereed_identifiers);
    _NEWAST_GetIdentifiers(return_clause, refereed_identifiers);
    _NEWAST_GetIdentifiers(delete_clause, refereed_identifiers);
    _NEWAST_GetIdentifiers(unwind_clause, refereed_identifiers);

    char *alias;
    tm_len_t len;
    void *value;
    TrieMapIterator *it = TrieMap_Iterate(refereed_identifiers, "", 0);

    // See that each refereed identifier is defined.
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
    TrieMap_Free(refereed_identifiers, TrieMap_NOP_CB);
    return res;
}

AST_Validation NEWAST_Validate(const cypher_parse_result_t *ast, char **reason) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    
    if (_Validate_MATCH_Clause(root, reason) == AST_INVALID) {
        printf("_Validate_MATCH_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_CREATE_Clause(root, reason) == AST_INVALID) {
        printf("_Validate_CREATE_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_DELETE_Clause(root, reason) == AST_INVALID) {
        printf("_Validate_DELETE_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if (_Validate_RETURN_Clause(root, reason) == AST_INVALID) {
        printf("_Validate_RETURN_Clause, AST_INVALID\n");
        return AST_INVALID;
    }

    if(_NEWAST_Aliases_Defined(root, reason) == AST_INVALID) {
        printf("_NEWAST_Aliases_Defined, AST_INVALID\n");
        return AST_INVALID;
    }

    return AST_VALID;
}

bool NEWAST_ReadOnly(const cypher_parse_result_t *ast) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    return _NEWAST_ReadOnly(root);
}

const cypher_astnode_t *NEWAST_GetClause(const cypher_parse_result_t *ast, const char* clause) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    if(strcasecmp(clause, "create") == 0) {
        return _NEWAST_GetClause(root, CYPHER_AST_CREATE);
    } else if (strcasecmp(clause, "merge") == 0) {
        return _NEWAST_GetClause(root, CYPHER_AST_MERGE);
    } else if (strcasecmp(clause, "return") == 0) {
        return _NEWAST_GetClause(root, CYPHER_AST_RETURN);
    } else {
        // TODO: Log, unknown clause.
        assert(false);
    }

    return NULL;
}

bool NEWAST_ContainsClause(const cypher_parse_result_t *ast, const char* clause) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    if(strcasecmp(clause, "create") == 0) {
        return _NEWAST_GetClause(root, CYPHER_AST_CREATE) != NULL;
    } else if (strcasecmp(clause, "merge") == 0) {
        return _NEWAST_GetClause(root, CYPHER_AST_MERGE) != NULL;
    } else {
        // TODO: Log, unknown clause.
        assert(false);
    }

    return false;
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
void NEWAST_MatchClause_DefinedEntities(const cypher_parse_result_t *ast, TrieMap *definedEntities) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    const cypher_astnode_t *match_clause = _NEWAST_GetClause(root, CYPHER_AST_MATCH);
    
    _NEWAST_GetIdentifiers(match_clause, definedEntities);
}

//==============================================================================
//=== RETURN CLAUSE ============================================================
//==============================================================================
int NEWAST_ReturnClause_ContainsCollapsedNodes(const cypher_parse_result_t *ast) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    const cypher_astnode_t *return_clause = _NEWAST_GetClause(root, CYPHER_AST_RETURN);

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
//     const cypher_astnode_t *match = _NEWAST_GetClause(root, CYPHER_AST_MATCH);
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