/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <assert.h>
#include <pthread.h>

#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/qsort.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

// TODO duplicated logic, find shared place for it
static inline void _prepareIterateAll(rax *map, raxIterator *iter) {
	raxStart(iter, map);
	raxSeek(iter, "^", NULL, 0);
}

// Note each function call within given expression
// Example: given the expression: "abs(max(min(a), abs(k)))"
// referred_funcs will include: "abs", "max" and "min".
static void _consume_function_call_expression(const cypher_astnode_t *expression,
											  rax *referred_funcs) {
	// Value is an apply operator
	const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(expression);
	const char *func_name = cypher_ast_function_name_get_value(func);
	raxInsert(referred_funcs, (unsigned char *)func_name, strlen(func_name), NULL, NULL);

	uint narguments = cypher_ast_apply_operator_narguments(expression);
	for(int i = 0; i < narguments; i++) {
		const cypher_astnode_t *child_exp = cypher_ast_apply_operator_get_argument(expression, i);
		cypher_astnode_type_t child_exp_type = cypher_astnode_type(child_exp);
		if(child_exp_type != CYPHER_AST_APPLY_OPERATOR) continue;
		_consume_function_call_expression(child_exp, referred_funcs);
	}
}

static inline int _get_limit(const cypher_astnode_t *project_clause) {
	const cypher_astnode_t *limit_node = NULL;
	// Retrieve the AST LIMIT node if one is specified.
	if(cypher_astnode_type(project_clause) == CYPHER_AST_WITH) {
		limit_node = cypher_ast_with_get_limit(project_clause);
	} else {
		limit_node = cypher_ast_return_get_limit(project_clause);
	}

	if(limit_node == NULL) return UNLIMITED;
	// Parse the LIMIT value.
	return AST_ParseIntegerNode(limit_node);
}

// If the project clause has a LIMIT modifier, set its value in the constructed AST.
static void _AST_LimitResults(AST *ast, const cypher_astnode_t *root_clause,
							  const cypher_astnode_t *project_clause) {
	cypher_astnode_type_t root_type = cypher_astnode_type(root_clause);
	if(root_type == CYPHER_AST_RETURN || root_type == CYPHER_AST_WITH) {
		// Use the root clause of this AST if it is a projection.
		ast->limit = _get_limit(root_clause);
	} else if(project_clause) {
		// Use the subsequent projection clause (if one is provided) otherwise.
		ast->limit = _get_limit(project_clause);
	}
}

bool AST_ReadOnly(const cypher_parse_result_t *result) {
	// A lot of these steps will be unnecessary once we move
	// parsing into the subthread (and can thus perform this check
	// after validations).

	// Check for failures in libcypher-parser
	if(AST_ContainsErrors(result)) return true;

	const cypher_astnode_t *root = cypher_parse_result_get_root(result, 0);
	// Check for empty query
	if(root == NULL) return true;

	const cypher_astnode_t *body = cypher_ast_statement_get_body(root);
	// Iterate over children rather than clauses, as the root is not
	// guaranteed to be a query.
	uint num_children = cypher_astnode_nchildren(body);
	for(uint i = 0; i < num_children; i ++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(body, i);
		cypher_astnode_type_t type = cypher_astnode_type(child);
		if(type == CYPHER_AST_CREATE                      ||
		   type == CYPHER_AST_MERGE                  ||
		   type == CYPHER_AST_DELETE                 ||
		   type == CYPHER_AST_SET                    ||
		   type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
		   type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
			return false;
		}
	}

	return true;
}

inline bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause) {
	return AST_GetClause(ast, clause) != NULL;
}

// Recursively collect the names of all function calls beneath a node
void AST_ReferredFunctions(const cypher_astnode_t *root, rax *referred_funcs) {
	cypher_astnode_type_t root_type = cypher_astnode_type(root);
	if(root_type == CYPHER_AST_APPLY_OPERATOR) {
		_consume_function_call_expression(root, referred_funcs);
	} else {
		uint child_count = cypher_astnode_nchildren(root);
		for(int i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
			AST_ReferredFunctions(child, referred_funcs);
		}
	}
}


// Retrieve the first instance of the specified clause in the AST segment, if any.
const cypher_astnode_t *AST_GetClause(const AST *ast, cypher_astnode_type_t clause_type) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) == clause_type) return child;
	}

	return NULL;
}

uint *AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type) {
	uint *clause_indices = array_new(uint, 1);
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		if(cypher_astnode_type(cypher_ast_query_get_clause(ast->root, i)) == clause_type) {
			clause_indices = array_append(clause_indices, i);
		}
	}
	return clause_indices;
}

uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	uint num_found = 0;
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) == clause_type) num_found ++;
	}
	return num_found;
}

/* Collect references to all clauses of the specified type in the query. Since clauses
 * cannot be nested, we only need to check the immediate children of the query node. */
const cypher_astnode_t **AST_GetClauses(const AST *ast, cypher_astnode_type_t type) {
	uint count = AST_GetClauseCount(ast, type);
	if(count == 0) return NULL;

	const cypher_astnode_t **found = array_new(const cypher_astnode_t *, count);

	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) != type) continue;

		found = array_append(found, child);
	}

	return found;
}

AST *AST_Build(cypher_parse_result_t *parse_result) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->referenced_entities = NULL;
	ast->name_ctx = NULL;
	ast->project_all_ctx = NULL;
	ast->free_root = false;

	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = cypher_parse_result_get_root(parse_result, 0);
	// We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag,
	// and double-checking this in AST validations
	assert(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

	ast->root = cypher_ast_statement_get_body(statement);

	// Empty queries should be captured by AST validations
	assert(ast->root);

	// Set thread-local AST.
	QueryCtx_SetAST(ast);

	// Augment the AST with annotations for naming entities and populating WITH/RETURN * projections.
	AST_Enrich(ast);

	return ast;
}

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->name_ctx = master_ast->name_ctx;
	ast->project_all_ctx = master_ast->project_all_ctx;
	ast->free_root = true;
	ast->limit = UNLIMITED;
	uint n = end_offset - start_offset;

	const cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = cypher_ast_query_get_clause(master_ast->root, i + start_offset);
	}
	struct cypher_input_range range = {};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n,
								 (cypher_astnode_t **)clauses, n, range);

	// TODO This overwrites the previously-held AST pointer, which could lead to inconsistencies
	// in the future if we expect the variable to hold a different AST.
	QueryCtx_SetAST(ast);

	// If the segments are split, the next clause is either RETURN or WITH,
	// and its references should be included in this segment's map.
	const cypher_astnode_t *project_clause = NULL;
	uint clause_count = cypher_ast_query_nclauses(master_ast->root);
	if(clause_count > 1 && end_offset < clause_count) {
		project_clause = cypher_ast_query_get_clause(master_ast->root, end_offset);
	}

	// Set the max number of results for this AST if a LIMIT modifier is specified.
	_AST_LimitResults(ast, clauses[0], project_clause);

	// Build the map of referenced entities in this AST segment.
	AST_BuildReferenceMap(ast, project_clause);

	return ast;
}

inline bool AST_AliasIsReferenced(AST *ast, const char *alias) {
	return (raxFind(ast->referenced_entities, (unsigned char *)alias, strlen(alias)) != raxNotFound);
}

// TODO Consider augmenting libcypher-parser so that we don't need to perform this
// work in-module.
inline long AST_ParseIntegerNode(const cypher_astnode_t *int_node) {
	assert(int_node);

	const char *value_str = cypher_ast_integer_get_valuestr(int_node);
	return strtol(value_str, NULL, 0);
}

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause) {
	assert(clause);

	bool aggregated = false;

	// Retrieve all user-specified functions in clause.
	rax *referred_funcs = raxNew();
	AST_ReferredFunctions(clause, referred_funcs);

	char funcName[32];
	raxIterator it;
	_prepareIterateAll(referred_funcs, &it);
	while(raxNext(&it)) {
		size_t len = it.key_len;
		assert(len < 32);
		// Copy the triemap key so that we can safely add a terinator character
		memcpy(funcName, it.key, len);
		funcName[len] = 0;

		if(Agg_FuncExists(funcName)) {
			aggregated = true;
			break;
		}
	}
	raxStop(&it);
	raxFree(referred_funcs);

	return aggregated;
}

const char *AST_GetEntityName(const AST *ast, const cypher_astnode_t *entity) {
	return cypher_astnode_get_annotation(ast->name_ctx, entity);
}

const char **AST_GetProjectAll(const cypher_astnode_t *projection_clause) {
	AST *ast = QueryCtx_GetAST();
	return cypher_astnode_get_annotation(ast->project_all_ctx, projection_clause);
}

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast) {
	return MIN(ast->limit, 16);  // Use 16 as the default value.
}

void AST_Free(AST *ast) {
	if(ast == NULL) return;
	if(ast->referenced_entities) raxFree(ast->referenced_entities);
	if(ast->free_root) {
		// This is a generated AST, free its root node.
		cypher_astnode_free((cypher_astnode_t *)ast->root);
	} else {
		// This is the master AST, free the annotation contexts that have been constructed.
		if(ast->name_ctx) cypher_ast_annotation_context_free(ast->name_ctx);
		if(ast->project_all_ctx) cypher_ast_annotation_context_free(ast->project_all_ctx);
	}
	rm_free(ast);
}

cypher_parse_result_t *parse(const char *query) {
	return cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
}

void parse_result_free(cypher_parse_result_t *parse_result) {
	if(parse_result) cypher_parse_result_free(parse_result);
}

