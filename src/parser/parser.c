/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "parser.h"
#include <assert.h>

// Recursively compute the number of digits in a positive integer.
static inline int _digit_count(int n) {
	if(n < 10) return 1;
	return 1 + _digit_count(n / 10);
}

static cypher_astnode_t *_create_anon_identifier(const cypher_astnode_t *node, int anon_count) {
	// We need space for "anon_" (5), all digits, and a NULL terminator (1)
	int alias_len = _digit_count(anon_count) + 6;
	char alias[alias_len];
	sprintf(alias, "anon_%d", anon_count);
	struct cypher_input_range range = cypher_astnode_range(node);
	cypher_astnode_t *identifier = cypher_ast_identifier(alias, alias_len, range);
	return identifier;
}

static void _name_anonymous_entities_in_pattern(const cypher_astnode_t *node, uint *anon_count) {
	cypher_astnode_type_t t = cypher_astnode_type(node);

	if(t == CYPHER_AST_NODE_PATTERN && !cypher_ast_node_pattern_get_identifier(node)) {
		// AST entity is an unaliased node, create and set anonymous identifier.
		cypher_astnode_t *identifier = _create_anon_identifier(node, (*anon_count)++);
		cypher_ast_node_pattern_set_identifier((cypher_astnode_t *)node, identifier);
		return;
	} else if(t == CYPHER_AST_REL_PATTERN && !cypher_ast_rel_pattern_get_identifier(node)) {
		// AST entity is an unaliased edge, create and set anonymous identifier.
		cypher_astnode_t *identifier = _create_anon_identifier(node, (*anon_count)++);
		cypher_ast_rel_pattern_set_identifier((cypher_astnode_t *)node, identifier);
		return;
	}

	uint child_count = cypher_astnode_nchildren(node);
	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		_name_anonymous_entities_in_pattern(child, anon_count);
	}
}

static void _enrich_ast(const cypher_astnode_t *root) {
	/* Directives like CREATE INDEX are not queries. */
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return;
	uint anon_count = 0;
	_name_anonymous_entities_in_pattern(root, &anon_count);
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

