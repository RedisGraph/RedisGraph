/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "newast.h"
#include <assert.h>

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

bool NEWAST_ReadOnly(const cypher_parse_result_t *ast) {
    const cypher_astnode_t *root = cypher_parse_result_get_root(ast, 0);
    return _NEWAST_ReadOnly(root);
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