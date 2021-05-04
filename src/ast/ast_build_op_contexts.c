/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_build_op_contexts.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/rax_extensions.h"
#include "../arithmetic/arithmetic_expression_construct.h"
#include "../query_ctx.h"

static inline EdgeCreateCtx _NewEdgeCreateCtx(GraphContext *gc, const QGEdge *e,
											  const cypher_astnode_t *edge) {
	const cypher_astnode_t *props = cypher_ast_rel_pattern_get_properties(edge);

	EdgeCreateCtx new_edge = {  .alias = e->alias,
								.relation = e->reltypes[0],
								.reltypeId = e->reltypeIDs[0],
								.properties = PropertyMap_New(gc, props),
								.src = e->src->alias,
								.dest = e->dest->alias
							 };
	return new_edge;
}

static inline NodeCreateCtx _NewNodeCreateCtx(GraphContext *gc, const QGNode *n,
											  const cypher_astnode_t *ast_node) {
	const cypher_astnode_t *ast_props = cypher_ast_node_pattern_get_properties(ast_node);

	NodeCreateCtx new_node;
	new_node.alias = n->alias;
	new_node.properties = PropertyMap_New(gc, ast_props);
	array_clone(new_node.labels, n->labels);
	array_clone(new_node.labelsId, n->labelsID);

	return new_node;
}

// populates 'map' with set expressions e.g. v = 1
// 'map' must have enough space to contain all of set-expressions
// defined in 'ast_map'
static void _ConstructPropertyMap(PropertySetCtx *map, uint map_size,
		GraphContext *gc, const cypher_astnode_t *ast_map) {
	AR_ExpNode *exp;
	const char *attribute;
	Attribute_ID attribute_id;
	const cypher_astnode_t *ast_key;
	const cypher_astnode_t *ast_val;

	uint count = cypher_ast_map_nentries(ast_map);
	ASSERT(map_size == count);

	for(uint i = 0; i < count; i ++) {
		// property name
		ast_key       =  cypher_ast_map_get_key(ast_map, i);
		attribute     =  cypher_ast_prop_name_get_value(ast_key);
		attribute_id  =  GraphContext_FindOrAddAttribute(gc, attribute);

		// property value
		ast_val  = cypher_ast_map_get_value(ast_map, i);
		exp      = AR_EXP_FromASTNode(ast_val);
		map[i]   = (PropertySetCtx) { .id = attribute_id, .exp = exp };
	}
}

static void _UpdateCtx_AddPropertyMap(GraphContext *gc, EntityUpdateEvalCtx *ctx,
		const cypher_astnode_t *ast_map) {
	ASSERT(gc       !=  NULL);
	ASSERT(ctx      !=  NULL);
	ASSERT(ast_map  !=  NULL);

	// find out number of entries in map
	uint count = cypher_ast_map_nentries(ast_map);
	// allocate and populate stack base array holding each map element
	PropertySetCtx map[count];
	_ConstructPropertyMap(map, count, gc, ast_map);

	// merge map into entity update eval context
	for(uint i = 0; i < count; i ++) {
		ctx->properties = array_append(ctx->properties, map[i]);
	}
}

// Merge the given map with existing properties
static void _Update_MergePropertyMap(GraphContext *gc, rax *updates,
									 const cypher_astnode_t *set_item) {
	ASSERT(gc        !=  NULL);
	ASSERT(updates   !=  NULL);
	ASSERT(set_item  !=  NULL);

	// The SET_ITEM contains the entity alias and property map being appended
	// Entity alias
	const cypher_astnode_t *prop_expr = cypher_ast_merge_properties_get_identifier(set_item);
	ASSERT(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
	const char *alias = cypher_ast_identifier_get_name(prop_expr);

	// Property map
	const cypher_astnode_t *ast_map = cypher_ast_merge_properties_get_expression(set_item);
	if(cypher_astnode_type(ast_map) != CYPHER_AST_MAP) {
		// TODO introduce support for queries like:
		// MATCH (a {v: 1}), (b {v: 2}) SET a += b
		ErrorCtx_SetError("RedisGraph does not currently support assigning graph entities to non-map values.");
		return;
	}

	// retrieve or instantiate an update context
	int len = strlen(alias);
	EntityUpdateEvalCtx *ctx = raxFind(updates, (unsigned char *)alias, len);
	if(ctx == raxNotFound) {
		uint count = cypher_ast_map_nentries(ast_map);
		ctx = UpdateCtx_New(UPDATE_MERGE, count, alias);
		raxInsert(updates, (unsigned char *)alias, len, ctx, NULL);
	}

	// add all properties to update context
	_UpdateCtx_AddPropertyMap(gc, ctx, ast_map);
}

// Replace existing properties with the given map
static void _Update_SetPropertyMap(GraphContext *gc, rax *updates,
								   const cypher_astnode_t *set_item) {
	// a = {v: 5}
	// The SET_ITEM contains the entity alias and property map being set

	// Entity alias
	const cypher_astnode_t *prop_expr =
		cypher_ast_set_all_properties_get_identifier(set_item);
	ASSERT(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
	const char *alias = cypher_ast_identifier_get_name(prop_expr);

	// Property map
	const cypher_astnode_t *ast_map =
		cypher_ast_set_all_properties_get_expression(set_item);
	if(cypher_astnode_type(ast_map) != CYPHER_AST_MAP) {
		// TODO introduce support for queries like:
		// MATCH (a {v: 1}), (b {v: 2}) SET a = b
		ErrorCtx_SetError("RedisGraph does not currently support assigning graph entities to non-map values.");
		return;
	}

	// retrieve or instantiate an update context
	int len = strlen(alias);
	EntityUpdateEvalCtx *ctx = raxFind(updates, (unsigned char *)alias, len);
	if(ctx == raxNotFound) {
		uint count = cypher_ast_map_nentries(ast_map);
		ctx = UpdateCtx_New(UPDATE_REPLACE, count, alias);
		raxInsert(updates, (unsigned char *)alias, len, ctx, NULL);
	} else {
		// we have enqueued updates that will no longer be committed; clear them
		UpdateCtx_Clear(ctx);
		UpdateCtx_SetMode(ctx, UPDATE_REPLACE);
	}

	// add all properties to update context
	_UpdateCtx_AddPropertyMap(gc, ctx, ast_map);
}

static void _UpdateCtx_AddProperty(GraphContext *gc, EntityUpdateEvalCtx *ctx,
		const cypher_astnode_t *set_item) {
	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);

	// Property name
	const cypher_astnode_t *ast_key = cypher_ast_property_operator_get_prop_name(ast_prop);
	const char *attribute = cypher_ast_prop_name_get_value(ast_key);
	Attribute_ID attribute_id = GraphContext_FindOrAddAttribute(gc, attribute);

	// Updated value
	const cypher_astnode_t *ast_val = cypher_ast_set_property_get_expression(set_item);
	AR_ExpNode *exp = AR_EXP_FromASTNode(ast_val);

	PropertySetCtx update = { .id  = attribute_id, .exp = exp };
	ctx->properties = array_append(ctx->properties, update);
}

// Set a single property
static void _Update_SetProperty(GraphContext *gc, rax *updates,
		const cypher_astnode_t *set_item) {
	// The SET_ITEM contains the entity alias and property key being set
	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);

	// Entity alias
	const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(ast_prop);
	ASSERT(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
	const char *alias = cypher_ast_identifier_get_name(prop_expr);
	int len = strlen(alias);

	// retrieve or instantiate an update context
	EntityUpdateEvalCtx *ctx = raxFind(updates, (unsigned char *)alias, len);
	if(ctx == raxNotFound) {
		ctx = UpdateCtx_New(UPDATE_MERGE, 1, alias);
		raxInsert(updates, (unsigned char *)alias, len, ctx, NULL);
	}

	// add property to update context
	_UpdateCtx_AddProperty(gc, ctx, set_item);
}

static void _ConvertSetItem(GraphContext *gc, rax *updates, const cypher_astnode_t *set_item) {
	const cypher_astnode_type_t type = cypher_astnode_type(set_item);
	if(type == CYPHER_AST_SET_ALL_PROPERTIES) {
		// MATCH (a) SET a = {v: 5}
		_Update_SetPropertyMap(gc, updates, set_item);
	} else if(type == CYPHER_AST_MERGE_PROPERTIES) {
		// MATCH (a) SET a += {v: 5}
		_Update_MergePropertyMap(gc, updates, set_item);
	} else if(type == CYPHER_AST_SET_PROPERTY) {
		// MATCH (a) SET a.v = 5
		_Update_SetProperty(gc, updates, set_item);
	} else {
		ASSERT(false);
	}
}

void AST_PreparePathCreation(const cypher_astnode_t *path, const QueryGraph *qg, rax *bound_vars,
							 NodeCreateCtx **nodes, EdgeCreateCtx **edges) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	QueryGraph *g = QueryGraph_ExtractPaths(qg, &path, 1);
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

		if((i % 2) == 1) {
			// relation
			QGEdge *e = QueryGraph_GetEdgeByAlias(g, alias);
			EdgeCreateCtx new_edge = _NewEdgeCreateCtx(gc, e, elem);
			*edges = array_append(*edges, new_edge);
		} else {
			// node
			QGNode *n = QueryGraph_GetNodeByAlias(g, alias);
			NodeCreateCtx new_node = _NewNodeCreateCtx(gc, n, elem);
			*nodes = array_append(*nodes, new_node);
		}
	}

	QueryGraph_Free(g);
}

//------------------------------------------------------------------------------
// SORT operation
//------------------------------------------------------------------------------

// Get direction of each sort operation, append to an array, return the array in the form of out parameter
void AST_PrepareSortOp(const cypher_astnode_t *order_clause, int **sort_directions) {
	ASSERT(order_clause && sort_directions);

	unsigned int nitems = cypher_ast_order_by_nitems(order_clause);
	int *directions = array_new(int, nitems);

	for(unsigned int i = 0; i < nitems; i ++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		int direction = cypher_ast_sort_item_is_ascending(item) ? DIR_ASC : DIR_DESC;
		directions = array_append(directions, direction);
	}

	*sort_directions = directions;
}

//------------------------------------------------------------------------------
// UNWIND operation
//------------------------------------------------------------------------------

AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause) {
	const cypher_astnode_t *collection = cypher_ast_unwind_get_expression(unwind_clause);
	AR_ExpNode *exp = AR_EXP_FromASTNode(collection);
	exp->resolved_name = cypher_ast_identifier_get_name(cypher_ast_unwind_get_alias(unwind_clause));

	AST_UnwindContext ctx = { .exp = exp };
	return ctx;
}

//------------------------------------------------------------------------------
// DELETE operation
//------------------------------------------------------------------------------

AR_ExpNode **AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause) {
	uint delete_count = cypher_ast_delete_nexpressions(delete_clause);
	AR_ExpNode **exps = array_new(AR_ExpNode *, delete_count);

	for(uint i = 0; i < delete_count; i ++) {
		const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, i);
		AR_ExpNode *exp = AR_EXP_FromASTNode(ast_expr);
		exps = array_append(exps, exp);
	}
	return exps;
}

//------------------------------------------------------------------------------
// MERGE operation
//------------------------------------------------------------------------------

AST_MergeContext AST_PrepareMergeOp(const cypher_astnode_t *merge_clause, GraphContext *gc,
									QueryGraph *qg, rax *bound_vars) {
	AST_MergeContext merge_ctx = { .nodes_to_merge = NULL,
								   .edges_to_merge = NULL,
								   .on_match = NULL,
								   .on_create = NULL
								 };

	// Prepare all create contexts for nodes and edges on Merge path.
	rax *on_match_items = NULL;
	rax *on_create_items = NULL;
	NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, 1);
	EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, 1);
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);

	// Shouldn't operate on the original bound variables map, as this function may insert aliases.
	rax *bound_and_introduced_entities = (bound_vars) ? raxClone(bound_vars) : raxNew();
	AST_PreparePathCreation(path, qg, bound_and_introduced_entities, &nodes_to_merge, &edges_to_merge);
	raxFree(bound_and_introduced_entities);


	// Convert any ON MATCH and ON CREATE directives.
	uint directive_count = cypher_ast_merge_nactions(merge_clause);

	for(uint i = 0; i < directive_count; i ++) {
		const cypher_astnode_t *directive = cypher_ast_merge_get_action(merge_clause, i);
		cypher_astnode_type_t type = cypher_astnode_type(directive);

		if(type == CYPHER_AST_ON_CREATE) {
			uint create_prop_count = cypher_ast_on_create_nitems(directive);
			if(on_create_items == NULL) on_create_items = raxNew();
			for(uint j = 0; j < create_prop_count; j ++) {
				const cypher_astnode_t *create_item = cypher_ast_on_create_get_item(directive, j);
				_ConvertSetItem(gc, on_create_items, create_item);
			}
		} else if(type == CYPHER_AST_ON_MATCH) {
			uint match_prop_count = cypher_ast_on_match_nitems(directive);
			if(on_match_items == NULL) on_match_items = raxNew();
			for(uint j = 0; j < match_prop_count; j ++) {
				const cypher_astnode_t *match_item = cypher_ast_on_match_get_item(directive, j);
				_ConvertSetItem(gc, on_match_items, match_item);
			}
		} else {
			ASSERT(false);
		}
	}

	merge_ctx.on_match = on_match_items;
	merge_ctx.on_create = on_create_items;
	merge_ctx.edges_to_merge = edges_to_merge;
	merge_ctx.nodes_to_merge = nodes_to_merge;
	return merge_ctx;
}

//------------------------------------------------------------------------------
// UPDATE operation
//------------------------------------------------------------------------------

rax *AST_PrepareUpdateOp(GraphContext *gc, const cypher_astnode_t *set_clause) {
	rax *updates = raxNew(); // entity alias -> EntityUpdateEvalCtx 
	uint nitems = cypher_ast_set_nitems(set_clause);

	for(uint i = 0; i < nitems; i++) {
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		_ConvertSetItem(gc, updates, set_item);
	}

	return updates;
}

//------------------------------------------------------------------------------
// CREATE operation
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

