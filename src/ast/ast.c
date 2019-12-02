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
#include "ast_build_ar_exp.h"

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
	// Expression is an Apply or Apply All operator.
	bool apply_all = (cypher_astnode_type(expression) == CYPHER_AST_APPLY_ALL_OPERATOR);

	// Retrieve the function name and add to rax.
	const cypher_astnode_t *func = (!apply_all) ? cypher_ast_apply_operator_get_func_name(expression) :
								   cypher_ast_apply_all_operator_get_func_name(expression);
	const char *func_name = cypher_ast_function_name_get_value(func);
	raxInsert(referred_funcs, (unsigned char *)func_name, strlen(func_name), NULL, NULL);

	if(apply_all) return;  // Apply All operators have no arguments.

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
	} else if(cypher_astnode_type(project_clause) == CYPHER_AST_RETURN) {
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

/* This method extracts the query given parameters values, convert them into
 * constant arithmetic expressions and store them in a map of <name, value>
 * in the query context. */
static void _extract_params(const cypher_astnode_t *statement) {
	uint noptions = cypher_ast_statement_noptions(statement);
	if(noptions == 0) return;
	rax *params = QueryCtx_GetParams();
	for(uint i = 0; i < noptions; i++) {
		const cypher_astnode_t *option = cypher_ast_statement_get_option(statement, i);
		uint nparams = cypher_ast_cypher_option_nparams(option);
		for(uint j = 0; j < nparams; j++) {
			const cypher_astnode_t *param = cypher_ast_cypher_option_get_param(option, j);
			const char *paramName = cypher_ast_string_get_value(cypher_ast_cypher_option_param_get_name(param));
			const cypher_astnode_t *paramValue = cypher_ast_cypher_option_param_get_value(param);
			AR_ExpNode *exp = AR_EXP_FromExpression(paramValue);
			raxInsert(params, (unsigned char *) paramName, strlen(paramName), (void *)exp, NULL);
		}
	}
}

bool AST_ReadOnly(const cypher_parse_result_t *result) {
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

// Checks to see if an AST tree contains specified node type.
bool AST_TreeContainsType(const cypher_astnode_t *root, cypher_astnode_type_t search_type) {
	cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == search_type) return true;
	uint childCount = cypher_astnode_nchildren(root);
	for(uint i = 0; i < childCount; i++) {
		if(AST_TreeContainsType(cypher_astnode_get_child(root, i), search_type)) return true;
	}
	return false;
}

// Recursively collect the names of all function calls beneath a node
void AST_ReferredFunctions(const cypher_astnode_t *root, rax *referred_funcs) {
	cypher_astnode_type_t root_type = cypher_astnode_type(root);
	if(root_type == CYPHER_AST_APPLY_OPERATOR || root_type == CYPHER_AST_APPLY_ALL_OPERATOR) {
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
	ast->anot_ctx_collection = AST_AnnotationCtxCollection_New();
	ast->free_root = false;

	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = cypher_parse_result_get_root(parse_result, 0);
	// We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag,
	// and double-checking this in AST validations
	assert(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);
	// Extract the given query parameters value, and store them in query context.
	_extract_params(statement);
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
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
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
		/* Last clause is not necessarily a projection clause,
		 * [MATCH (a) RETURN a UNION] MATCH (a) RETURN a
		 * In this case project_clause = UNION, which is not a projection clause. */
		cypher_astnode_type_t project_type = cypher_astnode_type(project_clause);
		if(project_type == CYPHER_AST_UNION) project_clause = NULL;
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
	AnnotationCtx *name_ctx = AST_AnnotationCtxCollection_GetNameCtx(ast->anot_ctx_collection);
	return cypher_astnode_get_annotation(name_ctx, entity);
}

const char **AST_GetProjectAll(const cypher_astnode_t *projection_clause) {
	AST *ast = QueryCtx_GetAST();
	AnnotationCtx *project_all_ctx = AST_AnnotationCtxCollection_GetProjectAllCtx(
										 ast->anot_ctx_collection);
	return cypher_astnode_get_annotation(project_all_ctx, projection_clause);
}

const char **AST_BuildColumnNames(const cypher_astnode_t *return_clause) {
	const char **columns;
	if(cypher_ast_return_has_include_existing(return_clause)) {
		// If this is a RETURN *, the column names should be retrieved from the clause annotation.
		const char **projection_names = AST_GetProjectAll(return_clause);
		array_clone(columns, projection_names);
		return columns;
	}

	// Collect every alias from the RETURN projections.
	uint projection_count = cypher_ast_return_nprojections(return_clause);
	columns = array_new(const char *, projection_count);
	for(uint i = 0; i < projection_count; i++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(projection);
		// If the projection was not aliased, the projection itself is an identifier.
		if(ast_alias == NULL) ast_alias = cypher_ast_projection_get_expression(projection);
		const char *alias = cypher_ast_identifier_get_name(ast_alias);
		columns = array_append(columns, alias);
	}

	return columns;
}

// Determine the maximum number of records
// which will be considered when evaluating an algebraic expression.
int TraverseRecordCap(const AST *ast) {
	return MIN(ast->limit, 16);  // Use 16 as the default value.
}

inline AST_AnnotationCtxCollection *AST_GetAnnotationCtxCollection(AST *ast) {
	return ast->anot_ctx_collection;
}

void AST_Free(AST *ast) {
	if(ast == NULL) return;
	if(ast->referenced_entities) raxFree(ast->referenced_entities);
	if(ast->free_root) {
		// This is a generated AST, free its root node.
		cypher_astnode_free((cypher_astnode_t *)ast->root);
	} else {
		// This is the master AST, free the annotation contexts that have been constructed.
		AST_AnnotationCtxCollection_Free(ast->anot_ctx_collection);
	}
	rm_free(ast);
}

cypher_parse_result_t *parse(const char *query) {
	return cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
}

void parse_result_free(cypher_parse_result_t *parse_result) {
	if(parse_result) cypher_parse_result_free(parse_result);
}

