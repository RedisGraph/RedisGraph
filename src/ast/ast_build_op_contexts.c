/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include "ast_build_ar_exp.h"
#include "../util/arr.h"
#include "../util/rax_extensions.h"
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

static EntityUpdateEvalCtx _NewUpdateCtx(GraphContext *gc, const cypher_astnode_t *set_item) {
	const cypher_astnode_type_t type = cypher_astnode_type(set_item);
	// TODO Add handling for when we're setting labels (CYPHER_AST_SET_LABELS)
	// or all properties (CYPHER_AST_SET_ALL_PROPERTIES)
	assert(type == CYPHER_AST_SET_PROPERTY);

	// The SET_ITEM contains the entity alias and property key being set - type == CYPHER_AST_PROPERTY_OPERATOR
	const cypher_astnode_t *key_to_set = cypher_ast_set_property_get_property(set_item);
	// Property name - type == CYPHER_AST_PROP_NAME
	const cypher_astnode_t *prop = cypher_ast_property_operator_get_prop_name(key_to_set);
	// Entity alias
	const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(key_to_set);
	AR_ExpNode *entity = AR_EXP_FromExpression(prop_expr);
	// Can this ever be anything strange? Assuming it's always just an alias wrapper right now.
	assert(entity->type == AR_EXP_OPERAND && entity->operand.type == AR_EXP_VARIADIC &&
		   entity->operand.variadic.entity_alias);

	// Updated value - type == CYPHER_AST_SET_PROPERTY
	const cypher_astnode_t *val_to_set = cypher_ast_set_property_get_expression(set_item);

	/* Track all required information to perform an update. */
	const char *alias = entity->operand.variadic.entity_alias;
	const char *attribute = cypher_ast_prop_name_get_value(prop);
	Attribute_ID attribute_id = GraphContext_FindOrAddAttribute(gc, attribute);
	AR_ExpNode *exp = AR_EXP_FromExpression(val_to_set);

	AR_EXP_Free(entity);

	EntityUpdateEvalCtx update_ctx = { .alias = alias,
									   .attribute_id = attribute_id,
									   .exp = exp
									 };
	return update_ctx;
}

EntityUpdateEvalCtx *AST_PrepareUpdateOp(GraphContext *gc, const cypher_astnode_t *set_clause) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	EntityUpdateEvalCtx *update_expressions = array_new(EntityUpdateEvalCtx, nitems);

	for(uint i = 0; i < nitems; i++) {
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		update_expressions = array_append(update_expressions, _NewUpdateCtx(gc, set_item));
	}

	return update_expressions;
}

AR_ExpNode **AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause) {
	AST *ast = QueryCtx_GetAST();
	uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
	AR_ExpNode **exps = array_new(AR_ExpNode *, delete_count);

	for(uint i = 0; i < delete_count; i ++) {
		const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_expr);
		exps = array_append(exps, exp);
	}
	return exps;
}

// Get direction of each sort operation, append to an array, return the array in the form of out parameter
void AST_PrepareSortOp(const cypher_astnode_t *order_clause, int **sort_directions) {
	assert(order_clause && sort_directions);

	unsigned int nitems = cypher_ast_order_by_nitems(order_clause);
	int *directions = array_new(int, nitems);

	for(unsigned int i = 0; i < nitems; i ++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		int direction = cypher_ast_sort_item_is_ascending(item) ? DIR_ASC : DIR_DESC;
		directions = array_append(directions, direction);
	}

	*sort_directions = directions;
}

AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause) {
	const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
	AR_ExpNode *exp = AR_EXP_FromExpression(collection);
	exp->resolved_name = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

	AST_UnwindContext ctx = { .exp = exp };
	return ctx;
}

void AST_PreparePathCreation(const cypher_astnode_t *path, QueryGraph *qg, rax *bound_vars,
							 NodeCreateCtx **nodes, EdgeCreateCtx **edges) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Add the path to the QueryGraph
	QueryGraph_AddPath(qg, gc, path);

	uint path_elem_count = cypher_ast_pattern_path_nelements(path);
	for(uint i = 0; i < path_elem_count; i ++) {
		/* See if current entity needs to be created:
		 * 1. Current entity is NOT bound in a previous clause.
		 * 2. We have yet to account for this entity. */
		const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
		const char *alias = AST_GetEntityName(ast, elem);

		// Skip entities defined in previous clauses or already represented in our nodes/edges arrays.
		int rc = raxTryInsert(bound_vars, (unsigned char *)alias, strlen(alias), NULL, NULL);
		if(rc == 0) continue;

		if(i % 2) {  // Relation
			EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, qg, elem);
			*edges = array_append(*edges, new_edge);
		} else {     // Node
			NodeCreateCtx new_node = _NewNodeCreateCtx(gc, qg, elem);
			*nodes = array_append(*nodes, new_node);
		}
	}
}

AST_MergeContext AST_PrepareMergeOp(const cypher_astnode_t *merge_clause, GraphContext *gc,
									QueryGraph *qg, rax *bound_vars) {
	AST_MergeContext merge_ctx = { .nodes_to_merge = NULL,
								   .edges_to_merge = NULL,
								   .on_match = NULL,
								   .on_create = NULL
								 };

	// Prepare all create contexts for nodes and edges on Merge path.
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);
	NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, 1);
	EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, 1);

	// Shouldn't operate on the original bound variables map, as this function may insert aliases.
	rax *bound_and_introduced_entities = (bound_vars) ? raxClone(bound_vars) : raxNew();
	AST_PreparePathCreation(path, qg, bound_and_introduced_entities, &nodes_to_merge, &edges_to_merge);
	raxFree(bound_and_introduced_entities);

	merge_ctx.nodes_to_merge = nodes_to_merge;
	merge_ctx.edges_to_merge = edges_to_merge;

	// Convert any ON MATCH and ON CREATE directives.
	uint directive_count = cypher_ast_merge_nactions(merge_clause);
	if(directive_count == 0) return merge_ctx;

	EntityUpdateEvalCtx *on_create_items = NULL;
	EntityUpdateEvalCtx *on_match_items = NULL;

	for(uint i = 0; i < directive_count; i ++) {
		const cypher_astnode_t *directive = cypher_ast_merge_get_action(merge_clause, i);
		cypher_astnode_type_t type = cypher_astnode_type(directive);

		if(type == CYPHER_AST_ON_CREATE) {
			uint create_prop_count = cypher_ast_on_create_nitems(directive);
			if(on_create_items == NULL) on_create_items = array_new(EntityUpdateEvalCtx, create_prop_count);
			for(uint j = 0; j < create_prop_count; j ++) {
				const cypher_astnode_t *create_item = cypher_ast_on_create_get_item(directive, j);
				on_create_items = array_append(on_create_items, _NewUpdateCtx(gc, create_item));
			}
		} else if(type == CYPHER_AST_ON_MATCH) {
			uint match_prop_count = cypher_ast_on_match_nitems(directive);
			if(on_match_items == NULL) on_match_items = array_new(EntityUpdateEvalCtx, match_prop_count);
			for(uint j = 0; j < match_prop_count; j ++) {
				const cypher_astnode_t *match_item = cypher_ast_on_match_get_item(directive, j);
				on_match_items = array_append(on_match_items, _NewUpdateCtx(gc, match_item));
			}
		} else {
			assert(false);
		}
	}

	merge_ctx.on_match = on_match_items;
	merge_ctx.on_create = on_create_items;

	return merge_ctx;
}

//------------------------------------------------------------------------------
// CREATE operations
//------------------------------------------------------------------------------
AST_CreateContext AST_PrepareCreateOp(QueryGraph *qg, rax *bound_vars) {
	AST *ast = QueryCtx_GetAST();

	// Shouldn't operate on the original bound variables map, as this function may insert aliases.
	rax *bound_and_introduced_entities = raxClone(bound_vars);

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
			AST_PreparePathCreation(path, qg, bound_and_introduced_entities, &nodes_to_create,
									&edges_to_create);
		}
	}

	array_free(create_clauses);
	raxFree(bound_and_introduced_entities);

	AST_CreateContext ctx = { .nodes_to_create = nodes_to_create, .edges_to_create = edges_to_create };

	return ctx;
}

