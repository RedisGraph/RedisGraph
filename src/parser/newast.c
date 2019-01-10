/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "newast.h"

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
