/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast.h"
#include "../query_ctx.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/sds/sds.h"
#include "../procedures/procedure.h"

static void replace_create_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **paths = array_new(cypher_astnode_t *, count);
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

static void replace_match_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **paths = array_new(cypher_astnode_t *, count);
	cypher_astnode_t *predicate = NULL;
	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clauses[i]);
		uint npaths = cypher_ast_pattern_npaths(pattern);
		for (uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			array_append(paths, cypher_ast_clone(path));	
		}
		if(predicate == NULL) {
			predicate = (cypher_astnode_t *)cypher_ast_match_get_predicate(clauses[i]);
		} else {
			cypher_astnode_t *new_predicate = (cypher_astnode_t *)cypher_ast_match_get_predicate(clauses[i]);
			if(new_predicate != NULL) {
				cypher_astnode_t *children[] = {predicate, new_predicate};
				predicate = cypher_ast_binary_operator(CYPHER_OP_AND, predicate, new_predicate, children, 2, range);
			}
		}
		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *pattern = cypher_ast_pattern(paths, array_len(paths), paths, array_len(paths), range);
	// build the replacement clause
	cypher_astnode_t *children[] = {pattern, predicate};
	cypher_astnode_t *new_clause = cypher_ast_match(false, pattern, NULL, 0, predicate, children, 2, range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(paths);
}

static void replace_delete_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **exps = array_new(cypher_astnode_t *, count);
	for (uint i = 0; i < count; i++) {
		uint nexps = cypher_ast_delete_nexpressions(clauses[i]);
		for(uint j = 0; j < nexps; j++) {
			const cypher_astnode_t *exp = cypher_ast_delete_get_expression(clauses[i], j);
			array_append(exps, cypher_ast_clone(exp));
		}

		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *new_clause = cypher_ast_delete(false, exps, array_len(exps), exps, array_len(exps), range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(exps);
}


static void replace_set_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **items = array_new(cypher_astnode_t *, count);
	for (uint i = 0; i < count; i++) {
		uint nitems = cypher_ast_set_nitems(clauses[i]);
		for(uint j = 0; j < nitems; j++) {
			const cypher_astnode_t *item = cypher_ast_set_get_item(clauses[i], j);
			array_append(items, cypher_ast_clone(item));
		}

		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *new_clause = cypher_ast_set(items, array_len(items), items, array_len(items), range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(items);
}

static void replace_remove_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // begining of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **items = array_new(cypher_astnode_t *, count);
	for (uint i = 0; i < count; i++) {
		uint nitems = cypher_ast_remove_nitems(clauses[i]);
		for(uint j = 0; j < nitems; j++) {
			const cypher_astnode_t *item = cypher_ast_remove_get_item(clauses[i], j);
			array_append(items, cypher_ast_clone(item));
		}

		cypher_ast_free(clauses[i]);
	}
	
	cypher_astnode_t *new_clause = cypher_ast_remove(items, array_len(items), items, array_len(items), range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(items);
}

// returns true if type is compressible
static bool is_compressible
(
	cypher_astnode_type_t type
) {
	if(type == CYPHER_AST_CREATE || type == CYPHER_AST_MATCH ||
	   type == CYPHER_AST_DELETE || type == CYPHER_AST_SET ||
	   type == CYPHER_AST_REMOVE) return true;

	// type is not compressible
	return false;
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

	uint clause_count = cypher_ast_query_nclauses(root);

	cypher_astnode_t **clauses = array_new(cypher_astnode_t *, 0);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);
		if(!is_compressible(t)) continue;

		array_append(clauses, (cypher_astnode_t *)clause);

		uint j = i + 1;
		for (; j < clause_count; j++) {
			clause = cypher_ast_query_get_clause(root, j);
			cypher_astnode_type_t t2 = cypher_astnode_type(clause);
			if(t2 != t) break;
			array_append(clauses, (cypher_astnode_t *)clause);
		}
		j--;
		
		if(j - i > 0) {
			// multiple clauses with same type, replace them
			if(t == CYPHER_AST_CREATE) {
				replace_create_clause((cypher_astnode_t *)root, clauses, i, j);
			} else if(t == CYPHER_AST_MATCH) {
				replace_match_clause((cypher_astnode_t *)root, clauses, i, j);
			} else if(t == CYPHER_AST_DELETE) {
				replace_delete_clause((cypher_astnode_t *)root, clauses, i, j);
			} else if(t == CYPHER_AST_SET) {
				replace_set_clause((cypher_astnode_t *)root, clauses, i, j);
			} else if(t == CYPHER_AST_REMOVE) {
				replace_remove_clause((cypher_astnode_t *)root, clauses, i, j);
			}

			rewritten = true;

			// update clause count
			clause_count -= j - i;
		}
		array_clear(clauses);
	}

	array_free(clauses);

	return rewritten;
}
