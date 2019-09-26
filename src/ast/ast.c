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

// Adds an identifier or an alias to the reference map.
static inline void _AST_UpdateRefMap(AST *ast, const char *name) {
	raxInsert(ast->referenced_entities, (unsigned char *)name, strlen(name), NULL, NULL);
}

// Map identifiers within an expression.
static void _AST_MapExpression(AST *ast, const cypher_astnode_t *exp) {
	cypher_astnode_type_t type = cypher_astnode_type(exp);

	// In case of identifier.
	if(type == CYPHER_AST_IDENTIFIER) {
		const char *identifier_name = cypher_ast_identifier_get_name(exp);
		_AST_UpdateRefMap(ast, identifier_name);
	} else if(type == CYPHER_AST_PROPERTY_OPERATOR) {
		// In case of property.
		exp = cypher_ast_property_operator_get_expression(exp);
		assert(cypher_astnode_type(exp) == CYPHER_AST_IDENTIFIER);
		const char *identifier_name = cypher_ast_identifier_get_name(exp);
		_AST_UpdateRefMap(ast, identifier_name);
	} else {
		// Recurse over children.
		uint child_count = cypher_astnode_nchildren(exp);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(exp, i);
			// Recursively continue mapping.
			_AST_MapExpression(ast, child);
		}
	}
}

// Maps the RHS of "AS" projected entities.
static inline void _AST_MapProjectionAlias(AST *ast, const cypher_astnode_t *projection) {
	const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(projection);
	if(ast_alias == NULL) {
		// The projection was not aliased, so the projection itself must be an identifier.
		ast_alias = cypher_ast_projection_get_expression(projection);
		assert(cypher_astnode_type(ast_alias) == CYPHER_AST_IDENTIFIER);
	}
	// WITH and RETURN projections are always either aliased or themselves identifiers.
	const char *alias = cypher_ast_identifier_get_name(ast_alias);
	_AST_UpdateRefMap(ast, alias);
}

// Adds referenced entities of ORDER BY clause.
static void _AST_MapOrderByReferences(AST *ast, const cypher_astnode_t *order_by) {
	uint count = cypher_ast_order_by_nitems(order_by);
	// Go over each order by expression.
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_by, i);
		const cypher_astnode_t *expression = cypher_ast_sort_item_get_expression(item);
		_AST_MapExpression(ast, expression);
	}
}

// Adds a node to the referenced entities rax, in case it has labels or properties (inline filter).
static void _AST_MapReferencedNode(AST *ast, const cypher_astnode_t *node) {
	const cypher_astnode_t *properties = cypher_ast_node_pattern_get_properties(node);
	// A node with inlined filters is always referenced for the FilterTree.
	// (In the case of a CREATE path, these are properties being set)
	if(properties) {
		const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(node);
		const char *alias = cypher_ast_identifier_get_name(identifier);
		_AST_UpdateRefMap(ast, alias);

		// Map any references within the properties map, such as 'b' in:
		// ({val: ID(b)})
		_AST_MapExpression(ast, properties);
	}
}

// Adds an edge to the referenced entities rax if it has multiple types or any properties (inline filter).
static void _AST_MapReferencedEdge(AST *ast, const cypher_astnode_t *edge) {
	uint edgeTypesCount = cypher_ast_rel_pattern_nreltypes(edge);
	const cypher_astnode_t *properties = cypher_ast_rel_pattern_get_properties(edge);
	// An edge with inlined filters is always referenced for the FilterTree.
	// (In the case of a CREATE path, these are properties being set)
	if(properties) {
		const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(edge);
		const char *alias = cypher_ast_identifier_get_name(identifier);
		_AST_UpdateRefMap(ast, alias);

		// Map any references within the properties map, such as 'b' in:
		// ({val: ID(b)})
		_AST_MapExpression(ast, properties);
	}
}

// Maps entities in a given path.
static void _AST_MapReferencedEntitiesInPath(AST *ast, const cypher_astnode_t *path) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Node are in even positions.
	for(uint i = 0; i < path_len; i += 2)
		_AST_MapReferencedNode(ast, cypher_ast_pattern_path_get_element(path, i));
	// Edges are in odd positions.
	for(uint i = 1; i < path_len; i += 2)
		_AST_MapReferencedEdge(ast, cypher_ast_pattern_path_get_element(path, i));
}

// Add referenced aliases from MATCH clause - inline filtered and explicit WHERE filter.
static void _AST_MapMatchClauseReferences(AST *ast, const cypher_astnode_t *match_clause) {
	// Inline filters.
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match_clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapReferencedEntitiesInPath(ast, path);
	}

	// Where clause.
	const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(match_clause);
	if(predicate) _AST_MapExpression(ast, predicate);
}

// Add referenced aliases from CREATE clause.
static void _AST_MapCreateClauseReferences(AST *ast, const cypher_astnode_t *create_clause) {
	const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(create_clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapReferencedEntitiesInPath(ast, path);
	}
}

// Maps entities in SET property clause.
static void _AST_MapSetPropertyReferences(AST *ast, const cypher_astnode_t *set_item) {
	// Retrieve the alias being modified from the property descriptor.
	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);
	const cypher_astnode_t *ast_entity = cypher_ast_property_operator_get_expression(ast_prop);
	assert(cypher_astnode_type(ast_entity) == CYPHER_AST_IDENTIFIER);

	const char *alias = cypher_ast_identifier_get_name(ast_entity);
	_AST_UpdateRefMap(ast, alias);
}

// Maps entities in SET clause.
static void _AST_MapSetClauseReferences(AST *ast, const cypher_astnode_t *set_clause) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	for(uint i = 0; i < nitems; i++) {
		// Get the SET directive at this index.
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
		_AST_MapSetPropertyReferences(ast, set_item);
	}
}

// Maps entities in DELETE clause.
static void _AST_MapDeleteClauseReferences(AST *ast, const cypher_astnode_t *delete_clause) {
	uint nitems = cypher_ast_delete_nexpressions(delete_clause);
	for(uint i = 0; i < nitems; i++) {
		const cypher_astnode_t *delete_exp = cypher_ast_delete_get_expression(delete_clause, i);

		assert(cypher_astnode_type(delete_exp) == CYPHER_AST_IDENTIFIER);

		const char *alias = cypher_ast_identifier_get_name(delete_exp);
		_AST_UpdateRefMap(ast, alias);
	}
}

// Maps entities in MERGE clause. Either by implicit filters, or modified entities by SET clause.
static void _AST_MapMergeClauseReference(AST *ast, const cypher_astnode_t *merge_clause) {
	// Collect implicitly filtered entities.
	const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(merge_clause);
	_AST_MapReferencedEntitiesInPath(ast, merge_path);

	// Map modified entities, either by ON MATCH or ON CREATE clause.
	uint merge_actions = cypher_ast_merge_nactions(merge_clause);
	for(uint i = 0; i < merge_actions; i++) {
		const cypher_astnode_t *action = cypher_ast_merge_get_action(merge_clause, i);
		cypher_astnode_type_t type = cypher_astnode_type(action);
		// ON CREATE.
		if(type == CYPHER_AST_ON_CREATE) {
			uint on_create_items = cypher_ast_on_create_nitems(action);
			for(uint j = 0; j < on_create_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_create_get_item(action, j);
				assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		} else if(type == CYPHER_AST_ON_MATCH) {
			// ON MATCH.
			uint on_match_items = cypher_ast_on_match_nitems(action);
			for(uint j = 0; j < on_match_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_match_get_item(action, j);
				assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		}
	}
}

// Map the LHS of "AS" projected entities. "WITH a as x order by x" will just collect a to the map.
static void _AST_MapWithReferredEntities(AST *ast_segment, const cypher_astnode_t *with_clause) {
	int projectionCount = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0 ; i < projectionCount; i ++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		// The expression forms the LHS of the projection.
		const cypher_astnode_t *exp = cypher_ast_projection_get_expression(projection);
		_AST_MapExpression(ast_segment, exp);
	}
	// Add referenced aliases for WITH's ORDER BY entities.
	const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(with_clause);
	if(order_by) _AST_MapOrderByReferences(ast_segment, order_by);
}

// Map the LHS of "AS" projected entities. "RETURN a as x order by x" will just collect a to the map.
static void _AST_MapReturnReferredEntities(AST *ast_segment,
										   const cypher_astnode_t *return_clause) {
// Add referenced aliases for RETURN projections.
	uint projectionCount = cypher_ast_return_nprojections(return_clause);
	for(uint i = 0 ; i < projectionCount; i ++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		// The expression forms the LHS of the projection.
		const cypher_astnode_t *exp = cypher_ast_projection_get_expression(projection);
		_AST_MapExpression(ast_segment, exp);
	}
	// Add referenced aliases for RETURN's ORDER BY entities.
	const cypher_astnode_t *order_by = cypher_ast_return_get_order_by(return_clause);
	if(order_by) _AST_MapOrderByReferences(ast_segment, order_by);
}

// Map the referred aliases (LHS) in entities projected by a WITH or RETURN clause.
static void _AST_MapReferredEntities(AST *ast_segment, const cypher_astnode_t *project_clause) {
	if(!project_clause) return;
	cypher_astnode_type_t type = cypher_astnode_type(project_clause);
	assert(type == CYPHER_AST_WITH || type == CYPHER_AST_RETURN);

	if(type == CYPHER_AST_WITH) _AST_MapWithReferredEntities(ast_segment, project_clause);
	else _AST_MapReturnReferredEntities(ast_segment, project_clause);
}

static void _ASTClause_BuildReferenceMap(AST *ast, const cypher_astnode_t *clause) {
	if(!clause) return;

	cypher_astnode_type_t type = cypher_astnode_type(clause);
	if(type == CYPHER_AST_RETURN) {
		// Add referenced aliases for RETURN projections.
		uint projectionCount = cypher_ast_return_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionAlias(ast, cypher_ast_return_get_projection(clause, i));
		}
		// Add referenced aliases for RETURN's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_return_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);

	} else if(type == CYPHER_AST_WITH) {
		// Add referenced aliases for WITH projections.
		uint projectionCount = cypher_ast_with_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionAlias(ast, cypher_ast_with_get_projection(clause, i));
		}
		// Add referenced aliases for WITH's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);

	} else if(type == CYPHER_AST_MATCH) {
		// Add referenced aliases from MATCH clause - inline filtered and explicit WHERE filter.
		_AST_MapMatchClauseReferences(ast, clause);

	} else if(type == CYPHER_AST_CREATE) {
		// Add referenced aliases for DELETE clause.
		_AST_MapCreateClauseReferences(ast, clause);

	} else if(type == CYPHER_AST_MERGE) {
		// Add referenced aliases for MERGE clause - inline filtered and modified entities.
		_AST_MapMergeClauseReference(ast, clause);

	} else if(type == CYPHER_AST_SET) {
		// Add referenced aliases for SET clause.
		_AST_MapSetClauseReferences(ast, clause);
	} else if(type == CYPHER_AST_DELETE) {
		// Add referenced aliases for DELETE clause.
		_AST_MapDeleteClauseReferences(ast, clause);
	}
}

// Adds every reference entity in each clause.
static void _AST_BuildReferenceMap(AST *ast) {
	ast->referenced_entities = raxNew();
	// Check every clause in this AST.
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	if(!clause_count) return;
	for(uint i = 0; i < clause_count; i++) {
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
	_AST_BuildReferenceMap(ast);

	// If the segments are split, the next clause is either return or with clause.
	uint clause_count = cypher_ast_query_nclauses(master_ast->root);
	if(clause_count > 1 && end_offset < clause_count) {
		_AST_MapReferredEntities(ast, cypher_ast_query_get_clause(master_ast->root, end_offset));
	}

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

