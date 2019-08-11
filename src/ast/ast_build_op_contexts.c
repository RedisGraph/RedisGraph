/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

static inline EdgeCreateCtx _NewEdgeCreateCtx(const QueryGraph *qg,
											  const cypher_astnode_t *ast_edge) {
	// Get QueryGraph entity
	const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(ast_edge);
	const char *alias = cypher_ast_identifier_get_name(identifier);
	QGEdge *e = QueryGraph_GetEdgeByAlias(qg, alias);

	// Get properties.
	const cypher_astnode_t *ast_props = cypher_ast_rel_pattern_get_properties(ast_edge);
	PropertyMap *properties = AST_ConvertPropertiesMap(ast_props);

	EdgeCreateCtx new_edge = { .edge = e, .properties = properties };
	return new_edge;
}

static inline NodeCreateCtx _NewNodeCreateCtx(const QueryGraph *qg,
											  const cypher_astnode_t *ast_node) {
	// Get QueryGraph entity
	const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(ast_node);
	const char *alias = cypher_ast_identifier_get_name(identifier);
	QGNode *n = QueryGraph_GetNodeByAlias(qg, alias);

	// Get properties.
	const cypher_astnode_t *ast_props = cypher_ast_node_pattern_get_properties(ast_node);
	PropertyMap *properties = AST_ConvertPropertiesMap(ast_props);

	NodeCreateCtx new_node = { .node = n, .properties = properties };
	return new_node;
}

static void _buildAliasTrieMap(TrieMap *map, const cypher_astnode_t *entity) {
	if(!entity) return;

	cypher_astnode_type_t type = cypher_astnode_type(entity);

	const char *alias = NULL;
	const cypher_astnode_t *identifier = NULL;

	if(type == CYPHER_AST_NODE_PATTERN) {
		identifier = cypher_ast_node_pattern_get_identifier(entity);
		alias = cypher_ast_identifier_get_name(identifier);
	} else if(type == CYPHER_AST_REL_PATTERN) {
		identifier = cypher_ast_rel_pattern_get_identifier(entity);
		alias = cypher_ast_identifier_get_name(identifier);
	} else if(type == CYPHER_AST_UNWIND) {
		// The UNWIND clause aliases an expression
		identifier = cypher_ast_unwind_get_alias(entity);
		assert(identifier);
		alias = (char *)cypher_ast_identifier_get_name(identifier);
	} else {
		unsigned int child_count = cypher_astnode_nchildren(entity);
		for(unsigned int i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
			// Recursively continue searching
			_buildAliasTrieMap(map, child);
		}
		return;
	}

	if(alias) TrieMap_Add(map, (char *)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
}

static TrieMap *_MatchMerge_DefinedEntities(const AST *ast) {
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	uint match_count = (match_clauses) ? array_len(match_clauses) : 0;

	const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
	uint merge_count = (merge_clauses) ? array_len(merge_clauses) : 0;

	TrieMap *map = NewTrieMap();
	for(uint i = 0; i < match_count; i ++) _buildAliasTrieMap(map, match_clauses[i]);
	for(uint i = 0; i < merge_count; i ++) _buildAliasTrieMap(map, merge_clauses[i]);
	if(match_clauses) array_free(match_clauses);
	if(merge_clauses) array_free(merge_clauses);

	return map;
}

PropertyMap *AST_ConvertPropertiesMap(const cypher_astnode_t *props) {
	if(props == NULL) return NULL;
	assert(cypher_astnode_type(props) == CYPHER_AST_MAP); // TODO add parameter support

	uint prop_count = cypher_ast_map_nentries(props);

	PropertyMap *map = malloc(sizeof(PropertyMap));
	map->keys = malloc(prop_count * sizeof(char *));
	map->values = malloc(prop_count * sizeof(SIValue));
	map->property_count = prop_count;

	for(uint prop_idx = 0; prop_idx < prop_count; prop_idx++) {
		const cypher_astnode_t *ast_key = cypher_ast_map_get_key(props, prop_idx);
		map->keys[prop_idx] = cypher_ast_prop_name_get_value(ast_key);

		const cypher_astnode_t *ast_value = cypher_ast_map_get_value(props, prop_idx);
		AR_ExpNode *value_exp = AR_EXP_FromExpression(ast_value);
		// TODO It will be really nice to store AR_ExpNodes rather than resolved SIValues;
		// allowing things like "CREATE (:b {prop: a.name})"
		SIValue value = AR_EXP_Evaluate(value_exp, NULL);
		AR_EXP_Free(value_exp);
		map->values[prop_idx] = value;
	}
	return map;
}

AR_ExpNode **_AST_ConvertCollection(const cypher_astnode_t *collection) {
	assert(cypher_astnode_type(collection) == CYPHER_AST_COLLECTION);

	uint expCount = cypher_ast_collection_length(collection);
	AR_ExpNode **expressions = array_new(AR_ExpNode *, expCount);

	for(uint i = 0; i < expCount; i ++) {
		const cypher_astnode_t *exp_node = cypher_ast_collection_get(collection, i);
		AR_ExpNode *exp = AR_EXP_FromExpression(exp_node);
		expressions = array_append(expressions, exp);
	}

	return expressions;
}

//------------------------------------------------------------------------------
// SET operations
//------------------------------------------------------------------------------

EntityUpdateEvalCtx *AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, uint *nitems_ref) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	EntityUpdateEvalCtx *update_expressions = rm_malloc(sizeof(EntityUpdateEvalCtx) * nitems);

	for(uint i = 0; i < nitems; i++) {
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		const cypher_astnode_type_t type = cypher_astnode_type(set_item);
		// TODO Add handling for when we're setting labels (CYPHER_AST_SET_LABELS)
		// or all properties (CYPHER_AST_SET_ALL_PROPERTIES)
		assert(type == CYPHER_AST_SET_PROPERTY);

		// The SET_ITEM contains the entity alias and property key being set
		const cypher_astnode_t *key_to_set = cypher_ast_set_property_get_property(
												 set_item); // type == CYPHER_AST_PROPERTY_OPERATOR

		// Entity name.
		const cypher_astnode_t *identifier_node = cypher_ast_property_operator_get_expression(key_to_set);
		const char *alias = cypher_ast_identifier_get_name(identifier_node);

		// Property name.
		const cypher_astnode_t *prop_node = cypher_ast_property_operator_get_prop_name(key_to_set);
		const char *prop = cypher_ast_prop_name_get_value(prop_node);

		// Updated value.
		const cypher_astnode_t *val_to_set_node = cypher_ast_set_property_get_expression(set_item);
		AR_ExpNode *exp = AR_EXP_FromExpression(val_to_set_node);

		/* Track all required information to perform an update. */
		update_expressions[i].exp = exp;
		update_expressions[i].alias = alias;
		update_expressions[i].attribute = prop;
	}

	*nitems_ref = nitems;
	return update_expressions;
}

//------------------------------------------------------------------------------
// DELETE operations
//------------------------------------------------------------------------------

char **AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause) {
	uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
	char **deleted_entities = array_new(char *, delete_count);

	for(uint i = 0; i < delete_count; i ++) {
		const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
		assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
		const char *alias = cypher_ast_identifier_get_name(ast_expr);
		deleted_entities = array_append(deleted_entities, (char *)alias);
	}
	return deleted_entities;
}

//------------------------------------------------------------------------------
// ORDER-BY operations
//------------------------------------------------------------------------------
int AST_PrepareSortOp(const cypher_astnode_t *order_clause) {
	assert(order_clause);

	bool ascending = true;
	unsigned int nitems = cypher_ast_order_by_nitems(order_clause);

	for(unsigned int i = 0; i < nitems; i ++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		// TODO direction should be specifiable per order entity
		ascending = cypher_ast_sort_item_is_ascending(item);
	}

	int direction = ascending ? DIR_ASC : DIR_DESC;

	return direction;
}

//------------------------------------------------------------------------------
// UNWIND operations
//------------------------------------------------------------------------------
AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause) {
	const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
	AR_ExpNode **exps = _AST_ConvertCollection(collection);
	const char *alias = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

	AST_UnwindContext ctx = { .exps = exps, .alias = alias };
	return ctx;
}

//------------------------------------------------------------------------------
// MERGE operations
//------------------------------------------------------------------------------
AST_MergeContext AST_PrepareMergeOp(AST *ast, const cypher_astnode_t *merge_clause,
									QueryGraph *qg) {
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);

	uint entity_count = cypher_ast_pattern_path_nelements(path);

	NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, (entity_count / 2) + 1);
	EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, entity_count / 2);

	for(uint i = 0; i < entity_count; i ++) {
		const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
		if(i % 2) {  // Entity is a relationship
			EdgeCreateCtx new_edge = _NewEdgeCreateCtx(qg, path);
			edges_to_merge = array_append(edges_to_merge, new_edge);
		} else {
			// Entity is a node
			NodeCreateCtx new_node = _NewNodeCreateCtx(qg, elem);
			nodes_to_merge = array_append(nodes_to_merge, new_node);
		}
	}

	AST_MergeContext ctx = { .nodes_to_merge = nodes_to_merge, .edges_to_merge = edges_to_merge };
	return ctx;
}

//------------------------------------------------------------------------------
// CREATE operations
//------------------------------------------------------------------------------
AST_CreateContext AST_PrepareCreateOp(GraphContext *gc, AST *ast, QueryGraph *qg) {
	const cypher_astnode_t **create_clauses = AST_GetClauses(ast, CYPHER_AST_CREATE);
	uint create_count = (create_clauses) ? array_len(create_clauses) : 0;

	/* For every entity within the CREATE clause see if it's also mentioned
	 * within the MATCH clause. */
	TrieMap *match_entities = _MatchMerge_DefinedEntities(ast);

	NodeCreateCtx *nodes_to_create = array_new(NodeCreateCtx, 1);
	EdgeCreateCtx *edges_to_create = array_new(EdgeCreateCtx, 1);

	for(uint i = 0; i < create_count; i++) {
		const cypher_astnode_t *clause = create_clauses[i];
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
		uint npaths = cypher_ast_pattern_npaths(pattern);

		for(uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			// Add the path to the QueryGraph
			QueryGraph_AddPath(gc, qg, path);

			uint path_elem_count = cypher_ast_pattern_path_nelements(path);
			for(uint j = 0; j < path_elem_count; j ++) {
				/* See if current entity needs to be created:
				 * 1. current entity is NOT in MATCH clause.
				 * 2. We've yet to account for this entity. */
				const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, j);
				const cypher_astnode_t *ast_alias;
				ast_alias = (j % 2) ? cypher_ast_rel_pattern_get_identifier(elem) :
							cypher_ast_node_pattern_get_identifier(elem);

				// Verify that it is not defined in a MATCH clause or a previous CREATE pattern
				const char *alias = cypher_ast_identifier_get_name(ast_alias);

				// Skip entities defined in MATCH clauses or previously appearing in CREATE patterns
				int rc = TrieMap_Add(match_entities, (char *)alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
				if(rc == 0) continue;

				if(j % 2) {  // Relation
					EdgeCreateCtx new_edge = _NewEdgeCreateCtx(qg, elem);
					edges_to_create = array_append(edges_to_create, new_edge);
				} else { // Node
					NodeCreateCtx new_node = _NewNodeCreateCtx(qg, elem);
					nodes_to_create = array_append(nodes_to_create, new_node);
				}
			}
		}
	}

	TrieMap_Free(match_entities, TrieMap_NOP_CB);
	array_free(create_clauses);

	AST_CreateContext ctx = { .nodes_to_create = nodes_to_create, .edges_to_create = edges_to_create };

	return ctx;
}
