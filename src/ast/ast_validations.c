/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../RG.h"
#include "../errors.h"
#include "ast_shared.h"
#include "../util/arr.h"
#include "cypher_whitelist.h"
#include "../util/rax_extensions.h"
#include "../procedures/procedure.h"
#include "../arithmetic/arithmetic_expression.h"

// TODO: generic function should be used to validate different features positions
// static AST_Validation _NestedIn
// (
// 	const cypher_astnode_t *root,
// 	cypher_astnode_type_t search_type,
// 	cypher_astnode_type_t *whitelist,
// 	bool is_in_whitelist
// ) {
// 	ASSERT(root != NULL);
// 	ASSERT(whitelist != NULL);

// 	cypher_astnode_type_t t = cypher_astnode_type(root);
// 	if(t == search_type && is_in_whitelist) return true;

// 	if(!is_in_whitelist) {
// 		int len = array_len(whitelist);
// 		for (uint i = 0; i < len; i++) {
// 			if(t == whitelist[i]) {
// 				is_in_whitelist = true;
// 				break;
// 			}
// 		}
// 	}

// 	uint nchildren = cypher_astnode_nchildren(root);
// 	for(uint i = 0; i < nchildren; i ++) {
// 		const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
// 		bool res = _NestedIn(child, search_type, whitelist, is_in_whitelist);
// 		if(res) return true;
// 	}

// 	return false;
// }

// validate that allShortestPaths is in a supported places
static bool _ValidateAllShortestPaths
(
	const cypher_astnode_t *root
) {
	ASSERT(root != NULL);

	cypher_astnode_type_t t = cypher_astnode_type(root);
	// if we found allShortestPaths in invalid parent return true
	if(t == CYPHER_AST_SHORTEST_PATH &&
	   !cypher_ast_shortest_path_is_single(root)) {
		return true;
	}

	if(t == CYPHER_AST_MATCH) {
		// allShortesPath is invalid in the match predicate
		const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(root);
		return predicate != NULL && _ValidateAllShortestPaths(predicate);
	}

	// recursively traverse all children
	uint nchildren = cypher_astnode_nchildren(root);
	for(uint i = 0; i < nchildren; i ++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
		bool res = _ValidateAllShortestPaths(child);
		if(res) return true;
	}

	return false;
}

// Forward declaration
static void _AST_Path_GetDefinedIdentifiers(const cypher_astnode_t *path, rax *identifiers);
static void _AST_GetDefinedIdentifiers(const cypher_astnode_t *node, rax *identifiers);

inline static void _prepareIterateAll(rax *map, raxIterator *iter) {
	raxStart(iter, map);
	raxSeek(iter, "^", NULL, 0);
}

static void _AST_GetIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
	if(!node) return;
	ASSERT(identifiers != NULL);

	cypher_astnode_type_t type = cypher_astnode_type(node);
	if(type == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(node);
		raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
		return;
	}

	/* In case current node is of type CALL
	 * Process procedure call arguments, those should be defined prior
	 * to the procedure call.
	 * Outputs are not refering to previous identifiers */
	if(type == CYPHER_AST_CALL) {
		uint arg_count = cypher_ast_call_narguments(node);
		for(uint i = 0; i < arg_count; i++) {
			const cypher_astnode_t *arg = cypher_ast_call_get_argument(node, i);
			_AST_GetIdentifiers(arg, identifiers);
		}
		return;
	}

	// Don't visit the children of pattern comprehensions, as these will be aliased later.
	if(type == CYPHER_AST_PATTERN_COMPREHENSION) return;

	uint child_count = cypher_astnode_nchildren(node);

	/* In case current node is of type projection
	 * inspect first child only,
	 * @10  20..26  > > > projection           expression=@11, alias=@14
	 * @11  20..26  > > > > apply              @12(@13)
	 * @12  20..23  > > > > > function name    `max`
	 * @13  24..25  > > > > > identifier       `z`
	 * @14  20..26  > > > > identifier         `max(z)` */
	if(type == CYPHER_AST_PROJECTION) child_count = 1;

	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		_AST_GetIdentifiers(child, identifiers);
	}

	if(type == CYPHER_AST_LIST_COMPREHENSION ||
	   type == CYPHER_AST_ANY ||
	   type == CYPHER_AST_ALL ||
	   type == CYPHER_AST_SINGLE ||
	   type == CYPHER_AST_NONE) {
		// A list comprehension has a local variable that should only be accessed within its scope;
		// do not leave it in the identifiers map.
		const cypher_astnode_t *variable_node = cypher_ast_list_comprehension_get_identifier(node);
		const char *variable = cypher_ast_identifier_get_name(variable_node);
		raxRemove(identifiers, (unsigned char *)variable, strlen(variable), NULL);
	}

	if(type == CYPHER_AST_REDUCE) {
		// A reduce call has an accumulator and a local variable that should
		// only be accessed within its scope;
		// do not leave them in the identifiers map
		// example: reduce(sum=0, n in [1,2] | sum+n)
		const  char              *variable         =  NULL;
		const  cypher_astnode_t  *accum_node       =  NULL;
		const  cypher_astnode_t  *identifier_node  =  NULL;

		// `sum` in the above example
		accum_node = cypher_ast_reduce_get_accumulator(node);
		variable = cypher_ast_identifier_get_name(accum_node);
		raxRemove(identifiers, (unsigned char *)variable, strlen(variable),
				  NULL);

		// `n` in the above example
		identifier_node = cypher_ast_reduce_get_identifier(node);
		variable = cypher_ast_identifier_get_name(identifier_node);
		raxRemove(identifiers, (unsigned char *)variable, strlen(variable),
				  NULL);
	}
}

static void _AST_GetWithAliases(const cypher_astnode_t *node, rax *aliases) {
	if(!node) return;
	if(cypher_astnode_type(node) != CYPHER_AST_WITH) return;
	ASSERT(aliases != NULL);

	uint num_with_projections = cypher_ast_with_nprojections(node);
	for(uint i = 0; i < num_with_projections; i ++) {
		const cypher_astnode_t *child = cypher_ast_with_get_projection(node, i);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(child);
		const char *alias;
		if(alias_node) {
			alias = cypher_ast_identifier_get_name(alias_node);
		} else {
			const cypher_astnode_t *expr = cypher_ast_projection_get_expression(child);
			// This expression not being an identifier is an error case, but will be captured in a later validation.
			if(cypher_astnode_type(expr) != CYPHER_AST_IDENTIFIER) continue;
			// Retrieve "a" from "WITH a"
			alias = cypher_ast_identifier_get_name(expr);
		}
		raxInsert(aliases, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}
}

static void _AST_GetWithReferences(const cypher_astnode_t *node, rax *identifiers) {
	if(!node) return;
	if(cypher_astnode_type(node) != CYPHER_AST_WITH) return;
	ASSERT(identifiers != NULL);

	uint num_with_projections = cypher_ast_with_nprojections(node);
	for(uint i = 0; i < num_with_projections; i ++) {
		const cypher_astnode_t *child = cypher_ast_with_get_projection(node, i);
		_AST_GetIdentifiers(child, identifiers);
	}

	const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(node);
	if(order_by) _AST_GetIdentifiers(order_by, identifiers);
}

// Extract identifiers / aliases from a procedure call.
static void _AST_GetProcCallAliases(const cypher_astnode_t *node, rax *identifiers) {
	// CALL db.labels() yield label
	// CALL db.labels() yield label as l
	ASSERT(node && identifiers);
	ASSERT(cypher_astnode_type(node) == CYPHER_AST_CALL);

	uint projection_count = cypher_ast_call_nprojections(node);
	for(uint i = 0; i < projection_count; i++) {
		const char *identifier = NULL;
		const cypher_astnode_t *proj_node = cypher_ast_call_get_projection(node, i);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(proj_node);
		if(alias_node) {
			// Alias is given: YIELD label AS l.
			identifier = cypher_ast_identifier_get_name(alias_node);
		} else {
			// No alias, use identifier: YIELD label
			const cypher_astnode_t *exp_node = cypher_ast_projection_get_expression(proj_node);
			identifier = cypher_ast_identifier_get_name(exp_node);
		}
		ASSERT(identifiers != NULL);
		raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
	}
}

// UNWIND and WITH also form aliases, but don't need special handling for us yet.
static void _AST_GetReturnAliases(const cypher_astnode_t *node, rax *aliases) {
	ASSERT(node && aliases && cypher_astnode_type(node) == CYPHER_AST_RETURN);

	uint num_return_projections = cypher_ast_return_nprojections(node);
	if(num_return_projections == 0) return;

	for(uint i = 0; i < num_return_projections; i ++) {
		const cypher_astnode_t *child = cypher_ast_return_get_projection(node, i);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(child);
		if(alias_node == NULL) continue;
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		raxInsert(aliases, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}
}

static void _CollectIdentifiers(const cypher_astnode_t *root, rax *projections) {
	if(cypher_astnode_type(root) == CYPHER_AST_IDENTIFIER) {
		// Identifier found, add to triemap
		const char *identifier = cypher_ast_identifier_get_name(root);
		raxInsert(projections, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
	} else {
		uint child_count = cypher_astnode_nchildren(root);
		for(uint i = 0; i < child_count; i ++) {
			_CollectIdentifiers(cypher_astnode_get_child(root, i), projections);
		}
	}
}

/* Collect all identifiers used in the RETURN clause (but not aliases defined there) */
static rax *_AST_GetReturnProjections(const cypher_astnode_t *return_clause) {
	if(!return_clause) return NULL;

	uint projection_count = cypher_ast_return_nprojections(return_clause);
	if(projection_count == 0) return NULL;

	rax *projections = raxNew();
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		_CollectIdentifiers(projection, projections);
	}

	return projections;
}

/* Compares a triemap of user-specified functions with the registered functions we provide. */
static AST_Validation _ValidateReferredFunctions(rax *referred_functions, bool include_aggregates) {
	AST_Validation res = AST_VALID;
	char funcName[32];
	raxIterator it;
	_prepareIterateAll(referred_functions, &it);
	bool found = true;
	while(raxNext(&it)) {
		size_t len = it.key_len;
		// No functions have a name longer than 32 characters
		if(len >= 32) {
			res = AST_INVALID;
			break;
		}

		// Copy the triemap key so that we can safely add a terinator character
		memcpy(funcName, it.key, len);
		funcName[len] = 0;

		if(!include_aggregates && AR_FuncIsAggregate(funcName)) {
			// Provide a unique error for using aggregate functions from inappropriate contexts
			ErrorCtx_SetError("Invalid use of aggregating function '%s'", funcName);
			res = AST_INVALID;
			break;
		}

		if(AR_FuncExists(funcName)) continue;

		// If we reach this point, the function was not found
		found = false;
		res = AST_INVALID;
		break;
	}

	// If the function was not found, provide a reason if one is not set
	if(res == AST_INVALID && !found) {
		ErrorCtx_SetError("Unknown function '%s'", funcName);
	}

	raxStop(&it);
	return res;
}

// validate that a map doesn't contains a nested aggregation function
// e.g. {key: count(v)}
static AST_Validation _ValidateMapExp(const cypher_astnode_t *node) {
	ASSERT(cypher_astnode_type(node) == CYPHER_AST_MAP);

	if(AST_ClauseContainsAggregation(node)) {
		ErrorCtx_SetError("RedisGraph does not allow aggregate function calls \
to be nested within maps. Aggregate functions should instead be called in a \
preceding WITH clause and have their aliased values referenced in the map.");
		return AST_INVALID;
	}

	return AST_VALID;
}

// Recursively collect function names and perform validations on functions with STAR arguments.
static AST_Validation _VisitFunctions(const cypher_astnode_t *node, rax *func_names) {
	cypher_astnode_type_t type = cypher_astnode_type(node);
	if(type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		// Working with a function call that has * as its argument.
		const cypher_astnode_t *func = cypher_ast_apply_all_operator_get_func_name(node);
		const char *func_name = cypher_ast_function_name_get_value(func);

		// Verify that this is a COUNT call.
		if(strcasecmp(func_name, "COUNT")) {
			ErrorCtx_SetError("COUNT is the only function which can accept * as an argument");
			return AST_INVALID;
		}

		// Verify that DISTINCT is not specified.
		if(cypher_ast_apply_all_operator_get_distinct(node)) {
			// TODO consider opening a parser error, this construction is invalid in Neo's parser.
			ErrorCtx_SetError("Cannot specify both DISTINCT and * in COUNT(DISTINCT *)");
			return AST_INVALID;
		}

		// Collect the function name, which is always "count" here.
		raxInsert(func_names, (unsigned char *)"count", 5, NULL, NULL);

		// As Apply All operators have no children, we can return here.
		return AST_VALID;
	}

	if(type == CYPHER_AST_APPLY_OPERATOR) {
		// Collect the function name.
		const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(node);
		const char *func_name = cypher_ast_function_name_get_value(func);
		raxInsert(func_names, (unsigned char *)func_name, strlen(func_name), NULL, NULL);
	}

	if(type == CYPHER_AST_MAP) {
		// validate map expression
		AST_Validation res = _ValidateMapExp(node);
		if(res != AST_VALID) return res;
	}

	uint child_count = cypher_astnode_nchildren(node);
	for(uint i = 0; i < child_count; i ++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		AST_Validation res = _VisitFunctions(child, func_names);
		if(res != AST_VALID) return res;
	}

	return AST_VALID;
}

static AST_Validation _ValidateFunctionCalls(const cypher_astnode_t *node,
											 bool include_aggregates) {
	AST_Validation res = AST_VALID;
	rax *func_names = raxNew();

	// Collect function names and perform in-place validations.
	res = _VisitFunctions(node, func_names);
	if(res != AST_VALID) goto cleanup;

	// Validate all provided function names.
	res = _ValidateReferredFunctions(func_names, include_aggregates);

cleanup:
	raxFree(func_names);
	return res;
}

static inline bool _AliasIsReturned(rax *projections, const char *identifier) {
	return raxFind(projections, (unsigned char *)identifier, strlen(identifier)) != raxNotFound;
}

// If we have a multi-hop traversal (fixed or variable length), we cannot currently return that entity.
static AST_Validation _ValidateMultiHopTraversal(rax *projections, const cypher_astnode_t *edge,
												 const cypher_astnode_t *range) {
	int start = 1;
	int end = INT_MAX - 2;

	const cypher_astnode_t *range_start = cypher_ast_range_get_start(range);
	if(range_start) start = AST_ParseIntegerNode(range_start);

	const cypher_astnode_t *range_end = cypher_ast_range_get_end(range);
	if(range_end) end = AST_ParseIntegerNode(range_end);

	// Validate specified range
	if(start > end) {
		ErrorCtx_SetError("Variable length path, maximum number of hops must be greater or equal to minimum number of hops.");
		return AST_INVALID;
	}

	bool multihop = (start > 1) || (start != end);
	if(!multihop) return AST_VALID;

	// Multi-hop traversals cannot be referenced
	if(!projections) return AST_VALID;

	// Check if relation has an alias
	const cypher_astnode_t *ast_identifier = cypher_ast_rel_pattern_get_identifier(edge);
	if(!ast_identifier) return AST_VALID;

	// Verify that the alias is not found in the RETURN clause.
	const char *identifier = cypher_ast_identifier_get_name(ast_identifier);
	if(_AliasIsReturned(projections, identifier)) {
		ErrorCtx_SetError("RedisGraph does not support the return of variable-length traversal edges '%s'. \
        Instead, use a query in the style of: 'MATCH p = (a)-[%s*]->(b) RETURN relationships(p)'.",
						  identifier, identifier);
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_ReusedEdges(const cypher_astnode_t *node, rax *edge_aliases) {
	uint child_count = cypher_astnode_nchildren(node);
	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		cypher_astnode_type_t type = cypher_astnode_type(child);

		if(type == CYPHER_AST_IDENTIFIER) {
			const char *alias = cypher_ast_identifier_get_name(child);
			int new = raxInsert(edge_aliases, (unsigned char *)alias, strlen(alias), NULL,
								NULL);
			if(!new) {
				ErrorCtx_SetError("Cannot use the same relationship variable '%s' for multiple patterns.", alias);
				return AST_INVALID;
			}
		}
	}

	return AST_VALID;
}

static AST_Validation _ValidateRelation(rax *projections, const cypher_astnode_t *edge,
										rax *edge_aliases) {
	AST_Validation res = AST_VALID;

	// Make sure edge alias is unique
	res = _Validate_ReusedEdges(edge, edge_aliases);
	if(res != AST_VALID) return res;

	// If this is a multi-hop traversal, validate it
	const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(edge);
	if(range) {
		res = _ValidateMultiHopTraversal(projections, edge, range);
		if(res != AST_VALID) return res;
	}

	return res;
}

static AST_Validation _ValidatePath
(
	const cypher_astnode_t *path,
	rax *projections,
	rax *edge_aliases
) {
	AST_Validation res = AST_VALID;

	if(cypher_astnode_type(path) == CYPHER_AST_NAMED_PATH) {
		path = cypher_ast_named_path_get_path(path);
	}

	if(cypher_astnode_type(path) == CYPHER_AST_SHORTEST_PATH) {
		if(cypher_ast_shortest_path_is_single(path)) {
			// MATCH (a), (b), p = shortestPath((a)-[*]->(b)) RETURN p
			res = AST_INVALID;
			ErrorCtx_SetError("RedisGraph currently only supports shortestPath in WITH or RETURN clauses");
		} else {
			// MATCH (a), (b), p = allShortestPaths((a)-[*2..]->(b)) RETURN p
			// validate rel pattern range doesn't contains a minimum > 1
			const cypher_astnode_t **ranges =
				AST_GetTypedNodes(path, CYPHER_AST_RANGE);
			int range_count = array_len(ranges);
			for(int i = 0; i < range_count; i++) {
				long min_hops = 1;
				const cypher_astnode_t *r = ranges[i];
				const cypher_astnode_t *start = cypher_ast_range_get_start(r);
				if(start) min_hops = AST_ParseIntegerNode(start);
				if(min_hops != 1) {
					res = AST_INVALID;
					ErrorCtx_SetError("allShortestPaths(...) does not support a minimal length different from 1");
					break;
				}
			}
			array_free(ranges);
		}
	}

	if(res == AST_VALID) {
		uint path_len = cypher_ast_pattern_path_nelements(path);

		// validate relations on the path (every odd offset) and collect aliases
		for(uint i = 1; i < path_len; i += 2) {
			const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
			res = _ValidateRelation(projections, edge, edge_aliases);
			if(res != AST_VALID) break;
		}
	}

	return res;
}

static AST_Validation _ValidatePattern
(
	rax *projections,
	const cypher_astnode_t *pattern,
	rax *edge_aliases
) {
	AST_Validation  res         =  AST_VALID;
	uint            path_count  =  cypher_ast_pattern_npaths(pattern);

	for(uint i = 0; i < path_count; i ++) {
		res = _ValidatePath(cypher_ast_pattern_get_path(pattern, i),
							projections, edge_aliases);
		if(res != AST_VALID) break;
	}

	return res;
}

// Validate the property maps used in node/edge patterns in MATCH, and CREATE clauses
static AST_Validation _ValidateInlinedProperties(const cypher_astnode_t *props) {
	if(cypher_astnode_type(props) != CYPHER_AST_MAP) {
		// Emit an error if the properties are not presented as a map, as in:
		// MATCH (p {invalid_property_construction}) RETURN p
		ErrorCtx_SetError("Encountered unhandled type in inlined properties.");
		return AST_INVALID;
	}

	uint prop_count = cypher_ast_map_nentries(props);
	for(uint i = 0; i < prop_count; i++) {
		const cypher_astnode_t *prop_val = cypher_ast_map_get_value(props, i);
		const cypher_astnode_t **patterns = AST_GetTypedNodes(prop_val, CYPHER_AST_PATTERN_PATH);
		uint patterns_count = array_len(patterns);
		array_free(patterns);
		if(patterns_count > 0) {
			// Encountered query of the form:
			// MATCH (a {prop: ()-[]->()}) RETURN a
			ErrorCtx_SetError("Encountered unhandled type in inlined properties.");
			return AST_INVALID;
		}
	}

	return AST_VALID;
}

static AST_Validation _ValidateInlinedPropertiesOnPath(const cypher_astnode_t *path) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Check all nodes on path
	for(uint i = 0; i < path_len; i += 2) {
		const cypher_astnode_t *ast_identifier = NULL;
		const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *props = cypher_ast_node_pattern_get_properties(node);
		if(props && (_ValidateInlinedProperties(props) != AST_VALID)) return AST_INVALID;
	}

	// Check all edges on path
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *ast_identifier = NULL;
		const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(edge);
		if(props && (_ValidateInlinedProperties(props) != AST_VALID)) return AST_INVALID;
	}
	return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_Filters(const cypher_astnode_t *clause) {
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		if(_ValidateInlinedPropertiesOnPath(path) != AST_VALID) return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_CALL_Clauses(const AST *ast) {
	/* Make sure procedure calls are valid:
	 * 1. procedure exists
	 * 2. number of arguments to procedure is as expected
	 * 3. yield refers to procedure output */
	const cypher_astnode_t **call_clauses = AST_GetClauses(ast, CYPHER_AST_CALL);
	if(call_clauses == NULL) return AST_VALID;


	ProcedureCtx    *proc         =  NULL;
	rax             *identifiers  =  NULL;
	AST_Validation  res           =  AST_VALID;

	uint call_count = array_len(call_clauses);
	for(uint i = 0; i < call_count; i ++) {
		identifiers = raxNew();
		const cypher_astnode_t *call_clause = call_clauses[i];

		// Make sure procedure exists.
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		proc = Proc_Get(proc_name);

		if(proc == NULL) {
			ErrorCtx_SetError("Procedure `%s` is not registered", proc_name);
			res = AST_INVALID;
			goto cleanup;
		}

		// Validate num of arguments.
		if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) {
			unsigned int given_arg_count = cypher_ast_call_narguments(call_clause);
			if(Procedure_Argc(proc) != given_arg_count) {
				ErrorCtx_SetError("Procedure `%s` requires %d arguments, got %d", proc_name, proc->argc,
								  given_arg_count);
				res = AST_INVALID;
				goto cleanup;
			}
		}

		// validate projections
		uint proj_count = cypher_ast_call_nprojections(call_clause);
		if(proj_count > 0) {
			// collect call projections
			for(uint j = 0; j < proj_count; j++) {
				const cypher_astnode_t *proj = cypher_ast_call_get_projection(call_clause, j);
				const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(proj);
				ASSERT(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
				const char *identifier = cypher_ast_identifier_get_name(ast_exp);

				// make sure each yield output is mentioned only once
				if(!raxInsert(identifiers, (unsigned char *)identifier,
							  strlen(identifier), NULL, NULL)) {
					ErrorCtx_SetError("Variable `%s` already declared", identifier);
					res = AST_INVALID;
					goto cleanup;
				}

				// make sure procedure is aware of output
				if(!Procedure_ContainsOutput(proc, identifier)) {
					ErrorCtx_SetError("Procedure `%s` does not yield output `%s`",
									  proc_name, identifier);
					res = AST_INVALID;
					goto cleanup;
				}
			}

			raxFree(identifiers);
			identifiers = NULL;
		}

		Proc_Free(proc);
		proc = NULL;
	}

cleanup:
	if(proc) Proc_Free(proc);
	if(identifiers) raxFree(identifiers);
	array_free(call_clauses);
	return res;
}

static AST_Validation _ValidateNodeAlias(const cypher_astnode_t *node, rax *edge_aliases) {
	const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(node);
	if(ast_alias == NULL) return AST_VALID;

	// Verify that the node's alias is not in the map of edge aliases.
	const char *alias = cypher_ast_identifier_get_name(ast_alias);
	if(raxFind(edge_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
		ErrorCtx_SetError("The alias '%s' was specified for both a node and a relationship.", alias);
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _ValidateReusedAliases(rax *edge_aliases, const cypher_astnode_t *pattern) {
	AST_Validation res = AST_VALID;
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		uint path_len = cypher_ast_pattern_path_nelements(path);
		for(uint j = 0; j < path_len; j += 2) {
			const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, j);
			res = _ValidateNodeAlias(node, edge_aliases);
			if(res != AST_VALID) return res;
		}
	}

	return res;
}

static AST_Validation _Validate_MATCH_Clauses(const AST *ast) {
	// Check to see if all mentioned inlined, outlined functions exists.
	// Inlined functions appear within entity definition ({a:v})
	// Outlined functions appear within the WHERE clause.
	// libcypher-parser doesn't have a WHERE node, all of the filters
	// are specified within the MATCH node sub-tree.
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);

	rax             *edge_aliases     =  raxNew();
	rax             *reused_entities  =  raxNew();
	AST_Validation  res               =  AST_VALID;

	const cypher_astnode_t *return_clause =
		AST_GetClause(ast, CYPHER_AST_RETURN, NULL);
	rax *projections = _AST_GetReturnProjections(return_clause);
	uint match_count = array_len(match_clauses);
	for(uint i = 0; i < match_count; i ++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		// Validate the pattern described by the MATCH clause
		res = _ValidatePattern(projections,
							   cypher_ast_match_get_pattern(match_clause), edge_aliases);
		if(res == AST_INVALID) goto cleanup;

		// Validate that inlined filters do not use parameters
		res = _Validate_MATCH_Clause_Filters(match_clause);
		if(res == AST_INVALID) goto cleanup;

		// Validate all function references in clause. Aggregate calls cannot be made in MATCH
		// clauses or their WHERE predicates.
		bool include_aggregates = false;
		res = _ValidateFunctionCalls(match_clause, include_aggregates);
		if(res == AST_INVALID) goto cleanup;
	}

	// Verify that no relation alias is also used to denote a node
	for(uint i = 0; i < match_count; i ++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		res = _ValidateReusedAliases(edge_aliases, cypher_ast_match_get_pattern(match_clause));
		if(res == AST_INVALID) goto cleanup;
	}

cleanup:
	if(projections) raxFree(projections);
	raxFree(edge_aliases);
	raxFree(reused_entities);
	array_free(match_clauses);

	return res;
}

static AST_Validation _Validate_WITH_Clauses(const AST *ast) {
	// Verify that all functions used in the WITH clause and (if present) its WHERE predicate
	// are defined and used validly.
	// An AST segment has at most 1 WITH clause.
	const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH,
														NULL);
	if(with_clause == NULL) return AST_VALID;

	rax *rax = raxNew();
	AST_Validation res = AST_VALID;

	// Verify that each WITH projection either is aliased or is itself an identifier.
	uint projection_count = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *proj = cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(proj);
		if(!ast_alias &&
		   cypher_astnode_type(cypher_ast_projection_get_expression(proj)) != CYPHER_AST_IDENTIFIER) {
			ErrorCtx_SetError("WITH clause projections must be aliased");
			res = AST_INVALID;
			break;
		}
		if(ast_alias == NULL) ast_alias = cypher_ast_projection_get_expression(proj);
		const char *alias = cypher_ast_identifier_get_name(ast_alias);
		// column with same name is invalid
		if(raxTryInsert(rax, (unsigned char *)alias, strlen(alias), NULL, NULL) == 0) {
			ErrorCtx_SetError("Error: Multiple result columns with the same name are not supported.");
			res = AST_INVALID;
			break;
		}
	}

	raxFree(rax);

	if(res == AST_VALID) {
		// Verify that functions invoked by the WITH clause are valid.
		res = _ValidateFunctionCalls(with_clause, true);
	}

	return res;
}

// Verify that MERGE doesn't redeclare bound relations and that one reltype is specified for unbound relations.
static AST_Validation _ValidateMergeRelation(const cypher_astnode_t *entity, rax *defined_aliases) {
	const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(entity);
	const char *alias = NULL;
	if(identifier) {
		alias = cypher_ast_identifier_get_name(identifier);
		// Verify that we're not redeclaring a bound variable.
		if(raxFind(defined_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
			ErrorCtx_SetError("The bound variable %s' can't be redeclared in a MERGE clause", alias);
			return AST_INVALID;
		}
	}

	// Exactly one reltype should be specified for the introduced edge.
	uint reltype_count = cypher_ast_rel_pattern_nreltypes(entity);
	if(reltype_count != 1) {
		ErrorCtx_SetError("Exactly one relationship type must be specified for each relation in a MERGE pattern.");
		return AST_INVALID;
	}

	// We don't need to validate the MERGE edge's direction, as an undirected edge in MERGE
	// should cause a single outgoing edge to be created.

	return AST_VALID;
}

// Verify that MERGE doesn't redeclare bound nodes.
static AST_Validation _ValidateMergeNode(const cypher_astnode_t *entity, rax *defined_aliases) {
	if(raxSize(defined_aliases) == 0) return AST_VALID;

	const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(entity);
	if(identifier == NULL) return AST_VALID;

	const char *alias = cypher_ast_identifier_get_name(identifier);
	if(raxFind(defined_aliases, (unsigned char *)alias, strlen(alias)) == raxNotFound) {
		// If the entity is unaliased or not previously bound, it cannot be redeclared.
		return AST_VALID;
	}

	// If the entity is already bound, the MERGE pattern should not introduce labels or properties.
	if((cypher_ast_node_pattern_nlabels(entity) > 0) ||
	   cypher_ast_node_pattern_get_properties(entity)) {
		ErrorCtx_SetError("The bound node '%s' can't be redeclared in a MERGE clause", alias);
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_MERGE_Clauses(const AST *ast) {
	AST_Validation res = AST_VALID;
	uint *merge_clause_indices = AST_GetClauseIndices(ast, CYPHER_AST_MERGE);
	uint merge_count = array_len(merge_clause_indices);
	rax *defined_aliases = NULL;
	if(merge_count == 0) goto cleanup;

	defined_aliases = raxNew();
	uint start_offset = 0;
	for(uint i = 0; i < merge_count; i ++) {
		uint clause_idx = merge_clause_indices[i];

		// Collect all entities that are bound before this MERGE clause.
		for(uint j = start_offset; j < clause_idx; j ++) {
			const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, j);
			_AST_GetDefinedIdentifiers(clause, defined_aliases);
		}
		start_offset = clause_idx;

		const cypher_astnode_t *merge_clause = cypher_ast_query_get_clause(ast->root, clause_idx);
		const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);

		// Verify that functions invoked in the MERGE pattern are valid.
		res = _ValidateFunctionCalls(path, false);
		if(res != AST_VALID) goto cleanup;

		uint nelems = cypher_ast_pattern_path_nelements(path);
		for(uint j = 0; j < nelems; j ++) {
			const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, j);
			// Odd offsets correspond to edges, even offsets correspond to nodes.
			res = (j % 2) ? _ValidateMergeRelation(entity, defined_aliases)
				  : _ValidateMergeNode(entity, defined_aliases);
			if(res != AST_VALID) goto cleanup;
		}

		// Verify that any filters on the path refer to constants or resolved identifiers.
		res = _ValidateInlinedPropertiesOnPath(path);
		if(res != AST_VALID) goto cleanup;

		uint action_count = cypher_ast_merge_nactions(merge_clause);
		for (uint j = 0; j < action_count; j++) {
			const cypher_astnode_t *action = cypher_ast_merge_get_action(merge_clause, j);
			_ValidateFunctionCalls(action, false);
		}
	}

cleanup:
	array_free(merge_clause_indices);
	if(defined_aliases) raxFree(defined_aliases);
	return res;
}

// Validate each entity referenced in a single path of a CREATE clause.
static AST_Validation _Validate_CREATE_Entities(const cypher_astnode_t *path,
												rax *defined_aliases) {
	if(_ValidateInlinedPropertiesOnPath(path) != AST_VALID) return AST_INVALID;

	uint nelems = cypher_ast_pattern_path_nelements(path);
	 // Redeclaration of a node is not allowed only when the path is of length 0, as in: MATCH (a) CREATE (a).
	 // Otherwise, using a defined alias of a node is allowed, as in: MATCH (a) CREATE (a)-[:E]->(:B)
	if(nelems == 1) {
		const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, 0);
		const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(node);
		if(identifier) {
			const char *alias = cypher_ast_identifier_get_name(identifier);
			if(raxFind(defined_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
				ErrorCtx_SetError("The bound variable '%s' can't be redeclared in a CREATE clause", alias);
				return AST_INVALID;
			}
		}
	}
	//Visit every relationship (every odd offset) on the path to validate its alias and structure.
	for(uint j = 1; j < nelems; j += 2) {
		const cypher_astnode_t *rel = cypher_ast_pattern_path_get_element(path, j);
		const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(rel);
		// Validate that no relation aliases are previously bound.
		if(identifier) {
			const char *alias = cypher_ast_identifier_get_name(identifier);
			if(raxFind(defined_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
				ErrorCtx_SetError("The bound variable '%s' can't be redeclared in a CREATE clause", alias);
				return AST_INVALID;
			}
		}

		// Validate that each relation has exactly one type.
		uint reltype_count = cypher_ast_rel_pattern_nreltypes(rel);
		if(reltype_count != 1) {
			ErrorCtx_SetError("Exactly one relationship type must be specified for CREATE");
			return AST_INVALID;
		}

		// Validate that each relation being created is directed.
		if(cypher_ast_rel_pattern_get_direction(rel) == CYPHER_REL_BIDIRECTIONAL) {
			ErrorCtx_SetError("Only directed relationships are supported in CREATE");
			return AST_INVALID;
		}
	}

	return AST_VALID;
}

static AST_Validation _Validate_CREATE_Clauses(const AST *ast) {
	AST_Validation res = AST_VALID;

	uint *create_clause_indices = AST_GetClauseIndices(ast, CYPHER_AST_CREATE);
	uint clause_count = array_len(create_clause_indices);
	rax *defined_aliases = NULL;
	if(clause_count == 0) goto cleanup;

	defined_aliases = raxNew();
	uint start_offset = 0;
	for(uint i = 0; i < clause_count; i ++) {
		uint clause_idx = create_clause_indices[i];

		// Collect all entities that are bound before this CREATE clause.
		for (uint j = start_offset; j < clause_idx; j++) {
			const cypher_astnode_t *prev_clause = cypher_ast_query_get_clause(ast->root, j);
			_AST_GetDefinedIdentifiers(prev_clause, defined_aliases);
		}
		start_offset = clause_idx;

		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, clause_idx);
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);

		// Verify that functions invoked in the CREATE pattern are valid.
		if (_ValidateFunctionCalls(pattern, false) != AST_VALID) goto cleanup;

		uint path_count = cypher_ast_pattern_npaths(pattern);
		const cypher_astnode_t *prev_path = NULL;
		for (uint j = 0; j < path_count; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			// Collect aliases defined on the previous path in this CREATE clause.
			if (prev_path) _AST_Path_GetDefinedIdentifiers(prev_path, defined_aliases);
			res = _Validate_CREATE_Entities(path, defined_aliases);
			if (res == AST_INVALID) goto cleanup;
			prev_path = path;
		}
	}

	/* Since we combine all our CREATE clauses in a segment into one operation,
	 * make sure no data-modifying clauses can separate them. TCK example:
	 * CREATE (a:A), (b:B) MERGE (a)-[:KNOWS]->(b) CREATE (b)-[:KNOWS]->(c:C) RETURN count(*)
	 */
	for(uint i = create_clause_indices[0] + 1; i < create_clause_indices[clause_count - 1]; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(clause) == CYPHER_AST_MERGE) {
			ErrorCtx_SetError("RedisGraph does not support queries of the form CREATE...MERGE...CREATE without a separating WITH clause.");
			res = AST_INVALID;
			goto cleanup;
		}
	}
cleanup:
	array_free(create_clause_indices);
	if(defined_aliases) raxFree(defined_aliases);
	return res;
}

static AST_Validation _Validate_DELETE_Clauses(const AST *ast) {
	const cypher_astnode_t **delete_clauses = AST_GetClauses(ast, CYPHER_AST_DELETE);
	AST_Validation res = AST_VALID;
	uint clause_count = array_len(delete_clauses);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = delete_clauses[i];
		uint expression_count = cypher_ast_delete_nexpressions(clause);
		for(uint j = 0; j < expression_count; j++) {
			const cypher_astnode_t *exp = cypher_ast_delete_get_expression(clause, j);
			cypher_astnode_type_t type = cypher_astnode_type(exp);
			// expecting an identifier or a function call
			// identifiers and calls that don't resolve to a node or edge
			// will raise an error at run-time
			if(type != CYPHER_AST_IDENTIFIER &&
			   type != CYPHER_AST_APPLY_OPERATOR &&
			   type != CYPHER_AST_APPLY_ALL_OPERATOR &&
			   type != CYPHER_AST_SUBSCRIPT_OPERATOR) {
				ErrorCtx_SetError("DELETE can only be called on nodes and relationships");
				res = AST_INVALID;
				goto cleanup;
			}
		}

		// validate any func
		bool include_aggregates = false;
		res = _ValidateFunctionCalls(clause, include_aggregates);
		if(res == AST_INVALID) goto cleanup;
	}

cleanup:
	array_free(delete_clauses);
	return res;
}

AST_Validation _AST_ValidateResultColumns
(
	const cypher_astnode_t *return_clause
) {
	ASSERT(return_clause != NULL);

	if(cypher_ast_return_has_include_existing(return_clause)) {
		return AST_VALID;
	}

	rax           *rax          = raxNew();
	AST_Validation res          = AST_VALID;
	const char   **columns      = AST_BuildReturnColumnNames(return_clause);
	uint           column_count = array_len(columns);

	for (uint i = 0; i < column_count; i++) {
		// column with same name is invalid
		if(raxTryInsert(rax, (unsigned char *)columns[i], strlen(columns[i]), NULL, NULL) == 0) {
			ErrorCtx_SetError("Error: Multiple result columns with the same name are not supported.");
			res = AST_INVALID;
			break;
		}
	}
	
	raxFree(rax);
	array_free(columns);
	
	return res;
}

static AST_Validation _Validate_RETURN_Clause(const AST *ast) {
	const cypher_astnode_t *return_clause;
	return_clause = AST_GetClause(ast, CYPHER_AST_RETURN, NULL);

	// no RETURN clause, nothing to validate
	if(return_clause == NULL) return AST_VALID;

	// validate all user-specified functions in RETURN clause
	bool include_aggregates = true;
	if(_ValidateFunctionCalls(return_clause, include_aggregates) == AST_INVALID)
		return AST_INVALID;
	
	return _AST_ValidateResultColumns(return_clause);
}

static AST_Validation _Validate_UNWIND_Clauses(const AST *ast) {
	const cypher_astnode_t **unwind_clauses = AST_GetClauses(ast, CYPHER_AST_UNWIND);
	if(!unwind_clauses) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint clause_count = array_len(unwind_clauses);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *expression = cypher_ast_unwind_get_expression(unwind_clauses[i]);
		// Verify that all elements of the UNWIND collection are supported by RedisGraph
		uint child_count = cypher_astnode_nchildren(expression);
		for(uint j = 0; j < child_count; j ++) {
			res = CypherWhitelist_ValidateQuery(cypher_astnode_get_child(expression, j));
			if(res != AST_VALID) goto cleanup;
		}
		// Verify that UNWIND doesn't call non-existent or unsupported functions.
		res = _ValidateFunctionCalls(expression, true);
		if(res != AST_VALID) goto cleanup;
	}

cleanup:
	array_free(unwind_clauses);
	return res;
}

// LIMIT and SKIP are not independent clauses, but modifiers that can be applied to WITH or RETURN clauses
static AST_Validation _Validate_LIMIT_SKIP_Modifiers(const AST *ast) {
	// Handle modifiers on the RETURN clause
	const cypher_astnode_t *return_clause = AST_GetClause(ast,
														  CYPHER_AST_RETURN, NULL);
	// Skip check if the RETURN clause does not specify a limit
	if(return_clause) {
		// Handle LIMIT modifier
		const cypher_astnode_t *limit = cypher_ast_return_get_limit(return_clause);
		if(limit) {
			// Handle non-integer or non parameter types specified as LIMIT value
			// The value validation of integer node or parameter node is done in run time evaluation.
			if(cypher_astnode_type(limit) != CYPHER_AST_INTEGER &&
			   cypher_astnode_type(limit) != CYPHER_AST_PARAMETER) {
				ErrorCtx_SetError("LIMIT specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}
		}

		// Handle SKIP modifier
		const cypher_astnode_t *skip = cypher_ast_return_get_skip(return_clause);
		if(skip) {
			// Handle non-integer or non parameter types specified as skip value
			// The value validation of integer node or parameter node is done in run time evaluation.
			if(cypher_astnode_type(skip) != CYPHER_AST_INTEGER &&
			   cypher_astnode_type(skip) != CYPHER_AST_PARAMETER) {
				ErrorCtx_SetError("SKIP specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}
		}
	}

	// Handle LIMIT modifiers on all WITH clauses
	const cypher_astnode_t **with_clauses = AST_GetClauses(ast, CYPHER_AST_WITH);
	if(!with_clauses) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint with_count = array_len(with_clauses);
	for(uint i = 0; i < with_count; i++) {
		const cypher_astnode_t *with_clause = with_clauses[i];
		// Handle LIMIT modifier
		const cypher_astnode_t *limit = cypher_ast_with_get_limit(with_clause);
		if(limit) {
			// Handle non-integer or non parameter types specified as LIMIT value
			// The value validation of integer node or parameter node is done in run time evaluation.
			if(cypher_astnode_type(limit) != CYPHER_AST_INTEGER &&
			   cypher_astnode_type(limit) != CYPHER_AST_PARAMETER) {
				ErrorCtx_SetError("LIMIT specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}
		}

		// Handle SKIP modifier
		const cypher_astnode_t *skip = cypher_ast_with_get_skip(with_clause);
		if(skip) {
			// Handle non-integer or non parameter types specified as skip value
			// The value validation of integer node or parameter node is done in run time evaluation.
			if(cypher_astnode_type(skip) != CYPHER_AST_INTEGER &&
			   cypher_astnode_type(skip) != CYPHER_AST_PARAMETER) {
				ErrorCtx_SetError("SKIP specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}
		}
	}
	array_free(with_clauses);

	return res;
}

// A query must end in a RETURN clause, a procedure, or an updating clause
// (CREATE, MERGE, DELETE, SET, or REMOVE once supported)
static AST_Validation _ValidateQueryTermination(const AST *ast) {
	ASSERT(ast != NULL);

	uint clause_idx = 0;
	const cypher_astnode_t *return_clause = NULL;
	const cypher_astnode_t *following_clause = NULL;
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	// libcypher-parser do not enforce clause sequance order:
	// queries such as 'RETURN CREATE' and 'RETURN RETURN' are considered
	// valid by the parser
	// make sure the only clause following RETURN is UNION

	// get first instance of a RETURN clause
	return_clause = AST_GetClause(ast, CYPHER_AST_RETURN, &clause_idx);
	if(return_clause != NULL && clause_idx < clause_count - 1) {
		// RETURN clause isn't the last clause
		// the only clause which can follow a RETURN is the UNION clause
		following_clause = AST_GetClauseByIdx(ast, clause_idx + 1);
		if(cypher_astnode_type(following_clause) != CYPHER_AST_UNION) {
			// unexpected clause following RETURN
			ErrorCtx_SetError("Unexpected clause following RETURN");
			return AST_INVALID;
		}
	}

	const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
	cypher_astnode_type_t type = cypher_astnode_type(last_clause);
	if(type != CYPHER_AST_RETURN   &&
	   type != CYPHER_AST_CREATE   &&
	   type != CYPHER_AST_MERGE    &&
	   type != CYPHER_AST_DELETE   &&
	   type != CYPHER_AST_SET      &&
	   type != CYPHER_AST_CALL
	  ) {
		ErrorCtx_SetError("Query cannot conclude with %s (must be RETURN or an update clause)",
						  cypher_astnode_typestr(type));
		return AST_INVALID;
	}
	return AST_VALID;
}

static void _AST_RegisterCallOutputs(const cypher_astnode_t *call_clause, rax *identifiers) {
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
	ProcedureCtx *proc = Proc_Get(proc_name);
	ASSERT(proc != NULL);

	unsigned int output_count = array_len(proc->output);
	for(uint i = 0; i < output_count; i++) {
		const char *name = Procedure_GetOutput(proc, i);
		raxInsert(identifiers, (unsigned char *)name, strlen(name), NULL, NULL);
	}
}

// Perform validations not constrained to a specific scope
static AST_Validation _ValidateQuerySequence(const AST *ast) {

	// Validate the final clause
	if(_ValidateQueryTermination(ast) != AST_VALID) return AST_INVALID;

	// The query cannot begin with a "WITH/RETURN *" projection.
	const cypher_astnode_t *start_clause = cypher_ast_query_get_clause(ast->root, 0);
	if(cypher_astnode_type(start_clause) == CYPHER_AST_WITH &&
	   cypher_ast_with_has_include_existing(start_clause)) {
		ErrorCtx_SetError("Query cannot begin with 'WITH *'.");
		return AST_INVALID;
	}

	if(cypher_astnode_type(start_clause) == CYPHER_AST_RETURN &&
	   cypher_ast_return_has_include_existing(start_clause)) {
		ErrorCtx_SetError("Query cannot begin with 'RETURN *'.");
		return AST_INVALID;
	}

	return AST_VALID;
}

/* In any given query scope, reading clauses (MATCH, UNWIND, and InQueryCall)
 * cannot follow updating clauses (CREATE, MERGE, DELETE, SET, REMOVE).
 * https://s3.amazonaws.com/artifacts.opencypher.org/railroad/SinglePartQuery.html
 * Additionally, a MATCH clause cannot follow an OPTIONAL MATCH clause. */
static AST_Validation _ValidateClauseOrder(const AST *ast) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	bool encountered_optional_match = false;
	bool encountered_updating_clause = false;
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(!encountered_updating_clause && (type == CYPHER_AST_CREATE || type == CYPHER_AST_MERGE ||
											type == CYPHER_AST_DELETE || type == CYPHER_AST_SET ||
											type == CYPHER_AST_REMOVE)) {
			encountered_updating_clause = true;
		} else if(encountered_updating_clause && (type == CYPHER_AST_MATCH ||
												  type == CYPHER_AST_UNWIND ||
												  type == CYPHER_AST_CALL)) {
			ErrorCtx_SetError("A WITH clause is required to introduce %s after an updating clause.",
							  cypher_astnode_typestr(type));
			return AST_INVALID;
		}

		if(type == CYPHER_AST_MATCH) {
			// Check whether this match is optional.
			bool current_clause_is_optional = cypher_ast_match_is_optional(clause);
			// If the current clause is non-optional but we have already encountered an optional match, emit an error.
			if(!current_clause_is_optional && encountered_optional_match) {
				ErrorCtx_SetError("A WITH clause is required to introduce a MATCH clause after an OPTIONAL MATCH.");
				return AST_INVALID;
			}
			encountered_optional_match |= current_clause_is_optional;
		}
	}

	return AST_VALID;
}

static void _AST_Path_GetDefinedIdentifiers(const cypher_astnode_t *path, rax *identifiers) {
	/* Collect the aliases of named paths, nodes, and edges.
	 * All more deeply-nested identifiers are referenced rather than defined,
	 * and will not be collected. This enforces reference checking on aliases like 'fake' in:
	 * MATCH (a {val: fake}) RETURN a */
	if(cypher_astnode_type(path) == CYPHER_AST_NAMED_PATH) {
		// If this is a named path, collect its alias.
		const cypher_astnode_t *alias_node = cypher_ast_named_path_get_identifier(path);
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		raxInsert(identifiers, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}

	uint path_len = cypher_ast_pattern_path_nelements(path);
	for(uint j = 0; j < path_len; j ++) {
		const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, j);
		// Retrieve the path element's alias if one is present.
		// Odd offsets correspond to edges, even offsets correspond to nodes.
		const cypher_astnode_t *alias_node = (j % 2) ?
											 cypher_ast_rel_pattern_get_identifier(elem) :
											 cypher_ast_node_pattern_get_identifier(elem);
		if(!alias_node) continue; // Skip unaliased entities.
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		raxInsert(identifiers, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}

}

static void _AST_Pattern_GetDefinedIdentifiers(const cypher_astnode_t *pattern, rax *identifiers) {
	/* Collect all aliases defined in a MATCH or CREATE pattern,
	 * which is comprised of 1 or more paths. */
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		// Collect aliases defined on each path.
		_AST_Path_GetDefinedIdentifiers(path, identifiers);
	}
}

static void _AST_CreateIndex_GetDefinedIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
	uint child_count = cypher_astnode_nchildren(node);
	for(uint c = 0; c < child_count; c ++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
		cypher_astnode_type_t type = cypher_astnode_type(child);
		if(type == CYPHER_AST_IDENTIFIER) {
			const char *identifier = cypher_ast_identifier_get_name(child);
			raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
			return;
		}
	}
}

static void _AST_GetDefinedIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
	if(!node) return;
	cypher_astnode_type_t type = cypher_astnode_type(node);

	if(type == CYPHER_AST_RETURN) {
		/* Only collect aliases (which may be referenced in an ORDER BY)
		 * from the RETURN clause, rather than all identifiers */
		_AST_GetReturnAliases(node, identifiers);
	} else if(type == CYPHER_AST_WITH) {
		// Get alias if one is provided; otherwise use the expression identifier
		_AST_GetWithAliases(node, identifiers);
	} else if(type == CYPHER_AST_CALL) {
		// Get alias if one is provided; otherwise use the expression identifier
		_AST_GetProcCallAliases(node, identifiers);
	} else if(type == CYPHER_AST_MATCH) {
		/* Collect all identifiers defined by the pattern in the MATCH clause,
		 * ignoring references in property maps and WHERE predicates. */
		const cypher_astnode_t *match_pattern = cypher_ast_match_get_pattern(node);
		_AST_Pattern_GetDefinedIdentifiers(match_pattern, identifiers);
	} else if(type == CYPHER_AST_MERGE) {
		/* Collect all identifiers defined by the path in the MERGE clause,
		 * ignoring references in property maps and ON CREATE / ON MATCH actions. */
		const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(node);
		_AST_Path_GetDefinedIdentifiers(merge_path, identifiers);
	} else if(type == CYPHER_AST_CREATE) {
		/* Collect all identifiers defined by the pattern in the CREATE clause,
		 * ignoring references in property maps.  */
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(node);
		_AST_Pattern_GetDefinedIdentifiers(pattern, identifiers);
	} else if(type == CYPHER_AST_UNWIND) {
		/* UNWIND only defines its own alias, which is just 'defined' in the query:
		 * UNWIND [ref_1, ref_2] AS defined RETURN defined */
		const cypher_astnode_t *unwind_alias_node = cypher_ast_unwind_get_alias(node);
		const char *unwind_alias = cypher_ast_identifier_get_name(unwind_alias_node);
		raxInsert(identifiers, (unsigned char *)unwind_alias, strlen(unwind_alias), NULL, NULL);
	} else if(type == CYPHER_AST_CALL) {
		_AST_RegisterCallOutputs(node, identifiers);
	} else if(type == CYPHER_AST_CREATE_NODE_PROPS_INDEX || 
		type == CYPHER_AST_CREATE_PATTERN_PROPS_INDEX ||
		type == CYPHER_AST_DROP_PROPS_INDEX) {
		_AST_CreateIndex_GetDefinedIdentifiers(node, identifiers);
	}
	else {
		uint child_count = cypher_astnode_nchildren(node);
		for(uint c = 0; c < child_count; c ++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
			_AST_GetDefinedIdentifiers(child, identifiers);
		}
	}
}

static void _AST_GetReferredIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
	if(!node) return;
	if(cypher_astnode_type(node) == CYPHER_AST_WITH) {
		// WITH clauses should only have their inputs collected, not their outputs.
		_AST_GetWithReferences(node, identifiers);
	} else {
		_AST_GetIdentifiers(node, identifiers);
	}
}

static AST_Validation _Validate_Aliases_DefinedInClause(const cypher_astnode_t *clause,
														rax *defined_aliases) {
	AST_Validation res = AST_VALID;
	rax *referred_identifiers = raxNew();

	// Get defined identifiers.
	_AST_GetDefinedIdentifiers(clause, defined_aliases);

	// Get referred identifiers.
	_AST_GetReferredIdentifiers(clause, referred_identifiers);

	raxIterator it;
	_prepareIterateAll(referred_identifiers, &it);

	// See that each referred identifier is defined.
	while(raxNext(&it)) {
		int len = it.key_len;
		unsigned char *alias = it.key;
		if(raxFind(defined_aliases, alias, len) == raxNotFound) {
			ErrorCtx_SetError("%.*s not defined", len, alias);
			res = AST_INVALID;
			break;
		}
	}

	// Clean up:
	raxStop(&it);
	raxFree(referred_identifiers);
	return res;
}

/* Check that all referred identifiers been defined. */
static AST_Validation _Validate_Aliases_Defined(const AST *ast) {
	AST_Validation res = AST_VALID;

	// Retrieve the indices of each WITH clause to properly set the bounds of each scope.
	// If the query does not have a WITH clause, there is only one scope.
	uint end_offset = cypher_ast_query_nclauses(ast->root);
	rax *defined_aliases = raxNew();
	for(uint i = 0; i < end_offset; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		// For each clause, confirm that all referred aliases have been previously defined.
		res = _Validate_Aliases_DefinedInClause(clause, defined_aliases);
		if(res != AST_VALID) break;
		if(cypher_astnode_type(clause) == CYPHER_AST_WITH) {
			// each WITH clause marks the beginning of a new scope for defined aliases
			// if the WITH clause contains a star projection, all variables from
			// the previous scope are carried over
			if(!cypher_ast_with_has_include_existing(clause)) {
				raxFree(defined_aliases);
				defined_aliases = raxNew();
			}
			_AST_GetDefinedIdentifiers(clause, defined_aliases);
		}
	}
	raxFree(defined_aliases);
	return res;
}

// Report encountered errors by libcypher-parser.
void AST_ReportErrors(const cypher_parse_result_t *result) {
	ASSERT(cypher_parse_result_nerrors(result) > 0);

	// report first encountered error
	const cypher_parse_error_t *error =
		cypher_parse_result_get_error(result, 0);

	// Get the position of an error.
	struct cypher_input_position errPos = cypher_parse_error_position(error);

	// Get the error message of an error.
	const char *errMsg = cypher_parse_error_message(error);

	// Get the error context of an error.
	// This returns a pointer to a null-terminated string, which contains a
	// section of the input around where the error occurred, that is limited
	// in length and suitable for presentation to a user.
	const char *errCtx = cypher_parse_error_context(error);

	// Get the offset into the context of an error.
	// Identifies the point of the error within the context string, allowing
	// this to be reported to the user, typically with an arrow pointing to the
	// invalid character.
	size_t errCtxOffset = cypher_parse_error_context_offset(error);
	ErrorCtx_SetError("errMsg: %s line: %u, column: %u, offset: %zu errCtx: %s errCtxOffset: %zu",
					  errMsg, errPos.line, errPos.column, errPos.offset, errCtx, errCtxOffset);
}

// checks if set items contains non-alias referenes in lhs
static AST_Validation _Validate_SETItems(const cypher_astnode_t *set_clause) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	for(uint i = 0; i < nitems; i++) {
		// Get the SET directive at this index.
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		const cypher_astnode_type_t type = cypher_astnode_type(set_item);
		if(type == CYPHER_AST_SET_PROPERTY) {
			const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);
			const cypher_astnode_t *ast_entity = cypher_ast_property_operator_get_expression(ast_prop);
			if(cypher_astnode_type(ast_entity) != CYPHER_AST_IDENTIFIER) {
				ErrorCtx_SetError("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions");
				return AST_INVALID;
			}
		}
	}
	return AST_VALID;
}

// checks if SET cluase contains aggregation function
static AST_Validation _Validate_SET_Clauses(const AST *ast) {
	const cypher_astnode_t **set_clauses = AST_GetClauses(ast, CYPHER_AST_SET);
	if(set_clauses == NULL) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint set_count = array_len(set_clauses);

	for(uint i = 0; i < set_count; i++) {
		res = _Validate_SETItems(set_clauses[i]);
		if(res != AST_VALID) break;

		// validate function calls within the SET clause
		bool include_aggregates = false;
		res = _ValidateFunctionCalls(set_clauses[i], include_aggregates);
		if(res != AST_VALID) break;
	}

	array_free(set_clauses);
	return res;
}

static AST_Validation _ValidateClauses(const AST *ast) {
	// Verify that the clause order in the scope is valid.
	if(_ValidateClauseOrder(ast) != AST_VALID) return AST_INVALID;

	if(_Validate_CALL_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_MATCH_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_WITH_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_MERGE_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_CREATE_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_DELETE_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_RETURN_Clause(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_UNWIND_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_SET_Clauses(ast) == AST_INVALID) return AST_INVALID;

	if(_Validate_LIMIT_SKIP_Modifiers(ast) == AST_INVALID) return AST_INVALID;

	return AST_VALID;
}

static AST_Validation _ValidateUnion_Clauses(const AST *ast) {
	if(!AST_ContainsClause(ast, CYPHER_AST_UNION)) return AST_VALID;

	/* Make sure there's no conflict between UNION clauses
	 * either all UNION clauses specify ALL or nither of them does. */
	AST_Validation res = AST_VALID;
	uint *union_indices = AST_GetClauseIndices(ast, CYPHER_AST_UNION);
	uint union_clause_count = array_len(union_indices);
	int has_all_count = 0;

	for(uint i = 0; i < union_clause_count; i++) {
		const cypher_astnode_t *union_clause = cypher_ast_query_get_clause(ast->root, union_indices[i]);
		if(cypher_ast_union_has_all(union_clause)) has_all_count++;
	}
	array_free(union_indices);

	// If we've encountered UNION ALL clause, all UNION clauses should specify ALL.
	if(has_all_count != 0) {
		if(has_all_count != union_clause_count) {
			ErrorCtx_SetError("Invalid combination of UNION and UNION ALL.");
			return AST_INVALID;
		}
	}

	// Require all RETURN clauses to perform the exact same projection.
	uint *return_indices = AST_GetClauseIndices(ast, CYPHER_AST_RETURN);
	uint return_clause_count = array_len(return_indices);
	// We should have one more RETURN clause than we have UNION clauses.
	if(return_clause_count != union_clause_count + 1) {
		ErrorCtx_SetError("Found %d UNION clauses but only %d RETURN clauses.", union_clause_count,
						  return_clause_count);
		return AST_INVALID;
	}

	const cypher_astnode_t *return_clause = cypher_ast_query_get_clause(ast->root, return_indices[0]);
	uint proj_count = cypher_ast_return_nprojections(return_clause);
	const char *projections[proj_count];

	for(uint j = 0; j < proj_count; j++) {
		const cypher_astnode_t *proj = cypher_ast_return_get_projection(return_clause, j);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(proj);
		if(alias_node == NULL)  {
			// The projection was not aliased, so the projection itself must be an identifier.
			alias_node = cypher_ast_projection_get_expression(proj);
			ASSERT(cypher_astnode_type(alias_node) == CYPHER_AST_IDENTIFIER);
		}
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		projections[j] = alias;
	}

	for(uint i = 1; i < return_clause_count; i++) {
		return_clause = cypher_ast_query_get_clause(ast->root, return_indices[i]);
		if(proj_count != cypher_ast_return_nprojections(return_clause)) {
			ErrorCtx_SetError("All sub queries in an UNION must have the same column names.");
			res = AST_INVALID;
			goto cleanup;
		}

		for(uint j = 0; j < proj_count; j++) {
			const cypher_astnode_t *proj = cypher_ast_return_get_projection(return_clause, j);
			const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(proj);
			if(alias_node == NULL)  {
				// The projection was not aliased, so the projection itself must be an identifier.
				alias_node = cypher_ast_projection_get_expression(proj);
				ASSERT(cypher_astnode_type(alias_node) == CYPHER_AST_IDENTIFIER);
			}
			const char *alias = cypher_ast_identifier_get_name(alias_node);
			if(strcmp(projections[j], alias) != 0) {
				ErrorCtx_SetError("All sub queries in an UNION must have the same column names.");
				res = AST_INVALID;
				goto cleanup;
			}
		}
	}

cleanup:
	array_free(return_indices);
	return res;
}

static AST_Validation _ValidateParamsOnly(const cypher_astnode_t *statement) {
	uint noptions = cypher_ast_statement_noptions(statement);
	for(uint i = 0; i < noptions; i++) {
		const cypher_astnode_t *option = cypher_ast_statement_get_option(statement, i);
		const cypher_astnode_type_t type = cypher_astnode_type(option);
		if((type == CYPHER_AST_EXPLAIN_OPTION) || (type == CYPHER_AST_PROFILE_OPTION)) {
			const char *invalid_option_name = cypher_astnode_typestr(type);
			ErrorCtx_SetError("Please use GRAPH.%s 'key' 'query' command instead of GRAPH.QUERY 'key' '%s query'",
							  invalid_option_name, invalid_option_name);
			return AST_INVALID;
		}
	}
	return AST_VALID;
}

static AST_Validation _ValidateDuplicateParameters(const cypher_astnode_t *statement) {
	rax *param_names = raxNew();
	uint noptions = cypher_ast_statement_noptions(statement);
	for(uint i = 0; i < noptions; i++) {
		const cypher_astnode_t *option = cypher_ast_statement_get_option(statement, i);
		uint nparams = cypher_ast_cypher_option_nparams(option);
		for(uint j = 0; j < nparams; j++) {
			const cypher_astnode_t *param = cypher_ast_cypher_option_get_param(option, j);
			const char *paramName = cypher_ast_string_get_value(cypher_ast_cypher_option_param_get_name(param));
			// If parameter already exists return an error.
			if(!raxInsert(param_names, (unsigned char *) paramName, strlen(paramName), NULL, NULL)) {
				ErrorCtx_SetError("Duplicated parameter: %s", paramName);
				raxFree(param_names);
				return AST_INVALID;
			}
		}
	}
	raxFree(param_names);
	return AST_VALID;
}

static AST *_NewMockASTSegment(const cypher_astnode_t *root, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->free_root = true;
	ast->referenced_entities = NULL;
	ast->anot_ctx_collection = NULL;
	uint n = end_offset - start_offset;

	cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_query_get_clause(root, i + start_offset);
	}
	struct cypher_input_range range = {0};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n, clauses, n, range);
	ast->ref_count = rm_malloc(sizeof(uint));
	*(ast->ref_count) = 1;
	ast->parse_result = NULL;
	ast->params_parse_result = NULL;
	return ast;
}

static AST_Validation _ValidateScopes(AST *mock_ast) {
	AST_Validation res = AST_VALID;

	// Verify that the RETURN clause and terminating clause do not violate scoping rules.
	if(_ValidateQuerySequence(mock_ast) != AST_VALID) return AST_INVALID;

	// Validate identifiers, which may be passed between scopes
	if(_Validate_Aliases_Defined(mock_ast) == AST_INVALID) return AST_INVALID;

	// Aliases are scoped by the WITH clauses within the query.
	// If we have one or more WITH clauses, MATCH validations should be performed one scope at a time.
	uint *query_scopes = AST_GetClauseIndices(mock_ast, CYPHER_AST_WITH);
	uint with_clause_count = array_len(query_scopes);

	// Query has only one scope, no need to create sub-ASTs
	if(with_clause_count == 0) {
		res = _ValidateClauses(mock_ast);
		goto cleanup;
	}

	AST *scoped_ast;
	uint scope_end;
	uint scope_start = 0;
	for(uint i = 0; i < with_clause_count; i ++) {
		scope_end = query_scopes[i] + 1; // Switching from index to bound, so add 1
		// Make a sub-AST containing only the clauses in this scope
		scoped_ast = _NewMockASTSegment(mock_ast->root, scope_start, scope_end);

		// Perform validations
		res = _ValidateClauses(scoped_ast);
		AST_Free(scoped_ast);
		if(res != AST_VALID) goto cleanup;
		// Update the starting indices of the scope for the next iteration.
		scope_start = scope_end;
	}

	// Build and test the final scope (from the last WITH to the last clause)
	scope_end = cypher_ast_query_nclauses(mock_ast->root);
	scoped_ast = _NewMockASTSegment(mock_ast->root, scope_start, scope_end);
	res = _ValidateClauses(scoped_ast);
	AST_Free(scoped_ast);
	if(res != AST_VALID) goto cleanup;

cleanup:
	array_free(query_scopes);
	return res;
}

// Checks to see if libcypher-parser reported any errors.
bool AST_ContainsErrors(const cypher_parse_result_t *result) {
	return cypher_parse_result_nerrors(result) > 0;
}

/* This function checks for the existence a valid root in the query.
 * As cypher_parse_result_t can have multiple roots such as comments, only a query that has
 * a root with type CYPHER_AST_STATEMENT is considered valid. Comment roots are ignored. */
static AST_Validation _AST_Validate_ParseResultRoot(const cypher_parse_result_t *result,
													int *index) {
	// Check for failures in libcypher-parser
	ASSERT(AST_ContainsErrors(result) == false);

	uint nroots = cypher_parse_result_nroots(result);
	for(uint i = 0; i < nroots; i++) {
		const cypher_astnode_t *root = cypher_parse_result_get_root(result, i);
		cypher_astnode_type_t root_type = cypher_astnode_type(root);
		if(root_type == CYPHER_AST_LINE_COMMENT || root_type == CYPHER_AST_BLOCK_COMMENT ||
		   root_type == CYPHER_AST_COMMENT) {
			continue;
		} else if(root_type != CYPHER_AST_STATEMENT) {
			ErrorCtx_SetError("Encountered unsupported query type '%s'", cypher_astnode_typestr(root_type));
			return AST_INVALID;
		} else {
			// We got a statement.
			*index = i;
			return AST_VALID;
		}
	}

	// query with no roots like ';'
	if(nroots == 0) {
		ErrorCtx_SetError("Error: empty query.");
	}

	return AST_INVALID;
}

static AST_Validation _AST_ValidateUnionQuery(AST *mock_ast) {
	// Verify that the UNION clauses and the columns they join are valid.
	AST_Validation res = _ValidateUnion_Clauses(mock_ast);
	if(res != AST_VALID) return res;

	// Each self-contained query delimited by a UNION clause has its own scope.
	uint *query_scopes = AST_GetClauseIndices(mock_ast, CYPHER_AST_UNION);
	// Append the clause count to check the final scope (from the last UNION to the last clause)
	array_append(query_scopes, cypher_ast_query_nclauses(mock_ast->root));
	uint scope_count = array_len(query_scopes);
	uint scope_start = 0;
	for(uint i = 0; i < scope_count; i ++) {
		uint scope_end = query_scopes[i];
		// Make a sub-AST containing only the clauses in this scope.
		AST *scoped_ast = _NewMockASTSegment(mock_ast->root, scope_start, scope_end);
		res = _ValidateScopes(scoped_ast);
		AST_Free(scoped_ast);
		if(res != AST_VALID) goto cleanup;

		// Update the starting index of the scope for the next iteration..
		scope_start = scope_end;
	}

cleanup:
	array_free(query_scopes);
	return res;
}

AST_Validation AST_Validate_QueryParams(const cypher_parse_result_t *result) {
	int index;
	if(_AST_Validate_ParseResultRoot(result, &index) != AST_VALID) return AST_INVALID;

	const cypher_astnode_t *root = cypher_parse_result_get_root(result, index);

	// in case of no parameters
	if(cypher_ast_statement_noptions(root) == 0) return AST_VALID;

	if(_ValidateParamsOnly(root)            != AST_VALID)  return AST_INVALID;
	if(_ValidateDuplicateParameters(root)   != AST_VALID)  return AST_INVALID;
	if(_ValidateFunctionCalls(root, false)  != AST_VALID)  return AST_INVALID;

	return AST_VALID;
}

AST_Validation AST_Validate_Query(const cypher_parse_result_t *result) {
	int index;
	if(_AST_Validate_ParseResultRoot(result, &index) != AST_VALID) {
		return AST_INVALID;
	}

	const cypher_astnode_t *root = cypher_parse_result_get_root(result, index);

	// Verify that the query does not contain any expressions not in the
	// RedisGraph support whitelist
	if(CypherWhitelist_ValidateQuery(root) != AST_VALID) return AST_INVALID;
	
	AST_Validation res;
	const cypher_astnode_t *body = cypher_ast_statement_get_body(root);
	cypher_astnode_type_t body_type = cypher_astnode_type(body);
	if(body_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX    ||
	   body_type == CYPHER_AST_CREATE_PATTERN_PROPS_INDEX ||
	   body_type == CYPHER_AST_DROP_PROPS_INDEX) {
		// Index operation
		rax *defined_aliases = raxNew();
		res = _Validate_Aliases_DefinedInClause(body, defined_aliases);
		raxFree(defined_aliases);
		return res;
	}

	// validate positions of allShortestPaths
	bool invalid = _ValidateAllShortestPaths(body);
	if(invalid) {
		ErrorCtx_SetError("RedisGraph support allShortestPaths only in match clauses");
		return AST_INVALID;
	}

	AST mock_ast; // Build a fake AST with the correct AST root
	mock_ast.root = body;

	// Check for invalid queries not captured by libcypher-parser
	if(AST_ContainsClause(&mock_ast, CYPHER_AST_UNION)) {
		// If the query contains a UNION clause, it has nested scopes that should be checked separately.
		res = _AST_ValidateUnionQuery(&mock_ast);
	} else {
		res = _ValidateScopes(&mock_ast);
	}

	return res;
}

