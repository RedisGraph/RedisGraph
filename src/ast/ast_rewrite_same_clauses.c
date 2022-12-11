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

// compressing multiple consecutive CREATE clauses into a single CREATE clause
// this function collects all patterns scattered across multiple CREATE clauses
// and combines them into a single CREATE clause
static void replace_create_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // beginning of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **paths = array_new(cypher_astnode_t *, count);

	// collect paths
	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *pattern =
			cypher_ast_create_get_pattern(clauses[i]);
		uint npaths = cypher_ast_pattern_npaths(pattern);
		for (uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path =
				cypher_ast_pattern_get_path(pattern, j);
			array_append(paths, cypher_ast_clone(path));
		}
	}
	
	// build the replacement pattern
	cypher_astnode_t *pattern = cypher_ast_pattern(paths, array_len(paths),
			paths, array_len(paths), range);
	
	// build the replacement clause
	cypher_astnode_t *new_clause = cypher_ast_create(false, pattern, &pattern,
			1, range);

	// replace original clause with the new one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(paths);
}

// compressing multiple consecutive MATCH clauses into a single MATCH clause
// this function collects all patterns scattered across multiple MATCH clauses
// and combines them into a single MATCH clause
static void replace_match_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // beginning of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	cypher_astnode_t *predicate = NULL;
	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **paths = array_new(cypher_astnode_t *, count);

	// collect MATCH patterns and predicates
	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *pattern =
			cypher_ast_match_get_pattern(clauses[i]);
		uint npaths = cypher_ast_pattern_npaths(pattern);

		for (uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path =
				cypher_ast_pattern_get_path(pattern, j);
			array_append(paths, cypher_ast_clone(path));	
		}

		// combine MATCH predicates into a single predicate using AND connectors
		cypher_astnode_t *new_predicate =
			(cypher_astnode_t *)cypher_ast_match_get_predicate(clauses[i]);
		if(new_predicate != NULL) {
			new_predicate = cypher_ast_clone(new_predicate);
			if(predicate == NULL) {
				predicate = new_predicate;
			} else {
				// concat using AND
				cypher_astnode_t *children[] = {predicate, new_predicate};
				predicate = cypher_ast_binary_operator(CYPHER_OP_AND, predicate,
						new_predicate, children, 2, range);
			}
		}
	}
	
	// build the replacement pattern
	cypher_astnode_t *pattern =
		cypher_ast_pattern(paths, array_len(paths), paths, array_len(paths),
				range);
	
	// build the replacement clause
	cypher_astnode_t *children[] = {pattern, predicate};
	cypher_astnode_t *new_clause = cypher_ast_match(false, pattern, NULL, 0,
			predicate, children, predicate == NULL ? 1 : 2, range);

	// replace original clause with the new one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(paths);
}

// compressing multiple consecutive DELETE clauses into a single DELETE clause
// this function collects all expressions scattered across multiple DELETE
// clauses and combines them into a single DELETE clause
static void replace_delete_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // beginning of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **exps = array_new(cypher_astnode_t *, count);
	rax *identifiers = raxNew();

	// collect expressions
	for (uint i = 0; i < count; i++) {
		uint nexps = cypher_ast_delete_nexpressions(clauses[i]);
		for(uint j = 0; j < nexps; j++) {
			const cypher_astnode_t *exp =
				cypher_ast_delete_get_expression(clauses[i], j);
			
			// do not aggregate multiple appearances of an identifier
			if(cypher_astnode_type(exp) == CYPHER_AST_IDENTIFIER) {
				const char *identifier = cypher_ast_identifier_get_name(exp);
				if(raxTryInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL)) {
					array_append(exps, cypher_ast_clone(exp));
				}
			}
		}
	}
	raxFree(identifiers);

	// build the replacement clause
	cypher_astnode_t *new_clause = cypher_ast_delete(false, exps,
			array_len(exps), exps, array_len(exps), range);

	// replace original clause with the new one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(exps);
}

// compressing multiple consecutive SET clauses into a single SET clause
// this function collects all expressions scattered across multiple SET
// clauses and combines them into a single SET clause
static void replace_set_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // beginning of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **items = array_new(cypher_astnode_t *, count);

	for (uint i = 0; i < count; i++) {
		uint nitems = cypher_ast_set_nitems(clauses[i]);
		for(uint j = 0; j < nitems; j++) {
			const cypher_astnode_t *item =
				cypher_ast_set_get_item(clauses[i], j);
			array_append(items, cypher_ast_clone(item));
		}
	}
	
	// build the replacement clause
	cypher_astnode_t *new_clause = cypher_ast_set(items, array_len(items),
			items, array_len(items), range);

	// replace original clause with the new one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(items);
}

// compressing multiple consecutive REMOVE clauses into a single REMOVE clause
// this function collects all expressions scattered across multiple REMOVE
// clauses and combines them into a single REMOVE clause
static void replace_remove_clause
(
	cypher_astnode_t *root,      // ast root
	cypher_astnode_t **clauses,  // clause being replaced
	int scope_start,             // beginning of scope
	int scope_end                // ending of scope
) {
	uint count = array_len(clauses);

	struct cypher_input_range range = cypher_astnode_range(clauses[0]);
	cypher_astnode_t **items = array_new(cypher_astnode_t *, count);

	for (uint i = 0; i < count; i++) {
		uint nitems = cypher_ast_remove_nitems(clauses[i]);
		for(uint j = 0; j < nitems; j++) {
			const cypher_astnode_t *item =
				cypher_ast_remove_get_item(clauses[i], j);
			array_append(items, cypher_ast_clone(item));
		}
	}
	
	// build the replacement clause
	cypher_astnode_t *new_clause =
		cypher_ast_remove(items, array_len(items), items, array_len(items),
				range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(root, new_clause, scope_start, scope_end);
	
	array_free(items);
}

// returns true if AST clause type is compressible
static inline bool is_compressible
(
	const cypher_astnode_t *clause
) {
	// compressible clauses:
	// 1. None OPTIONAL MATCH
	// 2. CREATE
	// 3. SET
	// 4. DELETE
	// 5. REMOVE
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	return ( (t == CYPHER_AST_MATCH &&
			  !cypher_ast_match_is_optional(clause)) ||
			  t == CYPHER_AST_CREATE                 ||
			  t == CYPHER_AST_SET                    ||
			  t == CYPHER_AST_DELETE                 ||
			  t == CYPHER_AST_REMOVE);
}

// rewrite result by compressing consecutive clauses of the same type
// to a single clause, returning true if the rewrite has been performed
bool AST_RewriteSameClauses
(
	const cypher_astnode_t *root // root of AST
) {
	bool rewritten = false;
	
	if(cypher_astnode_type(root) != CYPHER_AST_STATEMENT) {
		return rewritten;
	}

	// retrieve the root's body
	const cypher_astnode_t *body = cypher_ast_statement_get_body(root);
	if(cypher_astnode_type(body) != CYPHER_AST_QUERY) {
		return rewritten;
	}

	uint clause_count = cypher_ast_query_nclauses(body);

	// traverse clauses
	// compress consecutive clauses
	cypher_astnode_t **clauses = array_new(cypher_astnode_t *, 0);

	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(body, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);

		// check compressibility, move on if not compressible
		if(!is_compressible(clause)) {
			continue;
		}

		//----------------------------------------------------------------------
		// collect clauses of the same type as current clause
		//----------------------------------------------------------------------

		for (uint j = i; j < clause_count; j++) {
			clause = cypher_ast_query_get_clause(body, j);
			cypher_astnode_type_t t2 = cypher_astnode_type(clause);
			if(t2 != t || !is_compressible(clause)) {
				break;
			}
			array_append(clauses, (cypher_astnode_t *)clause);
		}

		//----------------------------------------------------------------------
		// compress clauses
		//----------------------------------------------------------------------

		uint s = i;
		uint e = s + array_len(clauses) - 1;
		if(array_len(clauses) > 1) {
			// multiple consecutive clauses of the same type
			// compress them
			if(t == CYPHER_AST_CREATE) {
				replace_create_clause((cypher_astnode_t *)body, clauses, s, e);
			} else if(t == CYPHER_AST_MATCH) {
				replace_match_clause((cypher_astnode_t *)body, clauses, s, e);
			} else if(t == CYPHER_AST_DELETE) {
				replace_delete_clause((cypher_astnode_t *)body, clauses, s, e);
			} else if(t == CYPHER_AST_SET) {
				replace_set_clause((cypher_astnode_t *)body, clauses, s, e);
			} else if(t == CYPHER_AST_REMOVE) {
				replace_remove_clause((cypher_astnode_t *)body, clauses, s, e);
			}

			rewritten = true;

			// update clause count, skip compressed clauses
			clause_count -= array_len(clauses) - 1;
		}
		array_clear(clauses);
	}

	array_free(clauses);

	return rewritten;
}

