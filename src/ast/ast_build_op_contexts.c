/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include "ast_build_ar_exp.h"
#include "../util/arr.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"
#include <assert.h>

static inline EdgeCreateCtx _NewEdgeCreateCtx(GraphContext *gc, AST *ast, const QueryGraph *qg,
											  const cypher_astnode_t *path, uint edge_path_offset) {
	const cypher_astnode_t *ast_edge = cypher_ast_pattern_path_get_element(path, edge_path_offset);
	const cypher_astnode_t *ast_props = cypher_ast_rel_pattern_get_properties(ast_edge);
	const cypher_astnode_t *identifier = cypher_ast_rel_pattern_get_identifier(ast_edge);
	const char *alias = cypher_ast_identifier_get_name(identifier);

	// Get QueryGraph entity
	QGEdge *e = QueryGraph_GetEdgeByAlias(qg, alias);
	EdgeCreateCtx new_edge = { .edge = e,
							   .properties = PropertyMap_New(gc, ast_props)
							 };
	return new_edge;
}

static inline NodeCreateCtx _NewNodeCreateCtx(GraphContext *gc, AST *ast, const QueryGraph *qg,
											  const cypher_astnode_t *ast_node) {
	// Get QueryGraph entity
	const cypher_astnode_t *identifier = cypher_ast_node_pattern_get_identifier(ast_node);
	const char *alias = cypher_ast_identifier_get_name(identifier);
	QGNode *n = QueryGraph_GetNodeByAlias(qg, alias);

	const cypher_astnode_t *ast_props = cypher_ast_node_pattern_get_properties(ast_node);

	NodeCreateCtx new_node = { .node = n,
							   .properties = PropertyMap_New(gc, ast_props)
							 };

	return new_node;
}

static void _buildAliasrax(rax *map, const cypher_astnode_t *entity) {
	if(!entity) return;

	cypher_astnode_type_t type = cypher_astnode_type(entity);

	char *alias = NULL;
	if(type == CYPHER_AST_NODE_PATTERN) {
		const cypher_astnode_t *alias_node = cypher_ast_node_pattern_get_identifier(entity);
		if(alias_node) alias = (char *)cypher_ast_identifier_get_name(alias_node);
	} else if(type == CYPHER_AST_REL_PATTERN) {
		const cypher_astnode_t *alias_node = cypher_ast_rel_pattern_get_identifier(entity);
		if(alias_node) alias = (char *)cypher_ast_identifier_get_name(alias_node);
	} else if(type == CYPHER_AST_UNWIND) {
		// The UNWIND clause aliases an expression
		const cypher_astnode_t *alias_node = cypher_ast_unwind_get_alias(entity);
		assert(alias_node);
		alias = (char *)cypher_ast_identifier_get_name(alias_node);
	} else {
		unsigned int child_count = cypher_astnode_nchildren(entity);
		for(unsigned int i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
			// Recursively continue searching
			_buildAliasrax(map, child);
		}
		return;
	}

	if(alias) raxInsert(map, (unsigned char *)alias, strlen(alias), NULL, NULL);
}


static rax *_MatchMerge_DefinedEntities(const AST *ast) {
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	uint match_count = (match_clauses) ? array_len(match_clauses) : 0;

	const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
	uint merge_count = (merge_clauses) ? array_len(merge_clauses) : 0;

	rax *map = raxNew();

	for(uint i = 0; i < match_count; i ++) {
		_buildAliasrax(map, match_clauses[i]);
	}

	for(uint i = 0; i < merge_count; i ++) {
		_buildAliasrax(map, merge_clauses[i]);
	}

	if(match_clauses) array_free(match_clauses);
	if(merge_clauses) array_free(merge_clauses);

	return map;
}

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
		// Property name
		const cypher_astnode_t *prop = cypher_ast_property_operator_get_prop_name(
										   key_to_set); // type == CYPHER_AST_PROP_NAME
		// Entity alias
		const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(key_to_set);
		AR_ExpNode *entity = AR_EXP_FromExpression(prop_expr);
		// Can this ever be anything strange? Assuming it's always just an alias wrapper right now.
		assert(entity->type == AR_EXP_OPERAND && entity->operand.type == AR_EXP_VARIADIC &&
			   entity->operand.variadic.entity_alias);

		// Updated value
		const cypher_astnode_t *val_to_set = cypher_ast_set_property_get_expression(
												 set_item); // type == CYPHER_AST_SET_PROPERTY

		/* Track all required information to perform an update. */
		update_expressions[i].alias = entity->operand.variadic.entity_alias;
		update_expressions[i].attribute = cypher_ast_prop_name_get_value(prop);
		update_expressions[i].exp = AR_EXP_FromExpression(val_to_set);

		AR_EXP_Free(entity);
	}

	*nitems_ref = nitems;
	return update_expressions;
}

void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, const QueryGraph *qg,
						 const char ***nodes_ref, const char ***edges_ref) {
	uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
	const char **nodes_to_delete = array_new(const char *, delete_count);
	const char **edges_to_delete = array_new(const char *, delete_count);

	for(uint i = 0; i < delete_count; i ++) {
		const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
		assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
		const char *alias = cypher_ast_identifier_get_name(ast_expr);

		/* We need to determine whether each alias refers to a node or edge.
		 * Currently, we'll do this by consulting with the QueryGraph. */
		GraphEntityType type = QueryGraph_GetEntityTypeByAlias(qg, alias);
		if(type == GETYPE_NODE) {
			nodes_to_delete = array_append(nodes_to_delete, alias);
		} else if(type == GETYPE_EDGE) {
			edges_to_delete = array_append(edges_to_delete, alias);
		} else {
			assert(false);
		}
	}

	*nodes_ref = nodes_to_delete;
	*edges_ref = edges_to_delete;
}

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

AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause) {
	const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
	AR_ExpNode *exp = AR_EXP_FromExpression(collection);
	const char *alias = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

	AST_UnwindContext ctx = { .exp = exp };
	return ctx;
}

AST_MergeContext AST_PrepareMergeOp(GraphContext *gc, AST *ast,
									const cypher_astnode_t *merge_clause, QueryGraph *qg) {
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);

	uint entity_count = cypher_ast_pattern_path_nelements(path);

	NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, (entity_count / 2) + 1);
	EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, entity_count / 2);

	for(uint i = 0; i < entity_count; i ++) {
		const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
		if(i % 2) {  // Entity is a relationship
			EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, ast, qg, path, i);
			edges_to_merge = array_append(edges_to_merge, new_edge);
		} else { // Entity is a node
			NodeCreateCtx new_node = _NewNodeCreateCtx(gc, ast, qg, cypher_ast_pattern_path_get_element(path,
													   i));
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
	rax *match_entities = _MatchMerge_DefinedEntities(ast);

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
			for(uint k = 0; k < path_elem_count; k ++) {
				/* See if current entity needs to be created:
				 * 1. current entity is NOT in MATCH clause.
				 * 2. We've yet to account for this entity. */
				const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, k);
				const cypher_astnode_t *ast_alias;
				ast_alias = (k % 2) ? cypher_ast_rel_pattern_get_identifier(elem) :
							cypher_ast_node_pattern_get_identifier(elem);

				if(ast_alias) {
					// Encountered an aliased entity - verify that it is not defined
					// in a MATCH clause or a previous CREATE pattern
					const char *alias = cypher_ast_identifier_get_name(ast_alias);

					// Skip entities defined in MATCH clauses or previously appearing in CREATE patterns
					int rc = raxInsert(match_entities, (unsigned char *)alias, strlen(alias), NULL, NULL);
					if(rc == 0) continue;
				}

				if(k % 2) {  // Relation
					EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, ast, qg, path, k);
					edges_to_create = array_append(edges_to_create, new_edge);
				} else { // Node
					NodeCreateCtx new_node = _NewNodeCreateCtx(gc, ast, qg, elem);
					nodes_to_create = array_append(nodes_to_create, new_node);
				}
			}
		}
	}

	raxFree(match_entities);
	array_free(create_clauses);

	AST_CreateContext ctx = { .nodes_to_create = nodes_to_create, .edges_to_create = edges_to_create };

	return ctx;
}

