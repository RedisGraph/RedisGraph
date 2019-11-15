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
#include "../query_ctx.h"
#include <assert.h>

static inline EdgeCreateCtx _NewEdgeCreateCtx(GraphContext *gc, const QueryGraph *qg,
											  const cypher_astnode_t *edge) {
	const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(edge);
	AST *ast = QueryCtx_GetAST();
	const char *alias = AST_GetEntityName(ast, edge);

	// Get QueryGraph entity
	QGEdge *e = QueryGraph_GetEdgeByAlias(qg, alias);
	EdgeCreateCtx new_edge = { .edge = e,
							   .properties = PropertyMap_New(gc, props)
							 };
	return new_edge;
}

static inline NodeCreateCtx _NewNodeCreateCtx(GraphContext *gc, const QueryGraph *qg,
											  const cypher_astnode_t *ast_node) {
	// Get QueryGraph entity
	AST *ast = QueryCtx_GetAST();
	const char *alias = AST_GetEntityName(ast, ast_node);
	QGNode *n = QueryGraph_GetNodeByAlias(qg, alias);

	const cypher_astnode_t *ast_props = cypher_ast_node_pattern_get_properties(ast_node);

	NodeCreateCtx new_node = { .node = n,
							   .properties = PropertyMap_New(gc, ast_props)
							 };

	return new_node;
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
		EntityType type = QueryGraph_GetEntityTypeByAlias(qg, alias);
		if(type == ENTITY_NODE) {
			nodes_to_delete = array_append(nodes_to_delete, alias);
		} else if(type == ENTITY_EDGE) {
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
	exp->resolved_name = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

	AST_UnwindContext ctx = { .exp = exp };
	return ctx;
}

AST_MergeContext AST_PrepareMergeOp(const cypher_astnode_t *merge_clause, QueryGraph *qg,
									const char **bound_variables_arr) {
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	QueryGraph_AddPath(qg, gc, path); // TODO necessary?

	uint entity_count = cypher_ast_pattern_path_nelements(path);

	// Build a rax to quickly look up bound variables. // TODO simpler option?
	rax *bound_variables = raxNew();
	uint bound_var_count = array_len(bound_variables_arr);
	for(uint i = 0; i < bound_var_count; i ++) {
		const char *var = bound_variables_arr[i];
		raxInsert(bound_variables, (unsigned char *)var, strlen(var), NULL, NULL);
	}

	// TODO largely a duplicate of AST_PrepareCreateOp logic, consolidate the two.
	NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, (entity_count / 2) + 1);
	EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, entity_count / 2);

	AST *ast = QueryCtx_GetAST();
	for(uint i = 0; i < entity_count; i ++) {
		const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
		const char *elem_name = AST_GetEntityName(ast, elem);
		if(raxFind(bound_variables, (unsigned char *)elem_name, strlen(elem_name)) == raxNotFound) {
			if(i % 2) {  // Entity is a relationship
				EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, qg, elem);
				edges_to_merge = array_append(edges_to_merge, new_edge);
			} else { // Entity is a node
				NodeCreateCtx new_node = _NewNodeCreateCtx(gc, qg, elem);
				nodes_to_merge = array_append(nodes_to_merge, new_node);
			}
		}
	}

	raxFree(bound_variables);
	AST_MergeContext ctx = { .nodes_to_merge = nodes_to_merge, .edges_to_merge = edges_to_merge };
	return ctx;
}


//------------------------------------------------------------------------------
// CREATE operations
//------------------------------------------------------------------------------
AST_CreateContext AST_PrepareCreateOp(QueryGraph *qg, rax *bound_variables) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();
	const cypher_astnode_t **create_clauses = AST_GetClauses(ast, CYPHER_AST_CREATE);
	uint create_count = (create_clauses) ? array_len(create_clauses) : 0;

	NodeCreateCtx *nodes_to_create = array_new(NodeCreateCtx, 1);
	EdgeCreateCtx *edges_to_create = array_new(EdgeCreateCtx, 1);

	for(uint i = 0; i < create_count; i++) {
		const cypher_astnode_t *clause = create_clauses[i];
		const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
		uint npaths = cypher_ast_pattern_npaths(pattern);

		for(uint j = 0; j < npaths; j++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			// Add the path to the QueryGraph
			QueryGraph_AddPath(qg, gc, path);

			uint path_elem_count = cypher_ast_pattern_path_nelements(path);
			for(uint k = 0; k < path_elem_count; k ++) {
				/* See if current entity needs to be created:
				 * 1. current entity is NOT in MATCH clause.
				 * 2. We've yet to account for this entity. */
				const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, k);
				const char *alias = AST_GetEntityName(ast, elem);

				// Skip entities defined in MATCH clauses or previously appearing in CREATE patterns
				int rc = raxInsert(bound_variables, (unsigned char *)alias, strlen(alias), NULL, NULL);
				if(rc == 0) continue;

				if(k % 2) {  // Relation
					EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, qg, elem);
					edges_to_create = array_append(edges_to_create, new_edge);
				} else { // Node
					NodeCreateCtx new_node = _NewNodeCreateCtx(gc, qg, elem);
					nodes_to_create = array_append(nodes_to_create, new_node);
				}
			}
		}
	}

	array_free(create_clauses);

	AST_CreateContext ctx = { .nodes_to_create = nodes_to_create, .edges_to_create = edges_to_create };

	return ctx;
}

