/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast.h"
#include "../query_ctx.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/sds/sds.h"
#include "../procedures/procedure.h"

//------------------------------------------------------------------------------
//  Annotation context - WITH/RETURN * projections
//------------------------------------------------------------------------------
static void _collect_aliases_in_path
(
	const cypher_astnode_t *path,
	const cypher_astnode_t ***identifiers
) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// every even offset corresponds to a node
	for(uint i = 0; i < path_len; i += 2) {
		const cypher_astnode_t *ast_node =
			cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias =
			cypher_ast_node_pattern_get_identifier(ast_node);

		if(ast_alias == NULL) continue;  // unaliased node, do nothing

		// add node alias to projection array
		array_append(*identifiers, ast_alias);
	}

	// every odd offset corresponds to an edge
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *ast_edge =
			cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias =
			cypher_ast_rel_pattern_get_identifier(ast_edge);

		if(ast_alias == NULL) continue;  // unaliased edge, do nothing

		// add edge alias to projection array
		array_append(*identifiers, ast_alias);
	}
}

static void _collect_aliases_in_pattern
(
	const cypher_astnode_t *pattern,
	const cypher_astnode_t ***aliases
) {
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		_collect_aliases_in_path(cypher_ast_pattern_get_path(pattern, i),
								 aliases);
	}
}

static void _collect_with_projections
(
	const cypher_astnode_t *with_clause,
	const cypher_astnode_t ***identifiers
) {
	uint projection_count = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection =
			cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *identifier =
			cypher_ast_projection_get_alias(projection);
		if(identifier == NULL) {
			// the projection was not aliased
			// so the projection itself must be an identifier
			identifier = cypher_ast_projection_get_expression(projection);
			ASSERT(cypher_astnode_type(identifier) == CYPHER_AST_IDENTIFIER);
		}
		array_append(*identifiers, identifier);
	}
}

// sort an array of strings and remove duplicate entries.
static void _uniqueArray(const cypher_astnode_t **arr) {
#define MODIFIES_ISLT(a,b) (strcmp(cypher_ast_identifier_get_name(*a), \
            cypher_ast_identifier_get_name(*b)) < 0)
	int count = array_len(arr);
	if(count == 0) return;

	QSORT(const cypher_astnode_t *, arr, count, MODIFIES_ISLT);

	uint unique_idx = 0;
	for(int i = 0; i < count - 1; i ++) {
		if(strcmp(cypher_ast_identifier_get_name(arr[i]),
				  cypher_ast_identifier_get_name(arr[i + 1]))) {
			arr[unique_idx++] = arr[i];
		}
	}
	arr[unique_idx++] = arr[count - 1];
	array_trimm_len(arr, unique_idx);
}

static void _collect_call_projections(
	const cypher_astnode_t *call_clause,
	const cypher_astnode_t ***aliases
) {
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	if(yield_count == 0) {
		// if the procedure call is missing its yield part, include procedure outputs
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		ProcedureCtx *proc = Proc_Get(proc_name);
		// the procedure may not exist here, as we have not yet validated
		// that its name refers to a valid procedure
		if(proc == NULL) return;

		uint output_count = Procedure_OutputCount(proc);
		struct cypher_input_range range = { 0 };
		for(uint i = 0; i < output_count; i++) {
			const char *name = Procedure_GetOutput(proc, i);
			cypher_astnode_t *identifier = cypher_ast_identifier(name,
																 strlen(name), range);
			array_append(*aliases, identifier);
		}
		Proc_Free(proc);
		return;
	}

	for(uint i = 0; i < yield_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node == NULL) alias_node = ast_exp;

		array_append(*aliases, alias_node);
	}
}

static const cypher_astnode_t **_collect_aliases_in_scope
(
	const cypher_astnode_t *root,
	uint scope_start,
	uint scope_end
) {
	ASSERT(scope_start != scope_end);
	const cypher_astnode_t **aliases = array_new(const cypher_astnode_t *, 1);

	for(uint i = scope_start; i < scope_end; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);

		if(type == CYPHER_AST_WITH) {
			// the WITH clause contains either
			// aliases or its own STAR projection
			_collect_with_projections(clause, &aliases);
		} else if(type == CYPHER_AST_MATCH) {
			// the MATCH clause contains one pattern of N paths
			const cypher_astnode_t *pattern =
				cypher_ast_match_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_CREATE) {
			// the CREATE clause contains one pattern of N paths
			const cypher_astnode_t *pattern =
				cypher_ast_create_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_MERGE) {
			// the MERGE clause contains one path
			const cypher_astnode_t *path =
				cypher_ast_merge_get_pattern_path(clause);
			_collect_aliases_in_path(path, &aliases);
		} else if(type == CYPHER_AST_UNWIND) {
			// the UNWIND clause introduces one alias
			const cypher_astnode_t *unwind_alias =
				cypher_ast_unwind_get_alias(clause);
			array_append(aliases, unwind_alias);
		} else if(type == CYPHER_AST_CALL) {
			_collect_call_projections(clause, &aliases);
		}
	}

	// sort and unique the aliases array
	// so that we won't make redundant projections given queries like:
	// MATCH (a)-[]->(a) RETURN *
	_uniqueArray(aliases);

	return aliases;
}

static void replace_clause
(
	cypher_astnode_t *root,
	cypher_astnode_t *clause,
	int scope_start,
	int idx
) {
	// collect all aliases defined in this scope
	const cypher_astnode_t **aliases = _collect_aliases_in_scope(root,
																 scope_start, idx);
	uint alias_count = array_len(aliases);
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	uint other_projections_count = (t == CYPHER_AST_WITH) ?
								   cypher_ast_with_nprojections(clause) :
								   cypher_ast_return_nprojections(clause);

	cypher_astnode_t *projections[alias_count == 0 ? 1 :
											  alias_count + other_projections_count];

	// convert aliases to expressions
	for(uint i = 0; i < alias_count; i++) {
		cypher_astnode_t         *expression = cypher_ast_clone(aliases[i]);
		const cypher_astnode_t   *alias      = NULL;
		cypher_astnode_t        **children   = &expression;
		unsigned int              nchildren  = 1;
		struct cypher_input_range range      = cypher_astnode_range(aliases[i]);

		projections[i] = cypher_ast_projection(expression, alias, children,
											   nchildren, range);
	}

	if(alias_count == 0) {
		if(t == CYPHER_AST_RETURN) {
			// error if this is a RETURN clause with no aliases
			ErrorCtx_SetError("RETURN * is not allowed when there are no variables in scope");
			array_free(aliases);
			return;
		} else {
			// build an empty projection
			// to make variable-less WITH clauses work:
			// MATCH () WITH * CREATE ()
			struct cypher_input_range range = { 0 };
			// build a null node to project and an empty identifier as its alias
			cypher_astnode_t *expression = cypher_ast_null(range);
			cypher_astnode_t *identifier = cypher_ast_identifier("", 1, range);
			cypher_astnode_t *children[2];
			children[0] = expression;
			children[1] = identifier;
			projections[0] = cypher_ast_projection(expression, identifier,
												   children, 2, range);
			alias_count = 1;
		}
	}

	uint nprojections = alias_count;
	// clone non-star projections into projections array
	for(uint i = 0; i < other_projections_count; i ++) {
		const cypher_astnode_t *projection = (t == CYPHER_AST_WITH) ?
											 cypher_ast_with_get_projection(clause, i) :
											 cypher_ast_return_get_projection(clause, i);
		// if the projection has an alias use it,
		// otherwise the expression is the alias
		const cypher_astnode_t *alias = cypher_ast_projection_get_alias(projection);
		const cypher_astnode_t *exp = alias ? alias :
									  cypher_ast_projection_get_expression(projection);
		ASSERT(cypher_astnode_type(exp) == CYPHER_AST_IDENTIFIER);
		bool duplicate = false;
		if(cypher_astnode_type(aliases[i]) == CYPHER_AST_IDENTIFIER) {
			const char *alias = cypher_ast_identifier_get_name(exp);
			// don't introduce duplicates of existing aliases
			for(uint j = 0; j < alias_count; j ++) {
				const char *other = cypher_ast_identifier_get_name(aliases[j]);
				if(strcmp(alias, other) == 0) {
					duplicate = true;
					break;
				}
			}
		}
		if(duplicate) continue;
		projections[alias_count + i] = cypher_ast_clone(projection);
		nprojections++;
	}
	array_free(aliases);

	// prepare arguments for new return clause node
	bool                    distinct   =  false ;
	const cypher_astnode_t  *skip      =  NULL  ;
	const cypher_astnode_t  *limit     =  NULL  ;
	const cypher_astnode_t  *order_by  =  NULL  ;

	if(t == CYPHER_AST_WITH) {
		distinct      =  cypher_ast_with_is_distinct(clause);
		skip          =  cypher_ast_with_get_skip(clause);
		limit         =  cypher_ast_with_get_limit(clause);
		order_by      =  cypher_ast_with_get_order_by(clause);
	} else {
		distinct      =  cypher_ast_return_is_distinct(clause);
		skip          =  cypher_ast_return_get_skip(clause);
		limit         =  cypher_ast_return_get_limit(clause);
		order_by      =  cypher_ast_return_get_order_by(clause);
	}

	// copy projections to the children array
	cypher_astnode_t *children[nprojections + 3];
	for(uint i = 0; i < nprojections; i++) {
		children[i] = projections[i];
	}

	// clone any ORDER BY, SKIP, and LIMIT modifiers to
	// add to the children array and populate the new clause
	uint nchildren = nprojections;
	if(order_by) order_by = children[nchildren++] = cypher_ast_clone(order_by);
	if(skip)     skip     = children[nchildren++] = cypher_ast_clone(skip);
	if(limit)    limit    = children[nchildren++] = cypher_ast_clone(limit);

	struct cypher_input_range range = cypher_astnode_range(clause);

	// build the replacement clause
	cypher_astnode_t *new_clause;
	if(t == CYPHER_AST_WITH) {
		new_clause = cypher_ast_with(distinct,
									 false,
									 projections,
									 nprojections,
									 order_by,
									 skip,
									 limit,
									 NULL,
									 children,
									 nchildren,
									 range);
	} else {
		new_clause = cypher_ast_return(distinct,
									   false,
									   projections,
									   nprojections,
									   order_by,
									   skip,
									   limit,
									   children,
									   nchildren,
									   range);
	}

	cypher_ast_free(clause);
	// replace original clause with fully populated one
	cypher_ast_query_set_clause(root, new_clause, idx);
}

void AST_RewriteStarProjections
(
	cypher_parse_result_t *result
) {
	// retrieve the statement node
	const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
	if(cypher_astnode_type(statement) != CYPHER_AST_STATEMENT) return;

	// retrieve the root query node from the statement
	const cypher_astnode_t *root = cypher_ast_statement_get_body(statement);
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return;

	// rewrite all WITH * / RETURN * clauses to include all aliases
	uint  scope_start   =  0;
	uint  clause_count  =  cypher_ast_query_nclauses(root);

	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);
		if(t != CYPHER_AST_WITH && t != CYPHER_AST_RETURN) continue;

		bool has_include_existing = (t == CYPHER_AST_WITH) ?
									cypher_ast_with_has_include_existing(clause) :
									cypher_ast_return_has_include_existing(clause);

		if(has_include_existing) {
			// clause contains a star projection, replace it
			replace_clause((cypher_astnode_t *)root, (cypher_astnode_t *)clause,
						   scope_start, i);
		}

		// update scope start
		scope_start = i;
	}
}

