/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../RG.h"
#include "../errors.h"
#include "ast_shared.h"
#include "ast_visitor.h"
#include "../util/arr.h"
#include "cypher_whitelist.h"
#include "../util/rax_extensions.h"
#include "../procedures/procedure.h"
#include "../arithmetic/arithmetic_expression.h"

typedef struct
{
	rax *defined_identifiers;
	cypher_astnode_type_t clause;
	AST_Validation valid;
} validations_ctx;

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
		// allShortestPaths is invalid in the match predicate
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

static void _AST_GetWithAliases
(
	const cypher_astnode_t *node,
	rax *aliases
) {
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

// Extract identifiers / aliases from a procedure call.
static void _AST_GetProcCallAliases
(
	const cypher_astnode_t *node,
	rax *identifiers
) {
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

// If we have a multi-hop traversal (fixed or variable length), we cannot currently return that entity.
static AST_Validation _ValidateMultiHopTraversal
(
	const cypher_astnode_t *edge,
	const cypher_astnode_t *range
) {
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

	// Check if relation has an alias
	const cypher_astnode_t *ast_identifier = cypher_ast_rel_pattern_get_identifier(edge);
	if(!ast_identifier) return AST_VALID;

	const char *identifier = cypher_ast_identifier_get_name(ast_identifier);
	ErrorCtx_SetError("RedisGraph does not support alias of variable-length traversal edges '%s'. \
	Instead, use a query in the style of: 'MATCH p = (a)-[*]->(b) RETURN relationships(p)'.",
						identifier);
	return AST_INVALID;
}

// Verify that MERGE doesn't redeclare bound relations and that one reltype is specified for unbound relations.
static AST_Validation _ValidateMergeRelation
(
	const cypher_astnode_t *entity,
	rax *defined_aliases
) {
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
static AST_Validation _ValidateMergeNode
(
	const cypher_astnode_t *entity,
	rax *defined_aliases
) {
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

static AST_Validation _ValidateCreateRelation
(
	const cypher_astnode_t *entity,
	rax *defined_aliases
) {
	const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(entity);
	// Validate that no relation aliases are previously bound.
	if(identifier) {
		const char *alias = cypher_ast_identifier_get_name(identifier);
		if(raxFind(defined_aliases, (unsigned char *)alias, strlen(alias)) != raxNotFound) {
			ErrorCtx_SetError("The bound variable '%s' can't be redeclared in a CREATE clause", alias);
			return AST_INVALID;
		}
	}

	return AST_VALID;
}

// Validate each entity referenced in a single path of a CREATE clause.
static AST_Validation _Validate_CREATE_Entities
(
	const cypher_astnode_t *path,
	rax *defined_aliases
) {
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

	return AST_VALID;
}

static AST_Validation _Validate_referred_identifier
(
	rax *defined_identifiers,
	const char *identifier
) {
	int len = strlen(identifier);
	if(raxFind(defined_identifiers, (unsigned char *)identifier, len) == raxNotFound) {
		ErrorCtx_SetError("%.*s not defined", len, identifier);
		return AST_INVALID;
	}

	return AST_VALID;
}

static bool _Validate_identifier
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	const char *identifier = cypher_ast_identifier_get_name(n);
	vctx->valid = _Validate_referred_identifier(vctx->defined_identifiers, identifier);

	return true;
}

static bool _Validate_map
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	uint nentries = cypher_ast_map_nentries(n);
	for (uint i = 0; i < nentries; i++) {
		const cypher_astnode_t *exp = cypher_ast_map_get_value(n, i);
		AST_Visitor_visit(exp, visitor);
	}

	return false;
}

static bool _Validate_projection
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	const cypher_astnode_t *exp = cypher_ast_projection_get_expression(n);
	AST_Visitor_visit(exp, visitor);

	return false;
}

static AST_Validation _ValidateFunctionCall
(
	const char *funcName,
	bool include_aggregates
) {
	if(!AR_FuncExists(funcName)) {
		ErrorCtx_SetError("Unknown function '%s'", funcName);
		return AST_INVALID;
	}

	if(!include_aggregates && AR_FuncIsAggregate(funcName)) {
		// Provide a unique error for using aggregate functions from inappropriate contexts
		ErrorCtx_SetError("Invalid use of aggregating function '%s'", funcName);
		return AST_INVALID;
	}

	return AST_VALID;
}

static bool _Validate_apply_all_operator
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	// Working with a function call that has * as its argument.
	const cypher_astnode_t *func = cypher_ast_apply_all_operator_get_func_name(n);
	const char *func_name = cypher_ast_function_name_get_value(func);

	// Verify that this is a COUNT call.
	if(strcasecmp(func_name, "COUNT")) {
		ErrorCtx_SetError("COUNT is the only function which can accept * as an argument");
		vctx->valid = AST_INVALID;
		return false;
	}

	// Verify that DISTINCT is not specified.
	if(cypher_ast_apply_all_operator_get_distinct(n)) {
		// TODO consider opening a parser error, this construction is invalid in Neo's parser.
		ErrorCtx_SetError("Cannot specify both DISTINCT and * in COUNT(DISTINCT *)");
		vctx->valid = AST_INVALID;
		return false;
	}

	return true;
}

static bool _Validate_apply_operator
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	// Collect the function name.
	const cypher_astnode_t *func = cypher_ast_apply_operator_get_func_name(n);
	const char *func_name = cypher_ast_function_name_get_value(func);
	vctx->valid = _ValidateFunctionCall(func_name, vctx->clause == CYPHER_AST_WITH || vctx->clause == CYPHER_AST_RETURN);

	return vctx->valid == AST_VALID;
}

// Validate the property maps used in node/edge patterns in MATCH, and CREATE clauses
static AST_Validation _ValidateInlinedProperties
(
	const cypher_astnode_t *props
) {
	if(props == NULL) return AST_VALID;

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

static bool _Validate_rel_pattern
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		if(vctx->clause == CYPHER_AST_CREATE) {
			// Validate that each relation has exactly one type.
			uint reltype_count = cypher_ast_rel_pattern_nreltypes(n);
			if(reltype_count != 1) {
				ErrorCtx_SetError("Exactly one relationship type must be specified for CREATE");
				vctx->valid = AST_INVALID;
				return false;
			}

			// Validate that each relation being created is directed.
			if(cypher_ast_rel_pattern_get_direction(n) == CYPHER_REL_BIDIRECTIONAL) {
				ErrorCtx_SetError("Only directed relationships are supported in CREATE");
				vctx->valid = AST_INVALID;
				return false;
			}
		}

		vctx->valid = _ValidateInlinedProperties(cypher_ast_rel_pattern_get_properties(n));
		if(vctx->valid == AST_INVALID) return false;

		const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(n);
		if(!alias_node) return true; // Skip unaliased entities.
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		void *alias_type = raxFind(vctx->defined_identifiers, (unsigned char *)alias, strlen(alias));
		if(alias_type == raxNotFound) {
			raxInsert(vctx->defined_identifiers, (unsigned char *)alias, strlen(alias), T_EDGE, NULL);
			return true;
		}
		
		if(alias_type != T_EDGE) {
			ErrorCtx_SetError("The alias '%s' was specified for both a node and a relationship.", alias);
			vctx->valid = AST_INVALID;
			return false;
		}
		
		if(vctx->clause == CYPHER_AST_MATCH) {
			ErrorCtx_SetError("Cannot use the same relationship variable '%s' for multiple patterns.", alias);
			vctx->valid = AST_INVALID;
			return false;
		}

		// If this is a multi-hop traversal, validate it
		const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(n);
		if(range) {
			vctx->valid = _ValidateMultiHopTraversal(n, range);
			return vctx->valid == AST_INVALID;
		}

		if(vctx->clause == CYPHER_AST_MERGE) {
			vctx->valid = _ValidateMergeRelation(n, vctx->defined_identifiers);
			if(vctx->valid == AST_INVALID) return false;
		}

		if(vctx->clause == CYPHER_AST_CREATE) {
			vctx->valid = _ValidateCreateRelation(n, vctx->defined_identifiers);
			if(vctx->valid == AST_INVALID) return false;
		}
	}

	return true;
}

static bool _Validate_node_pattern
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->valid = _ValidateInlinedProperties(cypher_ast_node_pattern_get_properties(n));
		if(vctx->valid == AST_INVALID) return false;

		const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(n);
		if(!alias_node) return true;
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		void *alias_type = raxFind(vctx->defined_identifiers, (unsigned char *)alias, strlen(alias));
		if(alias_type != raxNotFound && alias_type != T_NODE) {
			ErrorCtx_SetError("The alias '%s' was specified for both a node and a relationship.", alias);
			vctx->valid = AST_INVALID;
			return false;
		}
		if(vctx->clause == CYPHER_AST_MERGE) {
			vctx->valid = _ValidateMergeNode(n, vctx->defined_identifiers);
			if(vctx->valid == AST_INVALID) return false;
		}
		raxInsert(vctx->defined_identifiers, (unsigned char *)alias, strlen(alias), T_NODE, NULL);
	}

	return true;
}

static bool _Validate_shortest_path
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(cypher_ast_shortest_path_is_single(n)) {
		// MATCH (a), (b), p = shortestPath((a)-[*]->(b)) RETURN p
		vctx->valid = AST_INVALID;
		ErrorCtx_SetError("RedisGraph currently only supports shortestPath in WITH or RETURN clauses");
	} else {
		// MATCH (a), (b), p = allShortestPaths((a)-[*2..]->(b)) RETURN p
		// validate rel pattern range doesn't contains a minimum > 1
		const cypher_astnode_t **ranges = AST_GetTypedNodes(n, CYPHER_AST_RANGE);
		int range_count = array_len(ranges);
		for(int i = 0; i < range_count; i++) {
			long min_hops = 1;
			const cypher_astnode_t *r = ranges[i];
			const cypher_astnode_t *start = cypher_ast_range_get_start(r);
			if(start) min_hops = AST_ParseIntegerNode(start);
			if(min_hops != 1) {
				vctx->valid = AST_INVALID;
				ErrorCtx_SetError("allShortestPaths(...) does not support a minimal length different from 1");
				break;
			}
		}
		array_free(ranges);
	}

	return vctx->valid == AST_VALID;
}

static bool _Validate_named_path
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		const cypher_astnode_t *alias_node = cypher_ast_named_path_get_identifier(n);
		const char *alias = cypher_ast_identifier_get_name(alias_node);
		raxInsert(vctx->defined_identifiers, (unsigned char *)alias, strlen(alias), NULL, NULL);
	}

	return true;
}

static AST_Validation _Validate_LIMIT_SKIP_Modifiers
(
	const cypher_astnode_t *limit,
	const cypher_astnode_t *skip
) {
	if(limit) {
		// Handle non-integer or non parameter types specified as LIMIT value
		// The value validation of integer node or parameter node is done in run time evaluation.
		if(cypher_astnode_type(limit) != CYPHER_AST_INTEGER &&
			cypher_astnode_type(limit) != CYPHER_AST_PARAMETER) {
			ErrorCtx_SetError("LIMIT specified value of invalid type, must be a positive integer");
			return AST_INVALID;
		}
	}

	if(skip) {
		// Handle non-integer or non parameter types specified as skip value
		// The value validation of integer node or parameter node is done in run time evaluation.
		if(cypher_astnode_type(skip) != CYPHER_AST_INTEGER &&
			cypher_astnode_type(skip) != CYPHER_AST_PARAMETER) {
			ErrorCtx_SetError("SKIP specified value of invalid type, must be a positive integer");
			return AST_INVALID;
		}
	}

	return AST_VALID;
}

static AST_Validation _ValidateUnion_Clauses(const AST *ast) {
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

static bool _Validate_CALL_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	/* Make sure procedure calls are valid:
	 * 1. procedure exists
	 * 2. number of arguments to procedure is as expected
	 * 3. yield refers to procedure output */

	ProcedureCtx    *proc         =  NULL;

	// Make sure procedure exists.
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(n));
	proc = Proc_Get(proc_name);

	if(proc == NULL) {
		ErrorCtx_SetError("Procedure `%s` is not registered", proc_name);
		vctx->valid = AST_INVALID;
		goto cleanup;
	}

	// Validate num of arguments.
	if(proc->argc != PROCEDURE_VARIABLE_ARG_COUNT) {
		unsigned int given_arg_count = cypher_ast_call_narguments(n);
		if(Procedure_Argc(proc) != given_arg_count) {
			ErrorCtx_SetError("Procedure `%s` requires %d arguments, got %d", proc_name, proc->argc,
								given_arg_count);
			vctx->valid = AST_INVALID;
			goto cleanup;
		}
	}

	// validate projections
	uint proj_count = cypher_ast_call_nprojections(n);
	if(proj_count > 0) {
		// collect call projections
		for(uint j = 0; j < proj_count; j++) {
			const cypher_astnode_t *proj = cypher_ast_call_get_projection(n, j);
			const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(proj);
			ASSERT(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			const char *identifier = cypher_ast_identifier_get_name(ast_exp);

			// make sure each yield output is mentioned only once
			if(!raxInsert(vctx->defined_identifiers, (unsigned char *)identifier,
							strlen(identifier), NULL, NULL)) {
				ErrorCtx_SetError("Variable `%s` already declared", identifier);
				vctx->valid = AST_INVALID;
				goto cleanup;
			}

			// make sure procedure is aware of output
			if(!Procedure_ContainsOutput(proc, identifier)) {
				ErrorCtx_SetError("Procedure `%s` does not yield output `%s`",
									proc_name, identifier);
				vctx->valid = AST_INVALID;
				goto cleanup;
			}
		}
	}

cleanup:
	if(proc) Proc_Free(proc);
	return vctx->valid == AST_VALID;
}

static bool _Validate_FOREACH_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		const cypher_astnode_t *identifier_node = cypher_ast_foreach_get_identifier(n);
		const char *identifier = cypher_ast_identifier_get_name(identifier_node);
		raxInsert(vctx->defined_identifiers, (unsigned char *)identifier, strlen(identifier), NULL, NULL);
		return true;
	}

	return true;
}

static bool _Validate_WITH_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	rax *rax = raxNew();

	// Verify that each WITH projection either is aliased or is itself an identifier.
	uint projection_count = cypher_ast_with_nprojections(n);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *proj = cypher_ast_with_get_projection(n, i);
		const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(proj);
		if(!ast_alias &&
		   cypher_astnode_type(cypher_ast_projection_get_expression(proj)) != CYPHER_AST_IDENTIFIER) {
			ErrorCtx_SetError("WITH clause projections must be aliased");
			vctx->valid = AST_INVALID;
			break;
		}
		if(ast_alias == NULL) ast_alias = cypher_ast_projection_get_expression(proj);
		const char *alias = cypher_ast_identifier_get_name(ast_alias);
		// column with same name is invalid
		if(raxTryInsert(rax, (unsigned char *)alias, strlen(alias), NULL, NULL) == 0) {
			ErrorCtx_SetError("Error: Multiple result columns with the same name are not supported.");
			vctx->valid = AST_INVALID;
			break;
		}
	}

	raxFree(rax);

	if(vctx->valid == AST_INVALID) return false;

	vctx->valid = _Validate_LIMIT_SKIP_Modifiers(cypher_ast_with_get_limit(n), cypher_ast_with_get_skip(n));

	return true;
}

static bool _Validate_DELETE_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}
	
	uint expression_count = cypher_ast_delete_nexpressions(n);
	for(uint j = 0; j < expression_count; j++) {
		const cypher_astnode_t *exp = cypher_ast_delete_get_expression(n, j);
		cypher_astnode_type_t type = cypher_astnode_type(exp);
		// expecting an identifier or a function call
		// identifiers and calls that don't resolve to a node or edge
		// will raise an error at run-time
		if(type != CYPHER_AST_IDENTIFIER &&
			type != CYPHER_AST_APPLY_OPERATOR &&
			type != CYPHER_AST_APPLY_ALL_OPERATOR &&
			type != CYPHER_AST_SUBSCRIPT_OPERATOR) {
			ErrorCtx_SetError("DELETE can only be called on nodes and relationships");
			vctx->valid = AST_INVALID;
			break;
		}
	}

	return true;
}

// checks if set property contains non-alias referenes in lhs
static bool _Validate_set_property
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID || !start) return false;

	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(n);
	const cypher_astnode_t *ast_entity = cypher_ast_property_operator_get_expression(ast_prop);
	if(cypher_astnode_type(ast_entity) != CYPHER_AST_IDENTIFIER) {
		ErrorCtx_SetError("RedisGraph does not currently support non-alias references on the left-hand side of SET expressions");
		vctx->valid = AST_INVALID;
		return false;
	}

	return true;
}

static bool _Validate_SET_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	return true;
}

static bool _Validate_CREATE_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	// /* Since we combine all our CREATE clauses in a segment into one operation,
	//  * make sure no data-modifying clauses can separate them. TCK example:
	//  * CREATE (a:A), (b:B) MERGE (a)-[:KNOWS]->(b) CREATE (b)-[:KNOWS]->(c:C) RETURN count(*)
	//  */
	// for(uint i = create_clause_indices[0] + 1; i < create_clause_indices[clause_count - 1]; i ++) {
	// 	const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
	// 	if(cypher_astnode_type(clause) == CYPHER_AST_MERGE) {
	// 		ErrorCtx_SetError("RedisGraph does not support queries of the form CREATE...MERGE...CREATE without a separating WITH clause.");
	// 		res = AST_INVALID;
	// 		goto cleanup;
	// 	}
	// }

	return true;
}

static bool _Validate_MERGE_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	return true;
}

static bool _Validate_UNWIND_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	return true;
}

static bool _Validate_RETURN_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;

	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	if(cypher_ast_return_has_include_existing(n)) return true;

	rax           *rax          = raxNew();
	const char   **columns      = AST_BuildReturnColumnNames(n);
	uint           column_count = array_len(columns);

	for (uint i = 0; i < column_count; i++) {
		// column with same name is invalid
		if(raxTryInsert(rax, (unsigned char *)columns[i], strlen(columns[i]), NULL, NULL) == 0) {
			ErrorCtx_SetError("Error: Multiple result columns with the same name are not supported.");
			vctx->valid = AST_INVALID;
			break;
		}
	}
	
	raxFree(rax);
	array_free(columns);

	if(vctx->valid == AST_INVALID) return false;

	vctx->valid = _Validate_LIMIT_SKIP_Modifiers(cypher_ast_return_get_limit(n), cypher_ast_return_get_skip(n));
	
	return true;
}

static bool _Validate_MATCH_Clause
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	validations_ctx *vctx = visitor->ctx;
	if(vctx->valid == AST_INVALID) return false;
	
	if(start) {
		vctx->clause = cypher_astnode_type(n);
		return true;
	}

	return true;
}

// A query must end in a RETURN clause, a procedure, or an updating clause
// (CREATE, MERGE, DELETE, SET, or REMOVE once supported)
static AST_Validation _ValidateQueryTermination
(
	const AST *ast
) {
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
	   type != CYPHER_AST_CALL     &&
	   type != CYPHER_AST_FOREACH
	  ) {
		ErrorCtx_SetError("Query cannot conclude with %s (must be RETURN or an update clause)",
						  cypher_astnode_typestr(type));
		return AST_INVALID;
	}
	return AST_VALID;
}

// Perform validations not constrained to a specific scope
static AST_Validation _ValidateQuerySequence
(
	const AST *ast
) {
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
static AST_Validation _ValidateClauseOrder
(
	const AST *ast
) {
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

static AST_Validation _ValidateScopes
(
	AST *ast
) {
	// Verify that the RETURN clause and terminating clause do not violate scoping rules.
	if(_ValidateQuerySequence(ast) != AST_VALID) return AST_INVALID;

	// Verify that the clause order in the scope is valid.
	if(_ValidateClauseOrder(ast) != AST_VALID) return AST_INVALID;

	validations_ctx ctx;
	ctx.valid = AST_VALID;
	ctx.defined_identifiers = raxNew();

	ast_visitor *visitor = AST_Visitor_new(&ctx);
	AST_Visitor_register(visitor, CYPHER_AST_MATCH, _Validate_MATCH_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_RETURN, _Validate_RETURN_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_UNWIND, _Validate_UNWIND_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_MERGE, _Validate_MERGE_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_CREATE, _Validate_CREATE_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_SET, _Validate_SET_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_SET_PROPERTY, _Validate_set_property);
	AST_Visitor_register(visitor, CYPHER_AST_DELETE, _Validate_DELETE_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_WITH, _Validate_WITH_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_FOREACH, _Validate_FOREACH_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_CALL, _Validate_CALL_Clause);
	AST_Visitor_register(visitor, CYPHER_AST_NAMED_PATH, _Validate_named_path);
	AST_Visitor_register(visitor, CYPHER_AST_SHORTEST_PATH, _Validate_shortest_path);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_PATTERN, _Validate_node_pattern);
	AST_Visitor_register(visitor, CYPHER_AST_REL_PATTERN, _Validate_rel_pattern);
	AST_Visitor_register(visitor, CYPHER_AST_APPLY_OPERATOR, _Validate_apply_operator);
	AST_Visitor_register(visitor, CYPHER_AST_APPLY_ALL_OPERATOR, _Validate_apply_all_operator);
	AST_Visitor_register(visitor, CYPHER_AST_IDENTIFIER, _Validate_identifier);
	AST_Visitor_register(visitor, CYPHER_AST_PROJECTION, _Validate_projection);
	AST_Visitor_register(visitor, CYPHER_AST_MAP, _Validate_map);
	
	AST_Visitor_visit(ast->root, visitor);
	
	return ctx.valid;
}

// Checks to see if libcypher-parser reported any errors.
bool AST_ContainsErrors
(
	const cypher_parse_result_t *result
) {
	return cypher_parse_result_nerrors(result) > 0;
}

/* This function checks for the existence a valid root in the query.
 * As cypher_parse_result_t can have multiple roots such as comments,
 * only a query that has a root with type CYPHER_AST_STATEMENT is considered valid.
 * Comment roots are ignored. */
AST_Validation AST_Validate_ParseResultRoot
(
	const cypher_parse_result_t *result,
	int *index
) {
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

AST_Validation AST_Validate_Query
(
	const cypher_astnode_t *root
) {
	// Verify that the query does not contain any expressions not in the
	// RedisGraph support whitelist
	if(CypherWhitelist_ValidateQuery(root) != AST_VALID) return AST_INVALID;

	const cypher_astnode_t *body = cypher_ast_statement_get_body(root);
	cypher_astnode_type_t body_type = cypher_astnode_type(body);
	if(body_type == CYPHER_AST_CREATE_NODE_PROPS_INDEX    ||
	   body_type == CYPHER_AST_CREATE_PATTERN_PROPS_INDEX ||
	   body_type == CYPHER_AST_DROP_PROPS_INDEX) {
		// Index operation; validations are handled elsewhere.
		return AST_VALID;
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
	AST_Validation res = _ValidateScopes(&mock_ast);

	return res;
}

//------------------------------------------------------------------------------
// Query params validations
//------------------------------------------------------------------------------

static AST_Validation _ValidateParamsOnly
(
	const cypher_astnode_t *statement
) {
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

static AST_Validation _ValidateDuplicateParameters
(
	const cypher_astnode_t *statement
) {
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

AST_Validation AST_Validate_QueryParams
(
	const cypher_parse_result_t *result
) {
	int index;
	if(AST_Validate_ParseResultRoot(result, &index) != AST_VALID) return AST_INVALID;

	const cypher_astnode_t *root = cypher_parse_result_get_root(result, index);

	// in case of no parameters
	if(cypher_ast_statement_noptions(root) == 0) return AST_VALID;

	if(_ValidateParamsOnly(root)            != AST_VALID)  return AST_INVALID;
	if(_ValidateDuplicateParameters(root)   != AST_VALID)  return AST_INVALID;

	return AST_VALID;
}

// report encountered errors by libcypher-parser
void AST_ReportErrors
(
	const cypher_parse_result_t *result
) {
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
