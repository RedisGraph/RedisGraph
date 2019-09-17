/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "parser.h"
#include <assert.h>

static void _name_anonymous_entities_in_path(const cypher_astnode_t *path, int *anon_count) {
	char *alias;
	uint nelems = cypher_ast_pattern_path_nelements(path);
	for(uint k = 0; k < nelems; k++) {
		const cypher_astnode_t *identifier = NULL;
		const cypher_astnode_t *element = cypher_ast_pattern_path_get_element(path, k);
		cypher_astnode_type_t element_type = cypher_astnode_type(element);

		assert(element_type == CYPHER_AST_NODE_PATTERN || element_type == CYPHER_AST_REL_PATTERN);
		if(element_type == CYPHER_AST_NODE_PATTERN) {
			identifier = cypher_ast_node_pattern_get_identifier(element);
		} else {
			identifier = cypher_ast_rel_pattern_get_identifier(element);
		}

		if(!identifier) {
			// Create  and set identifier.
			struct cypher_input_range range = cypher_astnode_range(element);
			int alias_len = asprintf(&alias, "anon_%d", (*anon_count)++);
			identifier = cypher_ast_identifier(alias, alias_len, range);
			assert(identifier);
			if(element_type == CYPHER_AST_NODE_PATTERN) {
				cypher_ast_node_pattern_set_identifier(element, identifier);
			} else {
				cypher_ast_rel_pattern_set_identifier(element, identifier);
			}
			free(alias);
		}
	}

}

/* Name each anonymous graph entity */
static void _name_anonymous_entities_in_pattern(const cypher_astnode_t *root) {
	/* Graph entities can be found in
	 * MATCH, MERGE and CREATE clauses */

	int anon_count = 0;
	uint clause_count = cypher_ast_query_nclauses(root);

	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t clause_type = cypher_astnode_type(clause);
		const cypher_astnode_t *pattern = NULL;
		if(clause_type == CYPHER_AST_MATCH) {
			pattern = cypher_ast_match_get_pattern(clause);
		} else if(clause_type == CYPHER_AST_CREATE) {
			pattern = cypher_ast_create_get_pattern(clause);
		} else if(clause_type == CYPHER_AST_MERGE) {
			const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
			_name_anonymous_entities_in_path(path, &anon_count);
			continue;
		}
		if(!pattern) continue;

		uint npaths = cypher_ast_pattern_npaths(pattern);
		for(uint j = 0; j < npaths; j ++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			_name_anonymous_entities_in_path(path, &anon_count);
		}
	}
}

static void _enrich_ast(const cypher_astnode_t *root) {
	/* Directives like CREATE INDEX are not queries. */
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return;
	_name_anonymous_entities_in_pattern(root);
}

cypher_parse_result_t *parse(const char *query) {
	cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	if(!parse_result) return NULL;

	/* Retrieve the AST root node from a parsed query.
	 * We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag. */
	const cypher_astnode_t *root = cypher_parse_result_get_root(parse_result, 0);

	// Return if the query root was an unexpected type such as an error.
	if(cypher_astnode_type(root) != CYPHER_AST_STATEMENT) return parse_result;

	_enrich_ast(cypher_ast_statement_get_body(root));
	return parse_result;
}

