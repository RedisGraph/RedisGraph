/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <assert.h>
#include <pthread.h>

#include "../util/arr.h"
#include "../util/qsort.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

extern pthread_key_t _tlsASTKey;  // Thread local storage AST key.

// Note each function call within given expression
// Example: given the expression: "abs(max(min(a), abs(k)))"
// referred_funcs will include: "abs", "max" and "min".
static void _consume_function_call_expression(const cypher_astnode_t *expression,
											  TrieMap *referred_funcs) {
	// Value is an apply operator
	const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(expression);
	const char *func_name = cypher_ast_function_name_get_value(func);
	TrieMap_Add(referred_funcs, (char *)func_name, strlen(func_name), NULL, TrieMap_DONT_CARE_REPLACE);

	uint narguments = cypher_ast_apply_operator_narguments(expression);
	for(int i = 0; i < narguments; i++) {
		const cypher_astnode_t *child_exp = cypher_ast_apply_operator_get_argument(expression, i);
		cypher_astnode_type_t child_exp_type = cypher_astnode_type(child_exp);
		if(child_exp_type != CYPHER_AST_APPLY_OPERATOR) continue;
		_consume_function_call_expression(child_exp, referred_funcs);
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
		   type == CYPHER_AST_CREATE_NODE_PROP_INDEX ||
		   type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
			return false;
		}
	}

	return true;
}

bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause) {
	return AST_GetClause(ast, clause) != NULL;
}

// Recursively collect the names of all function calls beneath a node
void AST_ReferredFunctions(const cypher_astnode_t *root, TrieMap *referred_funcs) {
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

void _AST_CollectAliases(const char ***aliases, const cypher_astnode_t *entity) {
	if(entity == NULL) return;

	if(cypher_astnode_type(entity) == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(entity);
		*aliases = array_append(*aliases, identifier);
		return;
	}

	uint nchildren = cypher_astnode_nchildren(entity);
	for(uint i = 0; i < nchildren; i ++) {
		_AST_CollectAliases(aliases, cypher_astnode_get_child(entity, i));
	}
}

// Collect aliases from clauses that introduce entities (MATCH, MERGE, CREATE, UNWIND)
const char **AST_CollectElementNames(AST *ast) {
	const char **aliases = array_new(const char *, 1);
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_MATCH  ||
		   type == CYPHER_AST_MERGE  ||
		   type == CYPHER_AST_CREATE ||
		   type == CYPHER_AST_UNWIND
		  ) {
			_AST_CollectAliases(&aliases, clause);
		}
	}

	// Trim array to only include unique aliases
#define ALIAS_STRCMP(a,b) (!strcmp(*a,*b))

	uint count = array_len(aliases);
	if(count == 0) return aliases;

	QSORT(const char *, aliases, count, ALIAS_STRCMP);
	uint unique_idx = 0;
	for(int i = 0; i < count - 1; i ++) {
		if(strcmp(aliases[i], aliases[i + 1])) {
			aliases[unique_idx++] = aliases[i];
		}
	}
	aliases[unique_idx++] = aliases[count - 1];
	array_trimm_len(aliases, unique_idx);

	return aliases;
}

AST *AST_Build(cypher_parse_result_t *parse_result) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->entity_map = NULL;

	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = cypher_parse_result_get_root(parse_result, 0);
	// We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag,
	// and double-checking this in AST validations
	assert(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

	ast->root = cypher_ast_statement_get_body(statement);
	// Empty queries should be captured by AST validations
	assert(ast->root);

	if(cypher_astnode_type(ast->root) == CYPHER_AST_QUERY) AST_BuildEntityMap(ast);

	// Set thread-local AST
	assert(pthread_setspecific(_tlsASTKey, ast) == 0);

	return ast;
}

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));

	uint n = end_offset - start_offset;

	cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_query_get_clause(master_ast->root, i + start_offset);
	}
	struct cypher_input_range range = {};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n, NULL, 0, range);

	// TODO This overwrites the previously-held AST pointer, which could lead to inconsistencies
	// in the future if we expect the variable to hold a different AST.
	assert(pthread_setspecific(_tlsASTKey, ast) == 0);
	AST_BuildEntityMap(ast);

	return ast;
}

// TODO Consider augmenting libcypher-parser so that we don't need to perform this
// work in-module.
long AST_ParseIntegerNode(const cypher_astnode_t *int_node) {
	assert(int_node);

	const char *value_str = cypher_ast_integer_get_valuestr(int_node);
	return strtol(value_str, NULL, 0);
}

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause) {
	assert(clause);

	bool aggregated = false;

	// Retrieve all user-specified functions in clause.
	TrieMap *referred_funcs = NewTrieMap();
	AST_ReferredFunctions(clause, referred_funcs);

	void *value;
	tm_len_t len;
	char *funcName;
	TrieMapIterator *it = TrieMap_Iterate(referred_funcs, "", 0);
	while(TrieMapIterator_Next(it, &funcName, &len, &value)) {
		if(Agg_FuncExists(funcName)) {
			aggregated = true;
			break;
		}
	}
	TrieMapIterator_Free(it);
	TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

	return aggregated;
}

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast) {
	int recordsCap = 16;    // Default.
	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	if(ret_clause == NULL) return recordsCap;
	// TODO Consider storing this number somewhere, as this logic is also in ExecutionPlan
	const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);
	if(limit_clause) {
		int limit = AST_ParseIntegerNode(limit_clause);
		recordsCap = MIN(recordsCap, limit);
	}
	return recordsCap;
}

AST *AST_GetFromTLS(void) {
	AST *ast = pthread_getspecific(_tlsASTKey);
	assert(ast);
	return ast;
}

void AST_Free(AST *ast) {
	if(ast == NULL) return;
	if(ast->entity_map) TrieMap_Free(ast->entity_map, TrieMap_NOP_CB);
	rm_free(ast);
}
