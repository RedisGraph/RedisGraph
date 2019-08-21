/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>


//------------------------------------------------------------------------------
// AST Map Construction
//------------------------------------------------------------------------------
// Cast an AST ID so that it may be added to the AST entity map
static inline void *_BuildMapValue(uint64_t id) {
	return (void *)id;
}

// Add an AST entity (pointer) to the AST entity map
static uint _ASTMap_AddEntity(const AST *ast, AST_IDENTIFIER identifier, uint id) {
	// Assign a new ID if one is not provided
	if(id == IDENTIFIER_NOT_FOUND) {
		id = raxSize(ast->entity_map);
	}
	void *id_ptr = _BuildMapValue(id);
	raxInsert(ast->entity_map, (unsigned char *)&identifier, sizeof(identifier), id_ptr,
			  NULL);

	return id;
}

/* A path is comprised of 1 or more node/relation patterns, with even offsets
 * corresponding to nodes and odd corresponding to edges:
 * (0)-[1]->(2) */
static void _AST_MapPath(AST *ast, const cypher_astnode_t *path) {
	uint nelems = cypher_ast_pattern_path_nelements(path);
	for(uint i = 0; i < nelems; i ++) {
		const cypher_astnode_t *ast_alias = NULL;
		const cypher_astnode_t *entity = cypher_ast_pattern_path_get_element(path, i);
		if(i % 2) {
			ast_alias = cypher_ast_rel_pattern_get_identifier(entity);
		} else {
			ast_alias = cypher_ast_node_pattern_get_identifier(entity);
		}

		// If the entity is aliased: (a:person)
		// the alias should be mapped as well as the entity.
		//  We may have already constructed a mapping
		// on a previous encounter: MATCH (a)-[]->(a)
		uint id = IDENTIFIER_NOT_FOUND;
		if(ast_alias) {
			// Add alias if it has not already been mapped.
			id = ASTMap_FindOrAddAlias(ast, cypher_ast_identifier_get_name(ast_alias), IDENTIFIER_NOT_FOUND);
		}

		_ASTMap_AddEntity(ast, entity, id);
	}
}

/* A pattern is comprised of 1 or more paths. */
static void _AST_MapPattern(AST *ast, const cypher_astnode_t *pattern) {
	uint npaths = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < npaths; i ++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapPath(ast, path);
	}
}

static void _AST_MapExpression(AST *ast, const cypher_astnode_t *expr) {
	if(expr == NULL) return;
	// A CYPHER_AST_EXPRESSION is a generic type, including function calls,
	// scalars, and identifiers.
	// Any identifiers described within the expression or its children must
	// be represented in the AST mapping.
	const cypher_astnode_type_t type = cypher_astnode_type(expr);

	// Function invocations
	if(type == CYPHER_AST_APPLY_OPERATOR || type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		uint arg_count = cypher_ast_apply_operator_narguments(expr);
		for(uint i = 0; i < arg_count; i ++) {
			const cypher_astnode_t *arg = cypher_ast_apply_operator_get_argument(expr, i);
			// Recursively map arguments
			_AST_MapExpression(ast, arg);
		}
	} else if(type == CYPHER_AST_IDENTIFIER) {
		/* Variables (full nodes and edges, UNWIND artifacts */
		const char *alias = cypher_ast_identifier_get_name(expr);
		ASTMap_FindOrAddAlias(ast, alias, IDENTIFIER_NOT_FOUND);
	} else if(type == CYPHER_AST_PROPERTY_OPERATOR) {
		// Identifier and property pair
		// Extract the entity alias from the property. Currently, the embedded
		// expression should only refer to the IDENTIFIER type.
		const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(expr);
		_AST_MapExpression(ast, prop_expr);
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		/* Operator types (comparisons, filters, functions) */
		const cypher_astnode_t *unary_expr = cypher_ast_unary_operator_get_argument(expr);
		_AST_MapExpression(ast, unary_expr);
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		// Arguments are of type CYPHER_AST_EXPRESSION
		_AST_MapExpression(ast, cypher_ast_binary_operator_get_argument1(expr));
		_AST_MapExpression(ast, cypher_ast_binary_operator_get_argument2(expr));
	} else if(type == CYPHER_AST_COMPARISON) {
		uint nchildren = cypher_ast_comparison_get_length(expr);
		for(uint i = 0; i < nchildren; i ++) {
			_AST_MapExpression(ast, cypher_ast_comparison_get_argument(expr, i));
		}
	} else if(type == CYPHER_AST_CASE) {
		// Value
		_AST_MapExpression(ast, cypher_ast_case_get_expression(expr));
		unsigned int alternatives = cypher_ast_case_nalternatives(expr);
		// Alternatives
		for(uint i = 0; i < alternatives; i++) {
			_AST_MapExpression(ast, cypher_ast_case_get_predicate(expr, i));
			_AST_MapExpression(ast, cypher_ast_case_get_value(expr, i));
		}
		// Default value.
		_AST_MapExpression(ast, cypher_ast_case_get_default(expr));
		return;
	} else if(type == CYPHER_AST_PROC_NAME) {
		return;
	} else if(type == CYPHER_AST_INTEGER ||
			  type == CYPHER_AST_FLOAT   ||
			  type == CYPHER_AST_STRING  ||
			  type == CYPHER_AST_TRUE    ||
			  type == CYPHER_AST_FALSE   ||
			  type == CYPHER_AST_NULL) {
		return;
	} else {
		printf("Encountered unhandled type '%s'\n", cypher_astnode_typestr(type));
		assert(false);
	}
}

static void _AST_MapProjection(AST *ast, const cypher_astnode_t *projection) {
	uint id = IDENTIFIER_NOT_FOUND;
	// A projection contains an expression and optionally an alias.
	// Aliases are not always explicitly provided by the user:
	// "e.name" and "MAX(a) are also considered aliases.
	const cypher_astnode_t *expr = cypher_ast_projection_get_expression(projection);

	// Given a projection like "RETURN a", use "a"
	if(cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(expr);
		id = AST_GetEntityIDFromAlias(ast, identifier);
	}

	const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(projection);
	id = _ASTMap_AddEntity(ast, expr, id);
	// Given a projection like "RETURN a AS e", use "e"
	// Note this also capture identifiers like "RETURN e.name", which causes some keys
	// to be mapped unnecessarily, but has no negative consequences.
	if(ast_alias) {
		id = ASTMap_FindOrAddAlias(ast, cypher_ast_identifier_get_name(ast_alias), id);
	}

	_AST_MapExpression(ast, expr);
}

static void _ASTMap_CollectAliases(AST *ast, const cypher_astnode_t *entity) {
	if(entity == NULL) return;

	if(cypher_astnode_type(entity) == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(entity);
		uint *v = raxFind(ast->entity_map, (unsigned char *)identifier, strlen(identifier));
		if(v == raxNotFound) {
			void *id_ptr = _BuildMapValue(raxSize(ast->entity_map));
			raxInsert(ast->entity_map, (unsigned char *)identifier, strlen(identifier), id_ptr,
					  NULL);
		}
		return;
	}

	uint nchildren = cypher_astnode_nchildren(entity);
	for(uint i = 0; i < nchildren; i ++) {
		_ASTMap_CollectAliases(ast, cypher_astnode_get_child(entity, i));
	}
}

void AST_BuildEntityMap(AST *ast) {
	/* The AST->entity_map uses AST node pointers and string aliases as keys.
	 * These keys resolve to integer IDs.
	 * Not all keys have a unique ID, as multiple AST nodes might describe the same
	 * entity (as will aliases).
	 * The ExecutionPlanSegment will contain a mapping that converts these IDs
	 * as well as other keys to Record IDs. */
	ast->entity_map = raxNew();

	// Recursively map every identifier in this AST.
	_ASTMap_CollectAliases(ast, ast->root);

	// Check every clause in this AST.
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_MATCH) {
			// MATCH and CREATE clauses have 1 pattern which contains 1 or more paths.
			_AST_MapPattern(ast, cypher_ast_match_get_pattern(clause));
		} else if(type == CYPHER_AST_CREATE) {
			// MATCH and CREATE clauses have 1 pattern which contains 1 or more paths.
			_AST_MapPattern(ast, cypher_ast_create_get_pattern(clause));
		} else if(type == CYPHER_AST_MERGE) {
			// MERGE clauses contain exactly one path.
			_AST_MapPath(ast, cypher_ast_merge_get_pattern_path(clause));
		} else if(type == CYPHER_AST_UNWIND) {
			// An UNWIND clause introduces 1 new alias.
			const char *alias = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(clause));
			ASTMap_FindOrAddAlias(ast, alias, IDENTIFIER_NOT_FOUND);
		} else if(type == CYPHER_AST_WITH) {
			// WITH entities are mapped elsewhere.
			continue;
		} else if(type == CYPHER_AST_RETURN) {
			uint projection_count = cypher_ast_return_nprojections(clause);
			for(uint i = 0 ; i < projection_count; i ++) {
				const cypher_astnode_t *projection = cypher_ast_return_get_projection(clause, i);
				_AST_MapProjection(ast, projection);
			}
		} else if(type == CYPHER_AST_SET) {
			uint set_count = cypher_ast_set_nitems(clause);
			for(uint i = 0 ; i < set_count; i ++) {
				const cypher_astnode_t *set_item = cypher_ast_set_get_item(clause, i);
				// SET clauses in Cypher can also set labels and all properties, which have different types
				// that we don't yet support.
				assert(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				const cypher_astnode_t *expr = cypher_ast_set_property_get_expression(set_item);
				_AST_MapExpression(ast, expr);

			}
		} else if(type == CYPHER_AST_CALL) {
			const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(clause));
			ASTMap_FindOrAddAlias(ast, proc_name, IDENTIFIER_NOT_FOUND);
			uint projection_count = cypher_ast_call_nprojections(clause);
			for(uint i = 0 ; i < projection_count; i ++) {
				const cypher_astnode_t *projection = cypher_ast_call_get_projection(clause, i);
				_AST_MapProjection(ast, projection);
			}
		} else {
			uint child_count = cypher_astnode_nchildren(clause);
			for(uint j = 0; j < child_count; j ++) {
				const cypher_astnode_t *child = cypher_astnode_get_child(clause, j);
				_AST_MapExpression(ast, child);
			}
		}
	}
}

//------------------------------------------------------------------------------
// AST Map Retrieval
//------------------------------------------------------------------------------
uint AST_GetEntityIDFromReference(const AST *ast, AST_IDENTIFIER entity) {
	void *id = raxFind(ast->entity_map, (unsigned char *)&entity, sizeof(entity));
	if(id == raxNotFound) return IDENTIFIER_NOT_FOUND;
	return (uint64_t)id;
}

uint AST_GetEntityIDFromAlias(const AST *ast, const char *alias) {
	void *id = raxFind(ast->entity_map, (unsigned char *)alias, strlen(alias));
	if(id == raxNotFound) return IDENTIFIER_NOT_FOUND;
	return (uint64_t)id;
}

// Add alias if it has not already been mapped and return ID
uint ASTMap_FindOrAddAlias(const AST *ast, const char *alias, uint id) {
	uint *v = raxFind(ast->entity_map, (unsigned char *)alias, strlen(alias));
	if(v != raxNotFound) return (uint64_t)v;

	// Assign a new ID if one is not provided
	if(id == IDENTIFIER_NOT_FOUND) {
		id = raxSize(ast->entity_map);
	}
	void *id_ptr = _BuildMapValue(id);
	raxInsert(ast->entity_map, (unsigned char *)alias, strlen(alias), id_ptr, NULL);

	return id;
}
