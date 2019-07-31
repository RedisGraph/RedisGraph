/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "ast_shared.h"
#include "cypher_whitelist.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

// Validate that an input string can be completely converted to a positive integer in range.
static inline AST_Validation _ValidatePositiveInteger(const char *input) {
	assert(input);
	char *endptr; // If the entire string is converted, endptr will point to a null byte
	errno = 0; // If underflow or overflow occurs, errno will be set

	strtol(input, &endptr, 0); // Perform conversion

	if(errno != 0 || *endptr != '\0') return AST_INVALID;

	return AST_VALID;
}

static void _AST_GetIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
	if(!node) return;
	assert(identifiers);

	if(cypher_astnode_type(node) == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(node);
		TrieMap_Add(identifiers, (char *)identifier, strlen(identifier), NULL, TrieMap_DONT_CARE_REPLACE);
	}

	uint child_count = cypher_astnode_nchildren(node);

	/* In case current node is of type projection
	 * inspect first child only,
	 * @10  20..26  > > > projection           expression=@11, alias=@14
	 * @11  20..26  > > > > apply              @12(@13)
	 * @12  20..23  > > > > > function name    `max`
	 * @13  24..25  > > > > > identifier       `z`
	 * @14  20..26  > > > > identifier         `max(z)` */
	if(cypher_astnode_type(node) == CYPHER_AST_PROJECTION) child_count = 1;

	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		_AST_GetIdentifiers(child, identifiers);
	}
}

static void _AST_GetWithAliases(const cypher_astnode_t *node, TrieMap *aliases) {
	if(!node) return;
	if(cypher_astnode_type(node) != CYPHER_AST_WITH) return;
	assert(aliases);

	uint num_with_projections = cypher_ast_with_nprojections(node);
	for(uint i = 0; i < num_with_projections; i ++) {
		const cypher_astnode_t *child = cypher_ast_with_get_projection(node, i);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(child);
		const char *alias;
		if(alias_node) {
			alias = cypher_ast_identifier_get_name(alias_node);
		} else {
			const cypher_astnode_t *expr = cypher_ast_projection_get_expression(child);
			assert(cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "WITH a"
			alias = cypher_ast_identifier_get_name(expr);
		}
		TrieMap_Add(aliases, (char *)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
	}
}

// UNWIND and WITH also form aliases, but don't need special handling for us yet.
static void _AST_GetReturnAliases(const cypher_astnode_t *node, TrieMap *aliases) {
	assert(node && aliases && cypher_astnode_type(node) == CYPHER_AST_RETURN);

	uint num_return_projections = cypher_ast_return_nprojections(node);
	if(num_return_projections == 0) return;

	for(uint i = 0; i < num_return_projections; i ++) {
		const cypher_astnode_t *child = cypher_ast_return_get_projection(node, i);
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(child);
		if(alias_node == NULL) continue;
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		TrieMap_Add(aliases, (char *)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
	}
}

static void _CollectIdentifiers(const cypher_astnode_t *root, TrieMap *projections) {
	if(cypher_astnode_type(root) == CYPHER_AST_IDENTIFIER) {
		// Identifier found, add to triemap
		const char *identifier = cypher_ast_identifier_get_name(root);
		TrieMap_Add(projections, (char *)identifier, strlen(identifier), NULL, TrieMap_DONT_CARE_REPLACE);
	} else {
		uint child_count = cypher_astnode_nchildren(root);
		for(uint i = 0; i < child_count; i ++) {
			_CollectIdentifiers(cypher_astnode_get_child(root, i), projections);
		}
	}
}

/* Collect all identifiers used in the RETURN clause (but not aliases defined there) */
static TrieMap *_AST_GetReturnProjections(const cypher_astnode_t *return_clause) {
	if(!return_clause) return NULL;

	uint projection_count = cypher_ast_return_nprojections(return_clause);
	if(projection_count == 0) return NULL;

	TrieMap *projections = NewTrieMap();
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		_CollectIdentifiers(projection, projections);
	}

	return projections;
}

/* Compares a triemap of user-specified functions with the registered functions we provide. */
static AST_Validation _ValidateReferredFunctions(TrieMap *referred_functions, char **reason,
												 bool include_aggregates) {
	AST_Validation res = AST_VALID;
	void *value;
	tm_len_t len;
	char *funcName;
	TrieMapIterator *it = TrieMap_Iterate(referred_functions, "", 0);
	*reason = NULL;

	// TODO: return RAX.
	while(TrieMapIterator_Next(it, &funcName, &len, &value)) {
		funcName[len] = 0;
		// No functions have a name longer than 32 characters
		if(len >= 32) {
			res = AST_INVALID;
			break;
		}

		if(AR_FuncExists(funcName)) continue;

		if(Agg_FuncExists(funcName)) {
			if(include_aggregates) {
				continue;
			} else {
				// Provide a unique error for using aggregate functions from inappropriate contexts
				asprintf(reason, "Invalid use of aggregating function '%s'", funcName);
				res = AST_INVALID;
				break;
			}
		}

		// If we reach this point, the function was not found
		res = AST_INVALID;
		break;
	}

	// If the function was not found, provide a reason if one is not set
	if(res == AST_INVALID && *reason == NULL) asprintf(reason, "Unknown function '%s'", funcName);

	TrieMapIterator_Free(it);
	return res;
}

static inline bool _AliasIsReturned(TrieMap *projections, const char *identifier) {
	if(TrieMap_Find(projections, (char *)identifier, strlen(identifier)) != TRIEMAP_NOTFOUND) {
		return true;
	}
	return false;
}

// If we have a multi-hop traversal (fixed or variable length), we cannot currently return that entity.
static AST_Validation _ValidateMultiHopTraversal(TrieMap *projections, const cypher_astnode_t *edge,
												 const cypher_astnode_t *range,
												 char **reason) {
	int start = 1;
	int end = INT_MAX - 2;

	const cypher_astnode_t *range_start = cypher_ast_range_get_start(range);
	if(range_start) start = AST_ParseIntegerNode(range_start);

	const cypher_astnode_t *range_end = cypher_ast_range_get_end(range);
	if(range_end) end = AST_ParseIntegerNode(range_end);

	// Validate specified range
	if(start > end) {
		asprintf(reason,
				 "Variable length path, maximum number of hops must be greater or equal to minimum number of hops.");
		return AST_INVALID;
	}

	bool multihop = (start > 1) || (start != end);
	if(!multihop) return AST_VALID;

	// Multi-hop traversals cannot (currently) be filtered on
	if(cypher_ast_rel_pattern_get_properties(edge) != NULL) {
		asprintf(reason, "RedisGraph does not currently support filters on variable-length paths.");
		return AST_INVALID;
	}

	// Multi-hop traversals cannot be referenced
	if(!projections) return AST_VALID;

	// Check if relation has an alias
	const cypher_astnode_t *ast_identifier = cypher_ast_rel_pattern_get_identifier(edge);
	if(!ast_identifier) return AST_VALID;

	// Verify that the alias is not found in the RETURN clause.
	const char *identifier = cypher_ast_identifier_get_name(ast_identifier);
	if(_AliasIsReturned(projections, identifier)) {
		asprintf(reason, "Cannot return variable-length traversal '%s'.", identifier);
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_ReusedEdges(const cypher_astnode_t *node,
											TrieMap *identifiers, char **reason) {
	uint child_count = cypher_astnode_nchildren(node);
	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		cypher_astnode_type_t type = cypher_astnode_type(child);

		if(type == CYPHER_AST_IDENTIFIER) {
			const char *identifier = cypher_ast_identifier_get_name(child);
			int new = TrieMap_Add(identifiers, (char *)identifier, strlen(identifier), NULL,
								  TrieMap_DONT_CARE_REPLACE);
			if(!new) {
				asprintf(reason, "Cannot use the same relationship variable '%s' for multiple patterns.",
						 identifier);
				return AST_INVALID;
			}
		}
	}

	return AST_VALID;
}

static AST_Validation _ValidateRelation(TrieMap *projections, const cypher_astnode_t *edge,
										TrieMap *identifiers,
										char **reason) {
	AST_Validation res = AST_VALID;

	// Make sure relation identifier is unique
	res = _Validate_ReusedEdges(edge, identifiers, reason);
	if(res != AST_VALID) return res;

	// If this is a multi-hop traversal, validate it
	const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(edge);
	if(range) {
		res = _ValidateMultiHopTraversal(projections, edge, range, reason);
		if(res != AST_VALID) return res;
	}

	// Validate that the relation is explicit and directed
	// TODO Lift this limitation when possible
	if(cypher_ast_rel_pattern_get_direction(edge) == CYPHER_REL_BIDIRECTIONAL) {
		asprintf(reason, "RedisGraph does not currently support undirected relations.");
		return AST_INVALID;
	};

	return res;
}

static AST_Validation _ValidatePath(const cypher_astnode_t *path,
									TrieMap *projections,
									TrieMap *identifiers,
									char **reason) {
	AST_Validation res = AST_VALID;
	uint path_len = cypher_ast_pattern_path_nelements(path);

	// Check all relations on the path (every odd offset)
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
		res = _ValidateRelation(projections, edge, identifiers, reason);
		if(res != AST_VALID) return res;
	}

	return res;
}

static AST_Validation _ValidatePattern(TrieMap *projections, const cypher_astnode_t *pattern,
									   TrieMap *identifiers, char **reason) {
	AST_Validation res = AST_VALID;
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		res = _ValidatePath(cypher_ast_pattern_get_path(pattern, i), projections, identifiers, reason);
		if(res != AST_VALID) break;
	}

	return res;
}

static bool _ValueIsConstant(const cypher_astnode_t *root) {
	cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == CYPHER_AST_PROPERTY_OPERATOR ||
	   type == CYPHER_AST_IDENTIFIER
	  ) {
		return false;
	}

	// Recursively visit children
	uint child_count = cypher_astnode_nchildren(root);
	for(uint i = 0; i < child_count; i++) {
		if(!_ValueIsConstant(cypher_astnode_get_child(root, i))) return false;
	}

	return true;
}

// Validate the property maps used in node/edge patterns in MATCH, MERGE, and CREATE clauses
static AST_Validation _ValidateInlinedProperties(const cypher_astnode_t *props, char **reason) {
	cypher_astnode_type_t type = cypher_astnode_type(props);
	if(type == CYPHER_AST_PARAMETER) {
		asprintf(reason, "Parameters are not currently supported in RedisGraph.");
		return AST_INVALID;
	}

	if(type != CYPHER_AST_MAP) {
		// This should be impossible
		asprintf(reason, "Encountered unhandled type in inlined properties.");
		return AST_INVALID;
	}

	AST_Validation res = AST_VALID;
	uint prop_count = cypher_ast_map_nentries(props);
	for(uint i = 0; i < prop_count; i ++) {
		const cypher_astnode_t *value = cypher_ast_map_get_value(props, i);
		cypher_astnode_type_t value_type = cypher_astnode_type(value);
		if(value_type == CYPHER_AST_IDENTIFIER) {
			res = AST_INVALID;
			break;
		} else if(value_type == CYPHER_AST_PROPERTY_OPERATOR) {
			res = AST_INVALID;
			break;
		} else if(value_type == CYPHER_AST_BINARY_OPERATOR) {
			const cypher_astnode_t *lhs = cypher_ast_binary_operator_get_argument1(value);
			if(!(_ValueIsConstant(lhs))) {
				res = AST_INVALID;
				break;
			}
			const cypher_astnode_t *rhs = cypher_ast_binary_operator_get_argument1(value);
			if(!(_ValueIsConstant(rhs))) {
				res = AST_INVALID;
				break;
			}
		}
	}
	if(res == AST_INVALID) {
		asprintf(reason, "Non-constant values cannot currently be used as inlined props in RedisGraph.");
	}
	return res;
}

static AST_Validation _ValidateInlinedPropertiesOnPath(const cypher_astnode_t *path,
													   char **reason) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Check all nodes on path
	for(uint i = 0; i < path_len; i += 2) {
		const cypher_astnode_t *ast_identifier = NULL;
		const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *props = cypher_ast_node_pattern_get_properties(node);
		if(props && (_ValidateInlinedProperties(props, reason) != AST_VALID)) return AST_INVALID;
	}

	// Check all edges on path
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *ast_identifier = NULL;
		const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(edge);
		if(props && (_ValidateInlinedProperties(props, reason) != AST_VALID)) return AST_INVALID;
	}
	return AST_VALID;
}

static AST_Validation _ValidateFilterPredicates(const cypher_astnode_t *predicate, char **reason) {
	cypher_astnode_type_t type = cypher_astnode_type(predicate);

	// TODO These should all be supported in filter trees
	if(type == CYPHER_AST_APPLY_OPERATOR) {
		const cypher_astnode_t *ast_name = cypher_ast_apply_operator_get_func_name(predicate);
		const char *func_name = cypher_ast_function_name_get_value(ast_name);
		asprintf(reason, "Unary APPLY operators ('%s') are not currently supported in filters", func_name);
		return AST_INVALID;
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		const cypher_operator_t *op = cypher_ast_binary_operator_get_operator(predicate);
		const cypher_astnode_t *left = cypher_ast_binary_operator_get_argument1(predicate);
		const cypher_astnode_t *right = cypher_ast_binary_operator_get_argument2(predicate);

		if(op != CYPHER_OP_EQUAL  &&
		   op != CYPHER_OP_NEQUAL &&
		   op != CYPHER_OP_LT     &&
		   op != CYPHER_OP_LTE    &&
		   op != CYPHER_OP_GT     &&
		   op != CYPHER_OP_GTE) {
			// Currently, unary functions are only valid in filter trees as arguments to a direct
			// comparison function. Failing expressions include:
			// WHERE EXISTS(a.age) AND a.age > 30
			if(_ValidateFilterPredicates(left, reason) != AST_VALID) return AST_INVALID;

			if(_ValidateFilterPredicates(right, reason) != AST_VALID) return AST_INVALID;

		} else {
			// Check for chains of form:
			// WHERE n.prop1 < m.prop1 = n.prop2 <> m.prop2
			cypher_astnode_type_t left_type = cypher_astnode_type(left);
			cypher_astnode_type_t right_type = cypher_astnode_type(right);

			if(cypher_astnode_type(left) == CYPHER_AST_BINARY_OPERATOR ||
			   cypher_astnode_type(right) == CYPHER_AST_BINARY_OPERATOR) {
				asprintf(reason, "Comparison chains of length > 1 are not currently supported.");
				return AST_INVALID;
			}
		}
	} else if(type == CYPHER_AST_COMPARISON) {
		// WHERE 10 < n.value <= 3
		if(cypher_ast_comparison_get_length(predicate) > 1) {
			asprintf(reason, "Comparison chains of length > 1 are not currently supported.");
			return AST_INVALID;

		}
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		// WHERE exists(a.name)
		asprintf(reason, "Unary APPLY operators are not currently supported in filters.");
		return AST_INVALID;
	} else if(type == CYPHER_AST_PATTERN_PATH) {
		// Comparisons of form:
		// MATCH (a), (b) WHERE a.id = 0 AND (a)-[:T]->(b:TheLabel)
		asprintf(reason, "Paths cannot currently be specified in filters.");
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clause_Filters(const cypher_astnode_t *clause,
													 char **reason) {
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		if(_ValidateInlinedPropertiesOnPath(path, reason) != AST_VALID) return AST_INVALID;
	}

	const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(clause);
	if(predicate == NULL) return AST_VALID;

	if(_ValidateFilterPredicates(predicate, reason) != AST_VALID) return AST_INVALID;

	return AST_VALID;
}

static AST_Validation _Validate_MATCH_Clauses(const AST *ast, char **reason) {
	// Check to see if all mentioned inlined, outlined functions exists.
	// Inlined functions appear within entity definition ({a:v})
	// Outlined functions appear within the WHERE clause.
	// libcypher-parser doesn't have a WHERE node, all of the filters
	// are specified within the MATCH node sub-tree.
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	if(match_clauses == NULL) return AST_VALID;

	TrieMap *referred_funcs = NewTrieMap();
	TrieMap *identifiers = NewTrieMap();
	TrieMap *reused_entities = NewTrieMap();
	AST_Validation res;

	const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	TrieMap *projections = _AST_GetReturnProjections(return_clause);
	uint match_count = array_len(match_clauses);
	for(uint i = 0; i < match_count; i ++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		// Validate the pattern described by the MATCH clause
		res = _ValidatePattern(projections, cypher_ast_match_get_pattern(match_clause), identifiers,
							   reason);
		if(res == AST_INVALID) goto cleanup;

		// Validate that inlined filters do not use parameters
		res = _Validate_MATCH_Clause_Filters(match_clause, reason);
		if(res == AST_INVALID) goto cleanup;

		// Collect function references
		AST_ReferredFunctions(match_clause, referred_funcs);
	}

	// Verify that referred functions exist.
	bool include_aggregates = false;
	res = _ValidateReferredFunctions(referred_funcs, reason, include_aggregates);

cleanup:
	TrieMap_Free(referred_funcs, TrieMap_NOP_CB);
	TrieMap_Free(identifiers, TrieMap_NOP_CB);
	TrieMap_Free(reused_entities, TrieMap_NOP_CB);
	array_free(match_clauses);

	return res;
}

static AST_Validation _ValidateWithEntitiesOnPath(const cypher_astnode_t *path,
												  TrieMap *projections, char **reason) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Check all entities on the path
	for(uint i = 0; i < path_len; i ++) {
		const cypher_astnode_t *ast_identifier = NULL;
		if(i % 2) {  // Checking an edge pattern
			const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
			ast_identifier = cypher_ast_rel_pattern_get_identifier(edge);
		} else { // Checking a node pattern
			const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, i);
			ast_identifier = cypher_ast_node_pattern_get_identifier(node);
		}

		// If this entity was labeled, ensure that it has not been projected from a WITH clause
		if(ast_identifier) {
			const char *identifier = cypher_ast_identifier_get_name(ast_identifier);
			if(TrieMap_Find(projections, (char *)identifier, strlen(identifier)) != TRIEMAP_NOTFOUND) {
				asprintf(reason, "Reusing the WITH projection '%s' in another pattern is currently not supported",
						 identifier);
				return AST_INVALID;
			}
		}
	}

	return AST_VALID;
}

static AST_Validation _Validate_WITH_Clauses(const AST *ast, char **reason) {
	// Verify that projected entities (nodes and edges) are not used in
	// later path patterns.
	// TODO The QueryGraph should be extended to allow this in the future.

	// Retrieve the indices of each WITH clause to properly set the bounds of each scope.
	// If the query does not have a WITH clause, there is only one scope.
	uint end_offset;
	uint start_offset = 0;
	uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
	if(with_clause_count == 0) return AST_VALID;

	TrieMap *with_projections = NewTrieMap();
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	AST_Validation res = AST_VALID;

	// Visit all clauses in order
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_WITH) {
			// Collect projected aliases from WITH clauses into the same triemap
			_AST_GetWithAliases(clause, with_projections);
		} else if(type == CYPHER_AST_MATCH) {
			// Verify that no path in a MATCH pattern reuses a WITH projection
			const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
			uint path_count = cypher_ast_pattern_npaths(pattern);
			for(uint j = 0; j < path_count; j ++) {
				const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
				res = _ValidateWithEntitiesOnPath(path, with_projections, reason);
				if(res == AST_INVALID) break;
			}
		} else if(type == CYPHER_AST_CREATE) {
			// Verify that no path in a CREATE pattern reuses a WITH projection
			const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
			uint path_count = cypher_ast_pattern_npaths(pattern);
			for(uint j = 0; j < path_count; j ++) {
				const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
				res = _ValidateWithEntitiesOnPath(path, with_projections, reason);
				if(res == AST_INVALID) break;
			}

		} else if(type == CYPHER_AST_MERGE) {
			// Verify that the single path in a MERGE clause doesn't reuse a WITH projection
			const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
			res = _ValidateWithEntitiesOnPath(path, with_projections, reason);
			if(res == AST_INVALID) break;
		}
	}

	TrieMap_Free(with_projections, TrieMap_NOP_CB);
	return res;
}

static AST_Validation _Validate_MERGE_Clauses(const AST *ast, char **reason) {
	const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
	if(merge_clauses == NULL) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint merge_count = array_len(merge_clauses);
	for(uint i = 0; i < merge_count; i ++) {
		const cypher_astnode_t *merge_clause = merge_clauses[i];
		const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);
		uint nelems = cypher_ast_pattern_path_nelements(path);
		// Check every relation (each odd index in the path) to verify that
		// exactly one reltype is specified.
		for(uint j = 1; j < nelems; j += 2) {
			const cypher_astnode_t *rel = cypher_ast_pattern_path_get_element(path, j);
			if(cypher_ast_rel_pattern_nreltypes(rel) != 1) {
				asprintf(reason,
						 "Exactly one relationship type must be specified for each relation in a MERGE pattern.");
				res = AST_INVALID;
				goto cleanup;
			}
		}

		// Verify that any filters on the path refer to constants rather than parameters
		res = _ValidateInlinedPropertiesOnPath(path, reason);
		if(res != AST_VALID) goto cleanup;
	}

cleanup:
	array_free(merge_clauses);
	return res;
}

// Validate that each relation pattern is typed.
static AST_Validation _Validate_CREATE_Clause_TypedRelations(const cypher_astnode_t *node) {
	uint child_count = cypher_astnode_nchildren(node);

	// Make sure relation pattern has a single relation type.
	if(cypher_astnode_type(node) == CYPHER_AST_REL_PATTERN) {
		uint relation_type_count = 0;
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
			if(cypher_astnode_type(child) == CYPHER_AST_RELTYPE) relation_type_count++;
		}
		if(relation_type_count != 1) return AST_INVALID;
	} else {
		// Keep scanning for relation pattern nodes.
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
			if(_Validate_CREATE_Clause_TypedRelations(child) == AST_INVALID) return AST_INVALID;
		}
	}

	return AST_VALID;
}

static AST_Validation _Validate_CREATE_Clause_Properties(const cypher_astnode_t *clause,
														 char **reason) {
	const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		if(_ValidateInlinedPropertiesOnPath(path, reason) != AST_VALID) return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _Validate_CREATE_Clauses(const AST *ast, char **reason) {
	const cypher_astnode_t **create_clauses = AST_GetClauses(ast, CYPHER_AST_CREATE);
	if(!create_clauses) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint clause_count = array_len(create_clauses);
	for(uint i = 0; i < clause_count; i++) {
		if(_Validate_CREATE_Clause_TypedRelations(create_clauses[i]) == AST_INVALID) {
			asprintf(reason, "Exactly one relationship type must be specified for CREATE");
			res = AST_INVALID;
			goto cleanup;
		}
		// Validate that inlined properties do not use parameters
		res = _Validate_CREATE_Clause_Properties(create_clauses[i], reason);
		if(res == AST_INVALID) goto cleanup;
	}

cleanup:
	array_free(create_clauses);
	return res;
}

static AST_Validation _Validate_DELETE_Clauses(const AST *ast, char **reason) {
	const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
	if(!delete_clause) return AST_VALID;

	const cypher_astnode_t *match_clause = AST_GetClause(ast, CYPHER_AST_MATCH);
	if(!match_clause) return AST_INVALID;

	return AST_VALID;
}

// Verify that we handle the projected types. An example of an unsupported projection is:
// RETURN count(a) > 0
static AST_Validation _Validate_ReturnedTypes(const cypher_astnode_t *return_clause,
											  char **reason) {
	uint projection_count = cypher_ast_return_nprojections(return_clause);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);
		cypher_astnode_type_t type = cypher_astnode_type(expr);
		if(type == CYPHER_AST_COMPARISON) {
			asprintf(reason, "RedisGraph does not currently support returning '%s'",
					 cypher_astnode_typestr(type));
			return AST_INVALID;
		} else if(type == CYPHER_AST_UNARY_OPERATOR) {
			const cypher_operator_t *oper = cypher_ast_unary_operator_get_operator(expr);
			if(oper == CYPHER_OP_NOT ||
			   oper == CYPHER_OP_IS_NULL ||
			   oper == CYPHER_OP_IS_NOT_NULL
			  ) {
				// TODO weird that we can't print operator strings?
				asprintf(reason, "RedisGraph does not currently support returning this unary operator.");
				return AST_INVALID;
			}
		} else if(type == CYPHER_AST_BINARY_OPERATOR) {
			const cypher_operator_t *oper = cypher_ast_binary_operator_get_operator(expr);
			if(oper == CYPHER_OP_OR ||
			   oper == CYPHER_OP_AND ||
			   oper == CYPHER_OP_EQUAL ||
			   oper == CYPHER_OP_NEQUAL ||
			   oper == CYPHER_OP_LT ||
			   oper == CYPHER_OP_GT ||
			   oper == CYPHER_OP_LTE ||
			   oper == CYPHER_OP_GTE) {
				// TODO weird that we can't print operator strings?
				asprintf(reason, "RedisGraph does not currently support returning this binary operator.");
				return AST_INVALID;
			}
		}
	}
	return AST_VALID;
}

static AST_Validation _Validate_RETURN_Clause(const AST *ast, char **reason) {
	const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	if(!return_clause) return AST_VALID;

	// Verify the types of RETURN projections
	if(_Validate_ReturnedTypes(return_clause, reason) != AST_VALID) return AST_INVALID;

	// Retrieve all user-specified functions in RETURN clause.
	TrieMap *referred_funcs = NewTrieMap();
	AST_ReferredFunctions(return_clause, referred_funcs);

	// Verify that referred functions exist.
	bool include_aggregates = true;
	AST_Validation res = _ValidateReferredFunctions(referred_funcs, reason, include_aggregates);
	TrieMap_Free(referred_funcs, TrieMap_NOP_CB);

	return res;
}

static AST_Validation _Validate_UNWIND_Clauses(const AST *ast, char **reason) {
	const cypher_astnode_t **unwind_clauses = AST_GetClauses(ast, CYPHER_AST_UNWIND);
	if(!unwind_clauses) return AST_VALID;

	AST_Validation res = AST_VALID;
	uint clause_count = array_len(unwind_clauses);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *expression = cypher_ast_unwind_get_expression(unwind_clauses[i]);
		cypher_astnode_type_t type = cypher_astnode_type(expression);
		// Ensure that the UNWIND expression is a collection (aka list/array)
		if(type != CYPHER_AST_COLLECTION) {
			asprintf(reason, "UNWIND expects a list argument; encountered ''%s'", cypher_astnode_typestr(type));
			res = AST_INVALID;
			goto cleanup;
		}
		// Verify that all elements of the UNWIND collection are supported by RedisGraph
		uint child_count = cypher_astnode_nchildren(expression);
		for(uint j = 0; j < child_count; j ++) {
			res = CypherWhitelist_ValidateQuery(cypher_astnode_get_child(expression, j), reason);
			if(res != AST_VALID) goto cleanup;
		}
	}

cleanup:
	array_free(unwind_clauses);
	return res;
}

// LIMIT and SKIP are not independent clauses, but modifiers that can be applied to WITH or RETURN clauses
static AST_Validation _Validate_LIMIT_SKIP_Modifiers(const AST *ast, char **reason) {
	// Handle modifiers on the RETURN clause
	const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	// Skip check if the RETURN clause does not specify a limit
	if(return_clause) {
		// Handle LIMIT modifier
		const cypher_astnode_t *limit = cypher_ast_return_get_limit(return_clause);
		if(limit) {
			// Handle non-integer types specified as LIMIT value
			if(cypher_astnode_type(limit) != CYPHER_AST_INTEGER) {
				asprintf(reason, "LIMIT specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}

			// Handle LIMIT strings that cannot be fully converted to integers,
			// due to size or invalid characters
			const char *value_str = cypher_ast_integer_get_valuestr(limit);
			if(_ValidatePositiveInteger(value_str) != AST_VALID) {
				asprintf(reason,
						 "LIMIT specified value '%s', must be a positive integer in the signed 8-byte range.", value_str);
				return AST_INVALID;
			}
		}

		// Handle SKIP modifier
		const cypher_astnode_t *skip = cypher_ast_return_get_skip(return_clause);
		if(skip) {
			// Handle non-integer types specified as skip value
			if(cypher_astnode_type(skip) != CYPHER_AST_INTEGER) {
				asprintf(reason, "SKIP specified value of invalid type, must be a positive integer");
				return AST_INVALID;
			}

			// Handle skip strings that cannot be fully converted to integers,
			// due to size or invalid characters
			const char *value_str = cypher_ast_integer_get_valuestr(skip);
			if(_ValidatePositiveInteger(value_str) != AST_VALID) {
				asprintf(reason,
						 "SKIP specified value '%s', must be a positive integer in the signed 8-byte range.", value_str);
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
			// Handle non-integer types specified as LIMIT value
			if(cypher_astnode_type(limit) != CYPHER_AST_INTEGER) {
				asprintf(reason, "LIMIT specified value of invalid type, must be a positive integer");
				res = AST_INVALID;
				break;
			}

			// Handle LIMIT strings that cannot be fully converted to integers,
			// due to size or invalid characters
			const char *value_str = cypher_ast_integer_get_valuestr(limit);
			if(_ValidatePositiveInteger(value_str) != AST_VALID) {
				asprintf(reason,
						 "LIMIT specified value '%s', must be a positive integer in the signed 8-byte range.", value_str);
				res = AST_INVALID;
				break;
			}
		}

		// Handle SKIP modifier
		const cypher_astnode_t *skip = cypher_ast_with_get_skip(with_clause);
		if(skip) {
			// Handle non-integer types specified as skip value
			if(cypher_astnode_type(skip) != CYPHER_AST_INTEGER) {
				asprintf(reason, "SKIP specified value of invalid type, must be a positive integer");
				res = AST_INVALID;
				break;
			}

			// Handle skip strings that cannot be fully converted to integers,
			// due to size or invalid characters
			const char *value_str = cypher_ast_integer_get_valuestr(skip);
			if(_ValidatePositiveInteger(value_str) != AST_VALID) {
				asprintf(reason,
						 "SKIP specified value '%s', must be a positive integer in the signed 8-byte range.", value_str);
				res = AST_INVALID;
				break;
			}
		}
	}
	array_free(with_clauses);

	return res;
}

// A query must end in a RETURN clause, a procedure, or an updating clause
// (CREATE, MERGE, DELETE, SET, or REMOVE once supported)
static AST_Validation _ValidateQueryTermination(const AST *ast, char **reason) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
	cypher_astnode_type_t type = cypher_astnode_type(last_clause);
	if(type != CYPHER_AST_RETURN   &&
	   type != CYPHER_AST_CREATE   &&
	   type != CYPHER_AST_MERGE    &&
	   type != CYPHER_AST_DELETE   &&
	   type != CYPHER_AST_SET      &&
	   type != CYPHER_AST_CALL
	  ) {
		asprintf(reason, "Query cannot conclude with %s (must be RETURN or an update clause)",
				 cypher_astnode_typestr(type));
		return AST_INVALID;
	}
	return AST_VALID;

}

static void _AST_GetDefinedIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
	if(!node) return;
	cypher_astnode_type_t type = cypher_astnode_type(node);
	if(type == CYPHER_AST_RETURN) {
		// Only collect aliases (which may be referenced in an ORDER BY)
		// from the RETURN clause, rather than all identifiers
		_AST_GetReturnAliases(node, identifiers);
	} else if(type == CYPHER_AST_WITH) {
		// Get alias if one is provided; otherwise use the expression identifier
		_AST_GetWithAliases(node, identifiers);
	} else if(type == CYPHER_AST_MERGE || type == CYPHER_AST_UNWIND || type == CYPHER_AST_MATCH ||
			  type == CYPHER_AST_CREATE) {
		_AST_GetIdentifiers(node, identifiers);
	} else {
		uint child_count = cypher_astnode_nchildren(node);
		for(uint c = 0; c < child_count; c ++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
			_AST_GetDefinedIdentifiers(child, identifiers);
		}
	}
}

static void _AST_GetReferredIdentifiers(const cypher_astnode_t *node, TrieMap *identifiers) {
	if(!node) return;
	cypher_astnode_type_t type = cypher_astnode_type(node);
	if(type == CYPHER_AST_SET || type == CYPHER_AST_RETURN || type == CYPHER_AST_DELETE ||
	   type == CYPHER_AST_WITH) {
		_AST_GetIdentifiers(node, identifiers);
	} else {
		uint child_count = cypher_astnode_nchildren(node);
		for(uint c = 0; c < child_count; c ++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
			_AST_GetReferredIdentifiers(child, identifiers);
		}
	}
}

/* Check that all referred identifiers been defined in the same AST scope. */
static AST_Validation _Validate_Aliases_DefinedInScope(const AST *ast, uint start_offset,
													   uint end_offset, char **undefined_alias) {
	AST_Validation res = AST_VALID;
	TrieMap *defined_aliases = NewTrieMap();
	TrieMap *referred_identifiers = NewTrieMap();

	for(uint i = start_offset; i < end_offset; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		// Get defined identifiers.
		_AST_GetDefinedIdentifiers(clause, defined_aliases);
		// Get referred identifiers except from the first clause in the scope,
		// which cannot introduce aliases but not contain references.
		if(i != start_offset) _AST_GetReferredIdentifiers(clause, referred_identifiers);
	}

	char *alias;
	tm_len_t len;
	void *value;
	TrieMapIterator *it = TrieMap_Iterate(referred_identifiers, "", 0);

	// See that each referred identifier is defined.
	while(TrieMapIterator_Next(it, &alias, &len, &value)) {
		if(TrieMap_Find(defined_aliases, alias, len) == TRIEMAP_NOTFOUND) {
			asprintf(undefined_alias, "%s not defined", alias);
			res = AST_INVALID;
			break;
		}
	}

	// Clean up:
	TrieMapIterator_Free(it);
	TrieMap_Free(defined_aliases, TrieMap_NOP_CB);
	TrieMap_Free(referred_identifiers, TrieMap_NOP_CB);
	return res;
}

/* Check that all referred identifiers been defined. */
static AST_Validation _Validate_Aliases_Defined(const AST *ast, char **undefined_alias) {
	AST_Validation res = AST_VALID;

	// Retrieve the indices of each WITH clause to properly set the bounds of each scope.
	// If the query does not have a WITH clause, there is only one scope.
	uint end_offset;
	uint start_offset = 0;
	uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
	if(with_clause_count > 0) {
		uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
		for(uint i = 0; i < with_clause_count; i++) {
			end_offset = segment_indices[i];
			res = _Validate_Aliases_DefinedInScope(ast, start_offset, end_offset, undefined_alias);
			if(res != AST_VALID) break;
			start_offset = end_offset; // Update the start offset for the next scope
		}
		array_free(segment_indices);
		if(res != AST_VALID) return res;  // Return early if we've encountered an error
	}

	end_offset = cypher_ast_query_nclauses(ast->root);
	return _Validate_Aliases_DefinedInScope(ast, start_offset, end_offset, undefined_alias);
}

// Report encountered errors by libcypher-parser.
static char *_AST_ReportErrors(const cypher_parse_result_t *result) {
	char *errorMsg;
	uint nerrors = cypher_parse_result_nerrors(result);
	// We are currently only reporting the first error to simplify the response.
	if(nerrors > 1) nerrors = 1;
	for(uint i = 0; i < nerrors; i++) {
		const cypher_parse_error_t *error = cypher_parse_result_get_error(result, i);

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

		asprintf(&errorMsg, "errMsg: %s line: %u, column: %u, offset: %zu errCtx: %s errCtxOffset: %zu",
				 errMsg, errPos.line, errPos.column, errPos.offset, errCtx, errCtxOffset);
	}
	return errorMsg;
}

static AST_Validation _ValidateMaps(const cypher_astnode_t *root, char **reason) {
	if(!root) return AST_VALID;

	cypher_astnode_type_t type = cypher_astnode_type(root);

	if(type == CYPHER_AST_REL_PATTERN || type == CYPHER_AST_NODE_PATTERN) return AST_VALID;

	if(type == CYPHER_AST_MAP) {
		asprintf(reason, "Maps are not currently supported outside of node and relation patterns.");
		return AST_INVALID;
	}

	uint child_count = cypher_astnode_nchildren(root);
	for(uint i = 0; i < child_count; i++) {
		if(_ValidateMaps(cypher_astnode_get_child(root, i), reason) != AST_VALID) return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _ValidateClauses(const AST *ast, char **reason) {
	if(_Validate_MATCH_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_WITH_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_MERGE_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_CREATE_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_DELETE_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_RETURN_Clause(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_UNWIND_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_LIMIT_SKIP_Modifiers(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_ValidateQueryTermination(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_Validate_Aliases_Defined(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	if(_ValidateMaps(ast->root, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	return AST_VALID;
}

// Checks to see if libcypher-parser reported any errors.
bool AST_ContainsErrors(const cypher_parse_result_t *result) {
	return cypher_parse_result_nerrors(result) > 0;
}

AST_Validation AST_Validate(RedisModuleCtx *ctx, const cypher_parse_result_t *result) {
	// Check for failures in libcypher-parser
	if(AST_ContainsErrors(result)) {
		char *errMsg = _AST_ReportErrors(result);
		RedisModule_Log(ctx, "debug", "Error parsing query: %s", errMsg);
		RedisModule_ReplyWithError(ctx, errMsg);
		free(errMsg);
		return AST_INVALID;
	}

	const cypher_astnode_t *root = cypher_parse_result_get_root(result, 0);
	// Check for empty query
	if(root == NULL) {
		RedisModule_ReplyWithError(ctx, "Error: empty query.");
		return AST_INVALID;
	}

	char *reason;
	// Verify that the query does not contain any expressions not in the RedisGraph support whitelist
	if(CypherWhitelist_ValidateQuery(root, &reason) != AST_VALID) {
		// Unsupported expressions found; reply with error.
		RedisModule_ReplyWithError(ctx, reason);
		free(reason);
		return AST_INVALID;
	}

	cypher_astnode_type_t root_type = cypher_astnode_type(root);
	if(root_type != CYPHER_AST_STATEMENT) {
		// This should be unnecessary, as we're currently parsing
		// with the CYPHER_PARSE_ONLY_STATEMENTS flag.
		asprintf(&reason, "Encountered unsupported query type '%s'", cypher_astnode_typestr(root_type));
		RedisModule_ReplyWithError(ctx, reason);
		free(reason);
		return AST_INVALID;
	}

	const cypher_astnode_t *body = cypher_ast_statement_get_body(root);
	cypher_astnode_type_t body_type = cypher_astnode_type(body);
	if(body_type == CYPHER_AST_CREATE_NODE_PROP_INDEX ||
	   body_type == CYPHER_AST_DROP_NODE_PROP_INDEX) {
		// Index operation; validations are handled elsewhere.
		return AST_VALID;
	}

	AST_Validation res = AST_VALID;
	// TODO either merge this all with AST_Build
	// or modify these calls to accept a cypher_astnode
	AST ast;
	ast.root = body;
	// Check for invalid queries not captured by libcypher-parser
	res = _ValidateClauses(&ast, &reason);
	if(res != AST_VALID) {
		RedisModule_ReplyWithError(ctx, reason);
		free(reason);
	}

	return res;
}
