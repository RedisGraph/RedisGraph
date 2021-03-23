#include "ast.h"
#include "RG.h"
#include "../util/arr.h"

// Forward declerations:
static void _AST_MapReferencedEntitiesInPath(AST *ast, const cypher_astnode_t *path);

// Adds an identifier or an alias to the reference map.
static inline void _AST_UpdateRefMap(AST *ast, const char *name) {
	raxInsert(ast->referenced_entities, (unsigned char *)name, strlen(name), NULL, NULL);
}

// Map identifiers within an expression.
static void _AST_MapExpression(AST *ast, const cypher_astnode_t *exp) {
	cypher_astnode_type_t type = cypher_astnode_type(exp);

	// In case of identifier.
	if(type == CYPHER_AST_IDENTIFIER) {
		const char *identifier_name = cypher_ast_identifier_get_name(exp);
		_AST_UpdateRefMap(ast, identifier_name);
	} else if(type == CYPHER_AST_PATTERN_PATH) {
		// In case of pattern filter.
		_AST_MapReferencedEntitiesInPath(ast, exp);
	} else if(type == CYPHER_AST_SHORTEST_PATH) {
		// Reference all entity names in a shortest path.
		_AST_MapReferencedEntitiesInPath(ast, exp);
	} else {
		// Recurse over children.
		uint child_count = cypher_astnode_nchildren(exp);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(exp, i);
			// Recursively continue mapping.
			_AST_MapExpression(ast, child);
		}
	}
}

// Maps the RHS of "AS" projected entities.
static inline void _AST_MapProjectionAlias(AST *ast, const cypher_astnode_t *projection) {
	const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(projection);
	if(ast_alias == NULL) {
		// The projection was not aliased, so the projection itself must be an identifier.
		ast_alias = cypher_ast_projection_get_expression(projection);
		ASSERT(cypher_astnode_type(ast_alias) == CYPHER_AST_IDENTIFIER);
	}
	// WITH and RETURN projections are always either aliased or themselves identifiers.
	const char *alias = cypher_ast_identifier_get_name(ast_alias);
	_AST_UpdateRefMap(ast, alias);
}

// Adds referenced entities of ORDER BY clause.
static void _AST_MapOrderByReferences(AST *ast, const cypher_astnode_t *order_by) {
	uint count = cypher_ast_order_by_nitems(order_by);
	// Go over each order by expression.
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_by, i);
		const cypher_astnode_t *expression = cypher_ast_sort_item_get_expression(item);
		_AST_MapExpression(ast, expression);
	}
}

// Adds a node to the referenced entities rax, in case it has labels or properties (inline filter).
static void _AST_MapReferencedNode(AST *ast, const cypher_astnode_t *node, bool force_mapping) {

	const cypher_astnode_t *properties = cypher_ast_node_pattern_get_properties(node);
	// Disregard empty property maps.
	if(properties && cypher_astnode_nchildren(properties) == 0) properties = NULL;
	// A node with inlined filters is always referenced for the FilterTree.
	// (In the case of a CREATE path, these are properties being set)
	if(properties || force_mapping) {
		const char *alias = AST_GetEntityName(ast, node);
		_AST_UpdateRefMap(ast, alias);

		// Map any references within the properties map, such as 'b' in:
		// ({val: ID(b)})
		if(properties) _AST_MapExpression(ast, properties);
	}
}

static void _AST_MapReferencedEdge_RelationPattern(AST *ast, const cypher_astnode_t *edge, bool force_mapping) {
	const cypher_astnode_t *properties = cypher_ast_rel_pattern_get_properties(edge);
	// Disregard empty property maps.
	if(properties && cypher_astnode_nchildren(properties) == 0) properties = NULL;
	// An edge with inlined filters is always referenced for the FilterTree.
	// (In the case of a CREATE path, these are properties being set)
	if(properties || force_mapping) {
		const char *alias = AST_GetEntityName(ast, edge);
		_AST_UpdateRefMap(ast, alias);

		// Map any references within the properties map, such as 'b' in:
		// ({val: ID(b)})
		if(properties) _AST_MapExpression(ast, properties);
	}
}

// Adds an edge to the referenced entities rax if it has multiple types or any properties (inline filter).
static void _AST_MapReferencedEdge(AST *ast, const cypher_astnode_t *edge, bool force_mapping) {
	if (cypher_astnode_instanceof(edge, CYPHER_AST_REL_PATTERN)) {
		_AST_MapReferencedEdge_RelationPattern(ast, edge, force_mapping);
	} else if (cypher_astnode_instanceof(edge, CYPHER_AST_PATH_PATTERN)) {
		// Now there is not aliases in path patterns, we don't need to collect them
	} else {
		ASSERT(false);
	}
}

// Maps entities in a given path.
static void _AST_MapReferencedEntitiesInPath(AST *ast, const cypher_astnode_t *path) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Check if the path is a named path or shortest path.
	// If so, map all entities, else map only referenced entities.
	const cypher_astnode_type_t type = cypher_astnode_type(path);
	bool force_mapping = (type == CYPHER_AST_NAMED_PATH || type == CYPHER_AST_SHORTEST_PATH);
	// Node are in even positions.
	for(uint i = 0; i < path_len; i += 2)
		_AST_MapReferencedNode(ast, cypher_ast_pattern_path_get_element(path, i), force_mapping);
	// Edges are in odd positions.
	for(uint i = 1; i < path_len; i += 2)
		_AST_MapReferencedEdge(ast, cypher_ast_pattern_path_get_element(path, i), force_mapping);
}

// Add referenced aliases from MATCH clause - inline filtered and explicit WHERE filter.
static void _AST_MapMatchClauseReferences(AST *ast, const cypher_astnode_t *match_clause) {
	// Inline filters.
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match_clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapReferencedEntitiesInPath(ast, path);
	}

	// Where clause.
	const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(match_clause);
	if(predicate) _AST_MapExpression(ast, predicate);
}

// Add referenced aliases from CREATE clause.
static void _AST_MapCreateClauseReferences(AST *ast, const cypher_astnode_t *create_clause) {
	const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(create_clause);
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		_AST_MapReferencedEntitiesInPath(ast, path);
	}
}

// Maps entities in SET property clause.
static void _AST_MapSetPropertyReferences(AST *ast, const cypher_astnode_t *set_item) {
	// Retrieve the alias being modified from the property descriptor.
	const cypher_astnode_t *ast_prop = cypher_ast_set_property_get_property(set_item);
	const cypher_astnode_t *ast_entity = cypher_ast_property_operator_get_expression(ast_prop);
	ASSERT(cypher_astnode_type(ast_entity) == CYPHER_AST_IDENTIFIER);

	const char *alias = cypher_ast_identifier_get_name(ast_entity);
	_AST_UpdateRefMap(ast, alias);

	// Map expression right hand side, e.g. a.v = 1, a.x = b.x
	const cypher_astnode_t *set_exp = cypher_ast_set_property_get_expression(set_item);
	_AST_MapExpression(ast, set_exp);
}

// Maps entities in SET clause.
static void _AST_MapSetClauseReferences(AST *ast, const cypher_astnode_t *set_clause) {
	uint nitems = cypher_ast_set_nitems(set_clause);
	for(uint i = 0; i < nitems; i++) {
		// Get the SET directive at this index.
		const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, i);
		ASSERT(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
		_AST_MapSetPropertyReferences(ast, set_item);
	}
}

// Maps entities in DELETE clause.
static void _AST_MapDeleteClauseReferences(AST *ast, const cypher_astnode_t *delete_clause) {
	uint nitems = cypher_ast_delete_nexpressions(delete_clause);
	for(uint i = 0; i < nitems; i++) {
		const cypher_astnode_t *delete_exp = cypher_ast_delete_get_expression(delete_clause, i);
		_AST_MapExpression(ast, delete_exp);
	}
}

// Maps entities in MERGE clause. Either by implicit filters, or modified entities by SET clause.
static void _AST_MapMergeClauseReference(AST *ast, const cypher_astnode_t *merge_clause) {
	// Collect implicitly filtered entities.
	const cypher_astnode_t *merge_path = cypher_ast_merge_get_pattern_path(merge_clause);
	_AST_MapReferencedEntitiesInPath(ast, merge_path);

	// Map modified entities, either by ON MATCH or ON CREATE clause.
	uint merge_actions = cypher_ast_merge_nactions(merge_clause);
	for(uint i = 0; i < merge_actions; i++) {
		const cypher_astnode_t *action = cypher_ast_merge_get_action(merge_clause, i);
		cypher_astnode_type_t type = cypher_astnode_type(action);
		// ON CREATE.
		if(type == CYPHER_AST_ON_CREATE) {
			uint on_create_items = cypher_ast_on_create_nitems(action);
			for(uint j = 0; j < on_create_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_create_get_item(action, j);
				ASSERT(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		} else if(type == CYPHER_AST_ON_MATCH) {
			// ON MATCH.
			uint on_match_items = cypher_ast_on_match_nitems(action);
			for(uint j = 0; j < on_match_items; j ++) {
				const cypher_astnode_t *set_item = cypher_ast_on_match_get_item(action, j);
				ASSERT(cypher_astnode_type(set_item) == CYPHER_AST_SET_PROPERTY);
				_AST_MapSetPropertyReferences(ast, set_item);
			}
		}
	}
}

// Map the LHS of "AS" projected entities. "WITH a as x order by x" will just collect a to the map.
static void _AST_MapWithReferredEntities(AST *ast_segment, const cypher_astnode_t *with_clause) {
	int projectionCount = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0 ; i < projectionCount; i ++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		// The expression forms the LHS of the projection.
		const cypher_astnode_t *exp = cypher_ast_projection_get_expression(projection);
		_AST_MapExpression(ast_segment, exp);
	}
	// Add referenced aliases for WITH's ORDER BY entities.
	const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(with_clause);
	if(order_by) _AST_MapOrderByReferences(ast_segment, order_by);
}

// Map the LHS of "AS" projected entities. "RETURN a as x order by x" will just collect a to the map.
static void _AST_MapReturnReferredEntities(AST *ast_segment,
										   const cypher_astnode_t *return_clause) {
	// Add referenced aliases for RETURN projections.
	uint projectionCount = cypher_ast_return_nprojections(return_clause);
	for(uint i = 0 ; i < projectionCount; i ++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		// The expression forms the LHS of the projection.
		const cypher_astnode_t *exp = cypher_ast_projection_get_expression(projection);
		_AST_MapExpression(ast_segment, exp);
	}
	// Add referenced aliases for RETURN's ORDER BY entities.
	const cypher_astnode_t *order_by = cypher_ast_return_get_order_by(return_clause);
	if(order_by) _AST_MapOrderByReferences(ast_segment, order_by);
}

static void _AST_MapProjectAll(AST *ast, const cypher_astnode_t *project_clause) {
	const char **aliases = AST_GetProjectAll(project_clause);
	uint alias_count = array_len(aliases);
	for(uint i = 0; i < alias_count; i ++) {
		_AST_UpdateRefMap(ast, aliases[i]);
	}
}

static void _ASTClause_BuildReferenceMap(AST *ast, const cypher_astnode_t *clause) {
	if(!clause) return;

	cypher_astnode_type_t type = cypher_astnode_type(clause);
	if(type == CYPHER_AST_RETURN) {
		// Add referenced aliases for RETURN projections.
		uint projectionCount = cypher_ast_return_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionAlias(ast, cypher_ast_return_get_projection(clause, i));
		}
		// Add referenced aliases for RETURN's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_return_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);
	} else if(type == CYPHER_AST_WITH) {
		// Add referenced aliases for WITH projections.
		uint projectionCount = cypher_ast_with_nprojections(clause);
		for(uint i = 0 ; i < projectionCount; i ++) {
			_AST_MapProjectionAlias(ast, cypher_ast_with_get_projection(clause, i));
		}
		// Add referenced aliases for WITH's ORDER BY entities.
		const cypher_astnode_t *order_by = cypher_ast_with_get_order_by(clause);
		if(order_by) _AST_MapOrderByReferences(ast, order_by);
	} else if(type == CYPHER_AST_MATCH) {
		// Add referenced aliases from MATCH clause - inline filtered and explicit WHERE filter.
		_AST_MapMatchClauseReferences(ast, clause);
	} else if(type == CYPHER_AST_CREATE) {
		// Add referenced aliases for CREATE clause.
		_AST_MapCreateClauseReferences(ast, clause);
	} else if(type == CYPHER_AST_MERGE) {
		// Add referenced aliases for MERGE clause - inline filtered and modified entities.
		_AST_MapMergeClauseReference(ast, clause);
	} else if(type == CYPHER_AST_SET) {
		// Add referenced aliases for SET clause.
		_AST_MapSetClauseReferences(ast, clause);
	} else if(type == CYPHER_AST_DELETE) {
		// Add referenced aliases for DELETE clause.
		_AST_MapDeleteClauseReferences(ast, clause);
	}
}

// Map the referred aliases (LHS) in entities projected by a WITH or RETURN clause.
static void _AST_MapProjectionClause(AST *ast_segment, const cypher_astnode_t *projection) {
	cypher_astnode_type_t type = cypher_astnode_type(projection);
	ASSERT(type == CYPHER_AST_WITH || type == CYPHER_AST_RETURN);

	if(type == CYPHER_AST_WITH) {
		// If the projection clause is WITH *, all user-defined aliases are referenced.
		if(cypher_ast_with_has_include_existing(projection)) _AST_MapProjectAll(ast_segment, projection);
		else _AST_MapWithReferredEntities(ast_segment, projection);
	} else {
		// If the projection clause is RETURN *, all user-defined aliases are referenced.
		if(cypher_ast_return_has_include_existing(projection)) _AST_MapProjectAll(ast_segment, projection);
		else _AST_MapReturnReferredEntities(ast_segment, projection);
	}
}

// Populate the AST's map of all referenced aliases.
void AST_BuildReferenceMap(AST *ast, const cypher_astnode_t *project_clause) {
	ast->referenced_entities = raxNew();

	// If this segment is followed by a projection clause, map that clause's references.
	if(project_clause) _AST_MapProjectionClause(ast, project_clause);

	// Check every clause in this AST.
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		_ASTClause_BuildReferenceMap(ast, clause);
	}
}

