/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "ast_shared.h"
#include "../util/arr.h"
#include "cypher_whitelist.h"
#include "../procedures/procedure.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

inline static void _prepareIterateAll(rax *map, raxIterator *iter) {
	raxStart(iter, map);
	raxSeek(iter, "^", NULL, 0);
}

// Validate that an input string can be completely converted to a positive integer in range.
static inline AST_Validation _ValidatePositiveInteger(const char *input) {
	assert(input);
	char *endptr; // If the entire string is converted, endptr will point to a null byte
	errno = 0; // If underflow or overflow occurs, errno will be set

	strtol(input, &endptr, 0); // Perform conversion

	if(errno != 0 || *endptr != '\0') return AST_INVALID;

	return AST_VALID;
}

static void _AST_GetIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
	if(!node) return;
	assert(identifiers);

	if(cypher_astnode_type(node) == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(node);
		raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
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

static void _AST_GetWithAliases(const cypher_astnode_t *node, rax *aliases) {
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
		raxInsert(aliases, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}
}

// Extract identifiers / aliases from a procedure call.
static void _AST_GetProcCallAliases(const cypher_astnode_t *node, rax *identifiers) {
	// CALL db.labels() yield label
	// CALL db.labels() yield label as l
	assert(node && identifiers);
	assert(cypher_astnode_type(node) == CYPHER_AST_CALL);

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
		assert(identifiers);
		raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
	}
}

// UNWIND and WITH also form aliases, but don't need special handling for us yet.
static void _AST_GetReturnAliases(const cypher_astnode_t *node, rax *aliases) {
	assert(node && aliases && cypher_astnode_type(node) == CYPHER_AST_RETURN);

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
static AST_Validation _ValidateReferredFunctions(rax *referred_functions, char **reason,
												 bool include_aggregates) {
	AST_Validation res = AST_VALID;
	char funcName[32];
	raxIterator it;
	_prepareIterateAll(referred_functions, &it);
	*reason = NULL;

	// TODO: return RAX.
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

	raxStop(&it);
	return res;
}

static inline bool _AliasIsReturned(rax *projections, const char *identifier) {
	if(raxFind(projections, (unsigned char *)identifier, strlen(identifier)) != raxNotFound) {
		return true;
	}
	return false;
}

// If we have a multi-hop traversal (fixed or variable length), we cannot currently return that entity.
static AST_Validation _ValidateMultiHopTraversal(rax *projections, const cypher_astnode_t *edge,
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
											rax *edge_aliases, char **reason) {
	uint child_count = cypher_astnode_nchildren(node);
	for(uint i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		cypher_astnode_type_t type = cypher_astnode_type(child);

		if(type == CYPHER_AST_IDENTIFIER) {
			const char *alias = cypher_ast_identifier_get_name(child);
			int new = raxInsert(edge_aliases, (unsigned char *)alias, strlen(alias), NULL,
								NULL);
			if(!new) {
				asprintf(reason, "Cannot use the same relationship variable '%s' for multiple patterns.",
						 alias);
				return AST_INVALID;
			}
		}
	}

	return AST_VALID;
}

static AST_Validation _ValidateRelation(rax *projections, const cypher_astnode_t *edge,
										rax *edge_aliases,
										char **reason) {
	AST_Validation res = AST_VALID;

	// Make sure edge alias is unique
	res = _Validate_ReusedEdges(edge, edge_aliases, reason);
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
									rax *projections,
									rax *edge_aliases,
									char **reason) {
	AST_Validation res = AST_VALID;
	uint path_len = cypher_ast_pattern_path_nelements(path);

	// Check all relations on the path (every odd offset) and collect aliases.
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, i);
		res = _ValidateRelation(projections, edge, edge_aliases, reason);
		if(res != AST_VALID) return res;
	}

	return res;
}

static AST_Validation _ValidatePattern(rax *projections, const cypher_astnode_t *pattern,
									   rax *edge_aliases, char **reason) {
	AST_Validation res = AST_VALID;
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		res = _ValidatePath(cypher_ast_pattern_get_path(pattern, i), projections, edge_aliases, reason);
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
			// TODO if the identifier resolves to a scalar WITH projection, it should be usable,
			// but we can't currently differentiate.
			res = AST_INVALID;
			break;
		}
	}
	if(res == AST_INVALID) {
		asprintf(reason, "Identifiers cannot currently be used to set inlined properties in RedisGraph.");
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
	if(type == CYPHER_AST_PATTERN_PATH) {
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

static AST_Validation _Validate_CALL_Clauses(const AST *ast, char **reason) {
	/* Make sure procedure calls are valid:
	 * 1. procedure exists
	 * 2. number of arguments to procedure is as expected
	 * 3. yield refers to procedure output */
	AST_Validation res = AST_VALID;
	ProcedureCtx *proc = NULL;
	rax *identifiers = raxNew();

	const cypher_astnode_t **call_clauses = AST_GetClauses(ast, CYPHER_AST_CALL);
	if(call_clauses == NULL) return AST_VALID;

	uint call_count = array_len(call_clauses);
	for(uint i = 0; i < call_count; i ++) {
		const cypher_astnode_t *call_clause = call_clauses[i];

		// Make sure procedure exists.
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		proc = Proc_Get(proc_name);

		if(proc == NULL) {
			asprintf(reason, "Procedure `%s` is not registered", proc_name);
			res = AST_INVALID;
			goto cleanup;
		}

		// Validate num of arguments.
		if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) {
			unsigned int given_arg_count = cypher_ast_call_narguments(call_clause);
			if(Procedure_Argc(proc) != given_arg_count) {
				asprintf(reason, "Procedure `%s` requires %d arguments, got %d", proc_name, proc->argc,
						 given_arg_count);
				res = AST_INVALID;
				goto cleanup;
			}
		}

		// Validate projections.
		uint proj_count = cypher_ast_call_nprojections(call_clause);
		if(proj_count > 0) {
			// Collect call projections.
			for(uint j = 0; j < proj_count; j++) {
				const cypher_astnode_t *proj = cypher_ast_call_get_projection(call_clause, j);
				const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(proj);
				assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
				const char *identifier = cypher_ast_identifier_get_name(ast_exp);
				// Make sure each yield output is mentioned only once.
				if(!raxInsert(identifiers, (unsigned char *)identifier, strlen(identifier), NULL,
							  NULL)) {
					asprintf(reason, "Variable `%s` already declared", identifier);
					res = AST_INVALID;
					goto cleanup;
				}
			}

			// Make sure procedure is aware of each output.
			char output[256];
			raxIterator it;
			_prepareIterateAll(identifiers, &it);

			while(raxNext(&it)) {
				size_t len = it.key_len;
				unsigned char *identifier = it.key;
				if(len >= 256) {
					asprintf(reason, "Output name `%s` too long", identifier);
					res = AST_INVALID;
					goto cleanup;
				}

				memcpy(output, identifier, len);
				output[len] = 0;
				if(!Procedure_ContainsOutput(proc, output)) {
					raxStop(&it);
					asprintf(reason, "Procedure `%s` does not yield output `%s`", proc_name, output);
					res = AST_INVALID;
					goto cleanup;
				}
			}

			raxStop(&it);
			raxFree(identifiers);
			identifiers = NULL;
		}

		Proc_Free(proc);
		proc = NULL;
	}

cleanup:
	if(proc) Proc_Free(proc);
	array_free(call_clauses);
	if(identifiers) raxFree(identifiers);
	return res;
}

static AST_Validation _ValidateNodeAlias(const cypher_astnode_t *node, rax *edge_aliases,
										 char **reason) {
	const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(node);
	if(ast_alias == NULL) return AST_VALID;

	// Verify that the node's alias is not in the map of edge aliases.
	const char *alias = cypher_ast_identifier_get_name(ast_alias);
	if(raxFind(edge_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
		asprintf(reason, "The alias '%s' was specified for both a node and a relationship.", alias);
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _ValidateReusedAliases(rax *edge_aliases,
											 const cypher_astnode_t *pattern,
											 char **reason) {
	AST_Validation res = AST_VALID;
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		uint path_len = cypher_ast_pattern_path_nelements(path);
		for(uint j = 0; j < path_len; j += 2) {
			const cypher_astnode_t *node = cypher_ast_pattern_path_get_element(path, j);
			res = _ValidateNodeAlias(node, edge_aliases, reason);
			if(res != AST_VALID) return res;
		}
	}

	return res;
}

static AST_Validation _Validate_MATCH_Clauses(const AST *ast, char **reason) {
	// Check to see if all mentioned inlined, outlined functions exists.
	// Inlined functions appear within entity definition ({a:v})
	// Outlined functions appear within the WHERE clause.
	// libcypher-parser doesn't have a WHERE node, all of the filters
	// are specified within the MATCH node sub-tree.
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	if(match_clauses == NULL) return AST_VALID;

	rax *referred_funcs = raxNew();
	rax *edge_aliases = raxNew();
	rax *reused_entities = raxNew();
	AST_Validation res;

	const cypher_astnode_t *return_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	rax *projections = _AST_GetReturnProjections(return_clause);
	uint match_count = array_len(match_clauses);
	for(uint i = 0; i < match_count; i ++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		// Validate the pattern described by the MATCH clause
		res = _ValidatePattern(projections, cypher_ast_match_get_pattern(match_clause), edge_aliases,
							   reason);
		if(res == AST_INVALID) goto cleanup;

		// Validate that inlined filters do not use parameters
		res = _Validate_MATCH_Clause_Filters(match_clause, reason);
		if(res == AST_INVALID) goto cleanup;

		// Collect function references
		AST_ReferredFunctions(match_clause, referred_funcs);
	}

	// Verify that no relation alias is also used to denote a node
	for(uint i = 0; i < match_count; i ++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		res = _ValidateReusedAliases(edge_aliases, cypher_ast_match_get_pattern(match_clause), reason);
		if(res == AST_INVALID) goto cleanup;
	}

	// Verify that referred functions exist.
	bool include_aggregates = false;
	res = _ValidateReferredFunctions(referred_funcs, reason, include_aggregates);

cleanup:
	if(projections) raxFree(projections);
	raxFree(referred_funcs);
	raxFree(edge_aliases);
	raxFree(reused_entities);
	array_free(match_clauses);

	return res;
}

static AST_Validation _ValidateWithEntitiesOnPath(const cypher_astnode_t *path,
												  rax *projections, char **reason) {
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
			if(raxFind(projections, (unsigned char *)identifier, strlen(identifier)) != raxNotFound) {
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

	rax *with_projections = raxNew();
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

	raxFree(with_projections);
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
		if(type == CYPHER_AST_UNARY_OPERATOR) {
			const cypher_operator_t *oper = cypher_ast_unary_operator_get_operator(expr);
			if(oper == CYPHER_OP_IS_NULL ||
			   oper == CYPHER_OP_IS_NOT_NULL
			  ) {
				// TODO weird that we can't print operator strings?
				asprintf(reason, "RedisGraph does not currently support returning this unary operator.");
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
	rax *referred_funcs = raxNew();
	AST_ReferredFunctions(return_clause, referred_funcs);

	// Verify that referred functions exist.
	bool include_aggregates = true;
	AST_Validation res = _ValidateReferredFunctions(referred_funcs, reason, include_aggregates);
	raxFree(referred_funcs);

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
		if(type == CYPHER_AST_APPLY_OPERATOR) {
			const cypher_astnode_t *funcNode =  cypher_ast_apply_operator_get_func_name(expression);
			const char *funcName = cypher_ast_function_name_get_value(funcNode);
			// function name is NOT range
			if(strcasecmp(funcName, "range")) {
				asprintf(reason, "UNWIND expects range funcition; encountered '%s'", funcName);
				res = AST_INVALID;
				goto cleanup;
			}
		} else if(!(type == CYPHER_AST_COLLECTION || type == CYPHER_AST_IDENTIFIER ||
					type == CYPHER_AST_PROPERTY_OPERATOR)) {
			asprintf(reason, "UNWIND expects a list argument or an list identifier; encountered ''%s'",
					 cypher_astnode_typestr(type));
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

static void _AST_RegisterCallOutputs(const cypher_astnode_t *call_clause, rax *identifiers) {
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
	ProcedureCtx *proc = Proc_Get(proc_name);
	assert(proc);

	unsigned int output_count = array_len(proc->output);
	for(uint i = 0; i < output_count; i++) {
		const char *name = proc->output[i]->name;
		raxInsert(identifiers, (unsigned char *)name, strlen(name), NULL, NULL);
	}
}

// Perform validations not constrained to a specific scope
static AST_Validation _ValidateQuerySequence(const AST *ast, char **reason) {
	// Validate the final clause
	if(_ValidateQueryTermination(ast, reason) != AST_VALID) return AST_INVALID;

	// Verify that no intermediate clause is a RETURN
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count - 1; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(clause) == CYPHER_AST_RETURN) {
			asprintf(reason, "RETURN must be the final clause in a query.");
			return AST_INVALID;
		}
	}

	return AST_VALID;
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
	} else if(type == CYPHER_AST_MERGE ||
			  type == CYPHER_AST_UNWIND ||
			  type == CYPHER_AST_MATCH ||
			  type == CYPHER_AST_CREATE) {
		_AST_GetIdentifiers(node, identifiers);
	} else if(type == CYPHER_AST_CALL) {
		_AST_RegisterCallOutputs(node, identifiers);
	} else {
		uint child_count = cypher_astnode_nchildren(node);
		for(uint c = 0; c < child_count; c ++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(node, c);
			_AST_GetDefinedIdentifiers(child, identifiers);
		}
	}
}

static void _AST_GetReferredIdentifiers(const cypher_astnode_t *node, rax *identifiers) {
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
	rax *defined_aliases = raxNew();
	rax *referred_identifiers = raxNew();

	for(uint i = start_offset; i < end_offset; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		// Get defined identifiers.
		_AST_GetDefinedIdentifiers(clause, defined_aliases);
		// Get referred identifiers except from the first clause in the scope,
		// which can introduce aliases but not contain references.
		if(i != start_offset) _AST_GetReferredIdentifiers(clause, referred_identifiers);
	}

	raxIterator it;
	_prepareIterateAll(referred_identifiers, &it);

	// See that each referred identifier is defined.
	while(raxNext(&it)) {
		int len = it.key_len;
		unsigned char *alias = it.key;
		if(raxFind(defined_aliases, alias, len) == raxNotFound) {
			asprintf(undefined_alias, "%.*s not defined", len, alias);
			res = AST_INVALID;
			break;
		}
	}

	// Clean up:
	raxStop(&it);
	raxFree(defined_aliases);
	raxFree(referred_identifiers);
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

static AST_Validation _validateList(const cypher_astnode_t *root, char **reason) {
	const cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == CYPHER_AST_APPLY_OPERATOR) {
		const cypher_astnode_t *funcNode =  cypher_ast_apply_operator_get_func_name(root);
		const char *funcName = cypher_ast_function_name_get_value(funcNode);
		// function name is NOT range
		if(strcasecmp(funcName, "range")) {
			asprintf(reason, "subscript index access expects range funcition; encountered '%s'", funcName);
			return AST_INVALID;
		}
		uint narguments = cypher_ast_apply_operator_narguments(root);
		if(narguments < 2 || narguments > 3) {
			asprintf(reason, "range function expects 2 or 3 argumetns; encountered %d", narguments);
			return AST_INVALID;
		}
		for(uint i = 0; i < narguments; i ++) {
			const cypher_astnode_t *argument = cypher_ast_apply_operator_get_argument(root, i);
			const cypher_astnode_type_t argument_type = cypher_astnode_type(argument);
			if(argument_type != CYPHER_AST_INTEGER || argument_type != CYPHER_AST_IDENTIFIER) {
				asprintf(reason, "expected interger or identifier; encountered %s",
						 cypher_astnode_typestr(argument_type));
				return AST_INVALID;
			}
		}
	} else if(type != CYPHER_AST_COLLECTION && type != CYPHER_AST_IDENTIFIER) {
		asprintf(reason, "subscript index access expects a list or an identifier; encountered %s",
				 cypher_astnode_typestr(type));
		return AST_INVALID;
	}
	return AST_VALID;
}

static AST_Validation _validateIndex(const cypher_astnode_t *root, char **reason) {
	const cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type != CYPHER_AST_INTEGER || type != CYPHER_AST_IDENTIFIER) {
		asprintf(reason, "subscript index must be an interger or an identifier");
		return AST_INVALID;
	}
	return AST_VALID;
}

static AST_Validation _validateSubscriptOps(const cypher_astnode_t *root, char **reason) {
	if(!root) return AST_VALID;

	const cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == CYPHER_AST_SUBSCRIPT_OPERATOR) {
		const cypher_astnode_t *exp_node = cypher_ast_subscript_operator_get_expression(root);
		if(_validateList(exp_node, reason) != AST_VALID) return AST_INVALID;

		const cypher_astnode_t *subscript_node = cypher_ast_subscript_operator_get_subscript(root);
		if(_validateIndex(subscript_node, reason) != AST_VALID) return AST_INVALID;
	}

	if(type == CYPHER_AST_SLICE_OPERATOR) {
		const cypher_astnode_t *exp_node = cypher_ast_slice_operator_get_expression(root);
		if(_validateList(exp_node, reason) != AST_VALID) return AST_INVALID;

		const cypher_astnode_t *start_node = cypher_ast_slice_operator_get_start(root);
		if(start_node)
			if(_validateIndex(start_node, reason) != AST_VALID) return AST_INVALID;

		const cypher_astnode_t *end_node = cypher_ast_slice_operator_get_end(root);
		if(end_node)
			if(_validateIndex(end_node, reason) != AST_VALID) return AST_INVALID;
	}

	uint child_count = cypher_astnode_nchildren(root);
	for(uint i = 0; i < child_count; i++) {
		if(_validateSubscriptOps(cypher_astnode_get_child(root, i),
								 reason) != AST_VALID) return AST_INVALID;
	}

	return AST_VALID;
}

static AST_Validation _ValidateClauses(const AST *ast, char **reason) {
	if(_Validate_CALL_Clauses(ast, reason) == AST_INVALID) {
		return AST_INVALID;
	}

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

	if(_ValidateMaps(ast->root, reason) == AST_INVALID) {
		return AST_INVALID;
	}

	return AST_VALID;
}

static AST *_NewMockASTSegment(const cypher_astnode_t *root, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->free_root = true;
	ast->entity_map = NULL;

	uint n = end_offset - start_offset;

	cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_query_get_clause(root, i + start_offset);
	}
	struct cypher_input_range range = {};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n, NULL, 0, range);

	return ast;
}

static AST_Validation _ValidateScopes(const cypher_astnode_t *root, char **reason) {
	AST_Validation res = AST_VALID;

	AST mock_ast; // Build a fake AST with the correct AST root
	mock_ast.root = root;

	// Verify that the RETURN clause and terminating clause do not violate scoping rules.
	if(_ValidateQuerySequence(&mock_ast, reason) != AST_VALID) return AST_INVALID;

	// Validate identifiers, which may be passed between scopes
	if(_Validate_Aliases_Defined(&mock_ast, reason) == AST_INVALID) return AST_INVALID;

	// Aliases are scoped by the WITH clauses within the query.
	// If we have one or more WITH clauses, MATCH validations should be performed one scope at a time.
	uint *query_scopes = AST_GetClauseIndices(&mock_ast, CYPHER_AST_WITH);
	uint with_clause_count = array_len(query_scopes);

	// Query has only one scope, no need to create sub-ASTs
	if(with_clause_count == 0) {
		res = _ValidateClauses(&mock_ast, reason);
		goto cleanup;
	}

	AST *scoped_ast;
	uint scope_end;
	uint scope_start = 0;
	for(uint i = 0; i < with_clause_count; i ++) {
		scope_end = query_scopes[i] + 1; // Switching from index to bound, so add 1
		// Make a sub-AST containing only the clauses in this scope
		scoped_ast = _NewMockASTSegment(root, scope_start, scope_end);

		// Perform validations
		res = _ValidateClauses(scoped_ast, reason);
		AST_Free(scoped_ast);
		if(res != AST_VALID) goto cleanup;
		// Update the starting indices of the scope for the next iteration.
		scope_start = scope_end;
	}

	// Build and test the final scope (from the last WITH to the last clause)
	scope_end = cypher_ast_query_nclauses(root);
	scoped_ast = _NewMockASTSegment(root, scope_start, scope_end);
	res = _ValidateClauses(scoped_ast, reason);
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
	if(body_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
	   body_type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		// Index operation; validations are handled elsewhere.
		return AST_VALID;
	}

	// Check for invalid queries not captured by libcypher-parser
	AST_Validation res = _ValidateScopes(body, &reason);
	if(res != AST_VALID) {
		RedisModule_ReplyWithError(ctx, reason);
		free(reason);
	}

	return res;
}
