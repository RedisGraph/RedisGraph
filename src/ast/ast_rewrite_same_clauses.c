/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast.h"
#include "../query_ctx.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/sds/sds.h"
#include "../procedures/procedure.h"

static void replace_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	const cypher_astnode_t **paths = array_new(const cypher_astnode_t *, count);
	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clauses[i]);
		uint npaths = cypher_ast_pattern_npaths(pattern);
		for (uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			array_append(paths, cypher_ast_clone(path));	
		}
		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *pattern = cypher_ast_pattern(paths, array_len(paths), paths, array_len(paths), range);
	// build the replacement clause
	cypher_astnode_t *new_clause = cypher_ast_create(false, pattern, &pattern, 1, range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(paths);
}

static void foreach_replace_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	const cypher_astnode_t **paths = array_new(const cypher_astnode_t *, count);
	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clauses[i]);
		uint npaths = cypher_ast_pattern_npaths(pattern);
		for (uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			array_append(paths, cypher_ast_clone(path));	
		}
		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *pattern = cypher_ast_pattern(paths, array_len(paths), paths, array_len(paths), range);
	// build the replacement clause
	cypher_astnode_t *new_clause = cypher_ast_create(false, pattern, &pattern, 1, range);

	// replace original clause with fully populated one
	cypher_ast_foreach_replace_clauses(root, new_clause, scope_start, scope_end);

	array_free(paths);
}

static bool _Foreach_RewriteSameClauses(const cypher_astnode_t *foreach_clause) {
	bool rewritten = false;
	uint clause_count = cypher_ast_foreach_nclauses(foreach_clause);

	const cypher_astnode_t **clauses = array_new(const cypher_astnode_t *, 0);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_foreach_get_clause(foreach_clause, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);
		if(t == CYPHER_AST_FOREACH) {
			rewritten |= _Foreach_RewriteSameClauses(clause);
			continue;
		}
		if(t != CYPHER_AST_CREATE) continue;

		array_append(clauses, clause);

		uint j = i + 1;
		for (; j < clause_count; j++) {
			clause = cypher_ast_foreach_get_clause(foreach_clause, j);
			t = cypher_astnode_type(clause);
			if(t != CYPHER_AST_CREATE) break;
			array_append(clauses, clause);
		}
		j--;
		

		if(j - i > 0) {
			// multiple clauses with same type, replace them
			foreach_replace_clause(foreach_clause, clauses, i, j);
			rewritten = true;

			// update clause count
			clause_count -= j - i;
		}
		
		array_clear(clauses);
	}

	array_free(clauses);

	return rewritten;
}

bool AST_RewriteSameClauses
(
	cypher_parse_result_t *result
) {
	bool rewritten = false;
	// retrieve the statement node
	const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
	if(cypher_astnode_type(statement) != CYPHER_AST_STATEMENT) return rewritten;

	// retrieve the root query node from the statement
	const cypher_astnode_t *root = cypher_ast_statement_get_body(statement);
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return rewritten;

	// rewrite all WITH * / RETURN * clauses to include all aliases
	uint clause_count = cypher_ast_query_nclauses(root);

	const cypher_astnode_t **clauses = array_new(const cypher_astnode_t *, 0);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);
		if(t == CYPHER_AST_FOREACH) {
			rewritten |= _Foreach_RewriteSameClauses(clause);
			continue;
		}
		if(t != CYPHER_AST_CREATE) continue;

		array_append(clauses, clause);

		uint j = i + 1;
		for (; j < clause_count; j++) {
			clause = cypher_ast_query_get_clause(root, j);
			t = cypher_astnode_type(clause);
			if(t != CYPHER_AST_CREATE) break;
			array_append(clauses, clause);
		}
		j--;
		

		if(j - i > 0) {
			// multiple clauses with same type, replace them
			replace_clause(root, clauses, i, j);
			rewritten = true;

			// update clause count
			clause_count -= j - i;
		}
		array_clear(clauses);
	}

	array_free(clauses);

	return rewritten;
}

