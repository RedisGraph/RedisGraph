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

bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause) {
	return AST_GetClause(ast, clause) != NULL;
}

inline bool AST_ContainsReturn(const AST *ast) {
	uint final_idx = cypher_ast_query_nclauses(ast->root) - 1;
	return cypher_astnode_type(cypher_ast_query_get_clause(ast->root, final_idx)) == CYPHER_AST_RETURN;
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
	ast->referenced_entities = NULL;
	ast->free_root = false;

	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = cypher_parse_result_get_root(parse_result, 0);
	// We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag,
	// and double-checking this in AST validations
	assert(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

	ast->root = cypher_ast_statement_get_body(statement);
	// Empty queries should be captured by AST validations
	assert(ast->root);

	// Set thread-local AST
	QueryCtx_SetAST(ast);

	return ast;
}

/*=============================================================================
 * ================== AST segment referenced entities mapping =================
 * ==========================================================================*/


void _AST_MapWhereClause(AST *ast, const cypher_astnode_t *predicate) {
	cypher_astnode_type_t type = cypher_astnode_type(predicate);

	if(type == CYPHER_AST_IDENTIFIER) {
		const char	*identifierName = cypher_ast_identifier_get_name(predicate);
		raxInsert(ast->referenced_entities, identifierName, strlen(identifierName), true, NULL);
	} else if(type == CYPHER_AST_PROPERTY_OPERATOR) {
		predicate = cypher_ast_property_operator_get_expression(predicate);
		assert(cypher_astnode_type(predicate) == CYPHER_AST_IDENTIFIER);
		const char	*identifierName = cypher_ast_identifier_get_name(predicate);
		raxInsert(ast->referenced_entities, identifierName, strlen(identifierName), true, NULL);
	} else {
		uint child_count = cypher_astnode_nchildren(predicate);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(predicate, i);
			// Recursively continue mapping.
			_AST_MapWhereClause(ast, child);
		}
	}
}

void _AST_MapProjectionReference(AST *ast, const cypher_astnode_t *projection) {
	const cypher_astnode_t *ast_identifier = cypher_ast_projection_get_alias(projection);
	const char *identifierName = NULL;
	if(ast_identifier == NULL) {
		// The projection was not aliased.
		ast_identifier = cypher_ast_projection_get_expression(projection);
		assert(cypher_astnode_type(ast_identifier) == CYPHER_AST_IDENTIFIER);
	}
	identifierName = cypher_ast_identifier_get_name(ast_identifier);
	// TODO: project property
	raxInsert(ast->referenced_entities, identifierName, strlen(identifierName), true, NULL);
}

void _AST_MapUnwindReferences(AST *ast, const cypher_astnode_t *unwindClause) {
	const cypher_astnode_t *alias = cypher_ast_unwind_get_alias(unwindClause);
	const char *aliasName = cypher_ast_identifier_get_name(alias);
	raxInsert(ast->referenced_entities, aliasName, strlen(aliasName), true, NULL);
}

void _AST_MapOrderByReferences(AST *ast, const cypher_astnode_t *orderByClause) {
	uint orderByItemsCount = cypher_ast_order_by_nitems(orderByClause);
	for(uint i = 0; i < orderByItemsCount; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(orderByClause, i);
	}
}

// Adds a node to the referenced entities rax, in case it has labels or properties (inline filter).
void _AST_MapReferencedNode(AST *ast, const cypher_astnode_t *node) {
	uint nodeLablesCount = cypher_ast_node_pattern_nlabels(node);
	const cypher_astnode_t *properties = cypher_ast_node_pattern_get_properties(node);
	// If a node has more then one label, or has properties, it contains inline filter.
	if(nodeLablesCount > 0 || properties != NULL) {
		const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(node);
		const char *nodeName = cypher_ast_identifier_get_name(identifier);
		raxInsert(ast->referenced_entities, nodeName, strlen(nodeName), true, NULL);
	}
}

// Adds an edge to the referenced entities rax, in case it has type or properties (inline filter).
void _AST_MapReferencedEdge(AST *ast, const cypher_astnode_t *edge) {
	uint edgeTypesCount = cypher_ast_rel_pattern_nreltypes(edge);
	const cypher_astnode_t *properties = cypher_ast_rel_pattern_get_properties(edge);
	// If an edge has more then one type, or has properties, it contains inline filter.
	if(edgeTypesCount > 0 || properties != NULL) {
		const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(edge);
		const char *edgeName = cypher_ast_identifier_get_name(identifier);
		raxInsert(ast->referenced_entities, edgeName, strlen(edgeName), true, NULL);
	}
}

void _AST_MapReferencedEntitiesInPath(AST *ast, const cypher_astnode_t *path) {
	uint pathLen = cypher_ast_pattern_path_nelements(path);
	// Node are in even positions.
	for(uint i = 0; i < pathLen; i += 2)
		_AST_MapReferencedNode(ast, cypher_ast_pattern_path_get_element(path, i));
	// Edges are in odd poisitions.
	for(uint i = 1; i < pathLen; i += 2)
		_AST_MapReferencedEdge(ast, cypher_ast_pattern_path_get_element(path, i));
}

static void _AST_MapMatchClauseReferences(AST *ast, const cypher_astnode_t *matchClause) {
	// Inline filters.
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(matchClause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapReferencedEntitiesInPath(ast, path);
	}

	// Where clause.
	const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(matchClause);
	if(predicate) _AST_MapWhereClause(ast, predicate);
}

static void _AST_MapSetPropertyReferences(AST *ast, const cypher_astnode_t *set_item) {
	// Retrieve the alias being modified from the property descriptor.
	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);
	const cypher_astnode_t *ast_entity = cypher_ast_property_operator_get_expression(ast_prop);
	assert(cypher_astnode_type(ast_entity) == CYPHER_AST_IDENTIFIER);

	const char *alias = cypher_ast_identifier_get_name(ast_entity);
	raxInsert(ast->referenced_entities, (unsigned char *)alias, strlen(alias), true, NULL);

}

static void _AST_MapSetClauseReferences(AST *ast, const cypher_astnode_t *set_clause) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	for(uint i = 0; i < nitems; i++) {
		// Get the SET directive at this index.
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
		_AST_MapSetPropertyReferences(ast, set_item);
	}
}

static void _AST_MapMergeClauseReference(AST *ast, const cypher_astnode_t *merge_clause) {
	const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(merge_clause);
	_AST_MapReferencedEntitiesInPath(ast, merge_path);
	uint merge_actions = cypher_ast_merge_nactions(merge_clause);
	for(uint i = 0; i < merge_actions; i++) {
		const cypher_astnode_t *action = cypher_ast_merge_get_action(merge_clause, i);
		cypher_astnode_type_t type = cypher_astnode_type(action);
		if(type == CYPHER_AST_ON_CREATE) {
			uint on_create_items = cypher_ast_on_create_nitems(action);
			for(uint j = 0; j < on_create_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_create_get_item(action, j);
				assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		} else if(type == CYPHER_AST_ON_MATCH) {
			uint on_match_items = cypher_ast_on_match_nitems(action);
			for(uint j = 0; j < on_match_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_match_get_item(action, j);
				assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		}
	}
}

void _ASTClause_BuildReferenceMap(AST *ast, const cypher_astnode_t *clause) {
	if(!clause) return;
	cypher_astnode_type_t type = cypher_astnode_type(clause);
	if(type == CYPHER_AST_RETURN) {
		// Add referenced aliases for RETURN projections.
		uint projectionCount = cypher_ast_return_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionReference(ast, cypher_ast_return_get_projection(clause, i));
		}
		// Add referenced aliases for RETURN's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_return_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);
	} else if(type == CYPHER_AST_WITH) {
		// Add referenced aliases for WITH projections.
		uint projectionCount = cypher_ast_with_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionReference(ast, cypher_ast_with_get_projection(clause, i));
		}
		// Add referenced aliases for WITH's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);
	} else if(type == CYPHER_AST_UNWIND) {
		// Add referenced aliases for UNWIND clause.
		_AST_MapUnwindReferences(ast, clause);
	} else if(type == CYPHER_AST_MATCH) {
		// Add referenced aliases for MATCH clause - inline properties and explicit WHERE filter.
		_AST_MapMatchClauseReferences(ast, clause);
		uint numberOfChildren = cypher_astnode_nchildren(clause);
	} else if(type == CYPHER_AST_MERGE) {
		_AST_MapMergeClauseReference(ast, clause);
	} else if(type == CYPHER_AST_SET) {
		_AST_MapSetClauseReferences(ast, clause);
	}
}

void AST_BuildReferenceMap(AST *ast) {
	ast->referenced_entities = raxNew();
	// Check every clause in this AST.
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	if(!clause_count) return;
	uint i = 0;
	/* Since ast segments are range inclusive: The segment of the caluse indices [i,j]
	 * holds all the ast clauses from i to j. The next segment with the caluse indices [j, k]
	 * holds all the clauses from j to k, so j is an intersection clause of both segments.
	 * The AST segments are built with respect only to the WITH or RETURN caluses as intersection clauses.
	 * If both first and last clauses are either WITH or RETURN, skip the first clause
	 * since it is an intersection clause and handeld in the previous segment.*/
	if(clause_count > 1) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, 0);
		cypher_astnode_type_t firstType = cypher_astnode_type(clause);
		clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
		cypher_astnode_type_t lastType = cypher_astnode_type(clause);
		if((firstType == CYPHER_AST_RETURN || firstType == CYPHER_AST_WITH) &&
		   (lastType == CYPHER_AST_RETURN || lastType == CYPHER_AST_WITH)) i++;
	}
	for(; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		_ASTClause_BuildReferenceMap(ast, clause);
	}
}

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->free_root = true;

	uint n = end_offset - start_offset;

	cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_query_get_clause(master_ast->root, i + start_offset);
	}
	struct cypher_input_range range = {};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n, NULL, 0, range);

	// TODO This overwrites the previously-held AST pointer, which could lead to inconsistencies
	// in the future if we expect the variable to hold a different AST.
	QueryCtx_SetAST(ast);
	AST_BuildReferenceMap(ast);

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

void AST_Free(AST *ast) {
	if(ast == NULL) return;
	if(ast->referenced_entities) raxFree(ast->referenced_entities);
	if(ast->free_root) free((cypher_astnode_t *)ast->root);
	rm_free(ast);
}

