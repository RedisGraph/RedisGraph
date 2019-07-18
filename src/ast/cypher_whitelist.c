/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cypher_whitelist.h"
#include "../../deps/rax/rax.h"

static rax *_whitelist;

void CypherWhitelist_Build() {
    _whitelist = raxNew();
	// The end_of_list token allows us to modify this array without worrying about its length
#define end_of_list UINT8_MAX
	// When we introduce support for one of these, simply remove it from the list.
    cypher_astnode_type_t supported_entities[] = {
                                                  // CYPHER_AST_STATEMENT,
                                                  // CYPHER_AST_STATEMENT_OPTION,
                                                  // CYPHER_AST_CYPHER_OPTION,
                                                  // CYPHER_AST_CYPHER_OPTION_PARAM,
                                                  // CYPHER_AST_EXPLAIN_OPTION,
                                                  // CYPHER_AST_PROFILE_OPTION,
                                                  // CYPHER_AST_SCHEMA_COMMAND,
                                                  CYPHER_AST_CREATE_NODE_PROP_INDEX,
                                                  CYPHER_AST_DROP_NODE_PROP_INDEX,
                                                  // CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT,
                                                  // CYPHER_AST_DROP_NODE_PROP_CONSTRAINT,
                                                  // CYPHER_AST_CREATE_REL_PROP_CONSTRAINT,
                                                  // CYPHER_AST_DROP_REL_PROP_CONSTRAINT,
                                                  CYPHER_AST_QUERY,
                                                  // CYPHER_AST_QUERY_OPTION,
                                                  // CYPHER_AST_USING_PERIODIC_COMMIT,
                                                  CYPHER_AST_QUERY_CLAUSE,
                                                  // CYPHER_AST_LOAD_CSV,
                                                  // CYPHER_AST_START,
                                                  // CYPHER_AST_START_POINT,
                                                  // CYPHER_AST_NODE_INDEX_LOOKUP,
                                                  // CYPHER_AST_NODE_INDEX_QUERY,
                                                  // CYPHER_AST_NODE_ID_LOOKUP,
                                                  // CYPHER_AST_ALL_NODES_SCAN,
                                                  // CYPHER_AST_REL_INDEX_LOOKUP,
                                                  // CYPHER_AST_REL_INDEX_QUERY,
                                                  // CYPHER_AST_REL_ID_LOOKUP,
                                                  // CYPHER_AST_ALL_RELS_SCAN,
                                                  CYPHER_AST_MATCH,
                                                  // CYPHER_AST_MATCH_HINT,
                                                  // CYPHER_AST_USING_INDEX,
                                                  // CYPHER_AST_USING_JOIN,
                                                  // CYPHER_AST_USING_SCAN,
                                                  CYPHER_AST_MERGE,
                                                  // CYPHER_AST_MERGE_ACTION,
                                                  // CYPHER_AST_ON_MATCH,
                                                  // CYPHER_AST_ON_CREATE,
                                                  CYPHER_AST_CREATE,
                                                  CYPHER_AST_SET,
                                                  CYPHER_AST_SET_ITEM,
                                                  CYPHER_AST_SET_PROPERTY,
                                                  // CYPHER_AST_SET_ALL_PROPERTIES,
                                                  CYPHER_AST_MERGE_PROPERTIES,
                                                  // CYPHER_AST_SET_LABELS,
                                                  CYPHER_AST_DELETE,
                                                  // CYPHER_AST_REMOVE,
                                                  // CYPHER_AST_REMOVE_ITEM,
                                                  // CYPHER_AST_REMOVE_LABELS,
                                                  // CYPHER_AST_REMOVE_PROPERTY,
                                                  // CYPHER_AST_FOREACH,
                                                  CYPHER_AST_WITH,
                                                  CYPHER_AST_UNWIND,
                                                  CYPHER_AST_CALL,
                                                  CYPHER_AST_RETURN,
                                                  CYPHER_AST_PROJECTION,
                                                  CYPHER_AST_ORDER_BY,
                                                  CYPHER_AST_SORT_ITEM,
                                                  // CYPHER_AST_UNION,
                                                  CYPHER_AST_EXPRESSION,
                                                  CYPHER_AST_UNARY_OPERATOR,
                                                  CYPHER_AST_BINARY_OPERATOR,
                                                  CYPHER_AST_COMPARISON,
                                                  CYPHER_AST_APPLY_OPERATOR,
                                                  // CYPHER_AST_APPLY_ALL_OPERATOR,
                                                  CYPHER_AST_PROPERTY_OPERATOR,
                                                  // CYPHER_AST_SUBSCRIPT_OPERATOR,
                                                  // CYPHER_AST_SLICE_OPERATOR,
                                                  // CYPHER_AST_LABELS_OPERATOR,
                                                  // CYPHER_AST_LIST_COMPREHENSION,
                                                  // CYPHER_AST_PATTERN_COMPREHENSION,
                                                  // CYPHER_AST_CASE,
                                                  // CYPHER_AST_FILTER,
                                                  // CYPHER_AST_EXTRACT,
                                                  // CYPHER_AST_REDUCE,
                                                  // CYPHER_AST_ALL,
                                                  // CYPHER_AST_ANY,
                                                  // CYPHER_AST_SINGLE,
                                                  // CYPHER_AST_NONE,
                                                  CYPHER_AST_COLLECTION,
                                                  CYPHER_AST_MAP,
                                                  CYPHER_AST_IDENTIFIER,
                                                  // CYPHER_AST_PARAMETER,
                                                  CYPHER_AST_STRING,
                                                  CYPHER_AST_INTEGER,
                                                  CYPHER_AST_FLOAT,
                                                  CYPHER_AST_BOOLEAN,
                                                  CYPHER_AST_TRUE,
                                                  CYPHER_AST_FALSE,
                                                  CYPHER_AST_NULL,
                                                  CYPHER_AST_LABEL,
                                                  CYPHER_AST_RELTYPE,
                                                  CYPHER_AST_PROP_NAME,
                                                  CYPHER_AST_FUNCTION_NAME,
                                                  // CYPHER_AST_INDEX_NAME,
                                                  CYPHER_AST_PROC_NAME,
                                                  CYPHER_AST_PATTERN,
                                                  // CYPHER_AST_NAMED_PATH,
                                                  // CYPHER_AST_SHORTEST_PATH,
                                                  CYPHER_AST_PATTERN_PATH,
                                                  CYPHER_AST_NODE_PATTERN,
                                                  CYPHER_AST_REL_PATTERN,
                                                  CYPHER_AST_RANGE,
                                                  // CYPHER_AST_COMMAND,
                                                  // CYPHER_AST_COMMENT,
                                                  // CYPHER_AST_LINE_COMMENT,
                                                  // CYPHER_AST_BLOCK_COMMENT,
                                                  CYPHER_AST_ERROR,
                                                  // CYPHER_AST_MAP_PROJECTION,
                                                  // CYPHER_AST_MAP_PROJECTION_SELECTOR,
                                                  // CYPHER_AST_MAP_PROJECTION_LITERAL,
                                                  // CYPHER_AST_MAP_PROJECTION_PROPERTY,
                                                  // CYPHER_AST_MAP_PROJECTION_IDENTIFIER,
                                                  // CYPHER_AST_MAP_PROJECTION_ALL_PROPERTIES,
                                                  end_of_list
    };

    int i = 0;
    cypher_astnode_type_t supported_entity;
    while ((supported_entity = supported_entities[i++]) != end_of_list) {
		// Introduce every type to the whitelist rax
        raxInsert(_whitelist, (unsigned char*)&supported_entity, sizeof(supported_entity), NULL, NULL);
    }
}

AST_Validation CypherWhitelist_ValidateQuery(const cypher_astnode_t *elem, char **reason) {
    if (elem == NULL) return AST_VALID;
    cypher_astnode_type_t type = cypher_astnode_type(elem);
    if (raxFind(_whitelist, (unsigned char*)&type, sizeof(type)) == raxNotFound) {
       asprintf(reason, "RedisGraph does not currently support %s", cypher_astnode_typestr(type));
       return AST_INVALID;
    }

    uint nchildren = cypher_astnode_nchildren(elem);
    for (uint i = 0; i < nchildren; i ++) {
        if (CypherWhitelist_ValidateQuery(cypher_astnode_get_child(elem, i), reason) != AST_VALID) {
            return AST_INVALID;
        }
    }

    return AST_VALID;
}
