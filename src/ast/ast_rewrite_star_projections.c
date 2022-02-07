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
static void _collect_aliases_in_path(const cypher_astnode_t *path, const char ***aliases) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	// Every even offset corresponds to a node.
	for(uint i = 0; i < path_len; i += 2) {
		const cypher_astnode_t *ast_node = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias = cypher_ast_node_pattern_get_identifier(ast_node);
		if(ast_alias == NULL) continue;  // Unaliased node, do nothing.
		// Add node alias to projection array.
		array_append(*aliases, cypher_ast_identifier_get_name(ast_alias));
	}

	// Every odd offset corresponds to an edge.
	for(uint i = 1; i < path_len; i += 2) {
		const cypher_astnode_t *ast_edge = cypher_ast_pattern_path_get_element(path, i);
		const cypher_astnode_t *ast_alias = cypher_ast_rel_pattern_get_identifier(ast_edge);
		if(ast_alias == NULL) continue;  // Unaliased edge, do nothing.
		// Add edge alias to projection array.
		array_append(*aliases, cypher_ast_identifier_get_name(ast_alias));
	}
}

static void _collect_aliases_in_pattern(const cypher_astnode_t *pattern, const char ***aliases) {
	uint path_count = cypher_ast_pattern_npaths(pattern);
	for(uint i = 0; i < path_count; i ++) {
		_collect_aliases_in_path(cypher_ast_pattern_get_path(pattern, i), aliases);
	}
}

static void _collect_with_projections(const cypher_astnode_t *with_clause, const char ***aliases) {
	uint projection_count = cypher_ast_with_nprojections(with_clause);
	for(uint i = 0; i < projection_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const char *alias = NULL;
		const cypher_astnode_t *identifier = cypher_ast_projection_get_alias(projection);
		if(identifier == NULL) {
			// The projection was not aliased, so the projection itself must be an identifier.
			identifier = cypher_ast_projection_get_expression(projection);
			ASSERT(cypher_astnode_type(identifier) == CYPHER_AST_IDENTIFIER);
		}
		alias = cypher_ast_identifier_get_name(identifier);

		array_append(*aliases, alias);
	}
}

// Sort an array of strings and remove duplicate entries.
static void _uniqueArray(const char **arr) {
#define MODIFIES_ISLT(a,b) (strcmp((*a),(*b)) < 0)
	int count = array_len(arr);
	if(count == 0) return;
	QSORT(const char *, arr, count, MODIFIES_ISLT);
	uint unique_idx = 0;
	for(int i = 0; i < count - 1; i ++) {
		if(strcmp(arr[i], arr[i + 1])) {
			arr[unique_idx++] = arr[i];
		}
	}
	arr[unique_idx++] = arr[count - 1];
	array_trimm_len(arr, unique_idx);
}

static void _collect_call_projections(const cypher_astnode_t *call_clause, const char ***aliases) {
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	if(yield_count == 0) {
		// If the procedure call is missing its yield part, include procedure outputs.
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		ProcedureCtx *proc = Proc_Get(proc_name);
		ASSERT(proc);

		uint output_count = Procedure_OutputCount(proc);
		for(uint i = 0; i < output_count; i++) {
			const char *name = Procedure_GetOutput(proc, i);
			array_append(*aliases, name);
		}
		Proc_Free(proc);
		return;
	}

	for(uint i = 0; i < yield_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node == NULL) alias_node = ast_exp;
		const char *identifier = cypher_ast_identifier_get_name(alias_node);

		array_append(*aliases, identifier);
	}
}

static const char **_collect_aliases_in_scope(const cypher_astnode_t *root,
											  uint scope_start, uint scope_end) {
	ASSERT(scope_start != scope_end);
	const char **aliases = array_new(const char *, 1);

	for(uint i = scope_start; i < scope_end; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_WITH) {
			// The WITH clause contains either aliases or its own STAR projection.
			_collect_with_projections(clause, &aliases);
		} else if(type == CYPHER_AST_MATCH) {
			// The MATCH clause contains one pattern of N paths.
			const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_CREATE) {
			// The CREATE clause contains one pattern of N paths.
			const cypher_astnode_t *pattern = cypher_ast_create_get_pattern(clause);
			_collect_aliases_in_pattern(pattern, &aliases);
		} else if(type == CYPHER_AST_MERGE) {
			// The MERGE clause contains one path.
			const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
			_collect_aliases_in_path(path, &aliases);
		} else if(type == CYPHER_AST_UNWIND) {
			// The UNWIND clause introduces one alias.
			const cypher_astnode_t *unwind_alias = cypher_ast_unwind_get_alias(clause);
			array_append(aliases, cypher_ast_identifier_get_name(unwind_alias));
		} else if(type == CYPHER_AST_CALL) {
			_collect_call_projections(clause, &aliases);
		}
	}

	// Sort and unique the aliases array so that we won't make redundant projections given queries like:
	// MATCH (a)-[]->(a) RETURN *
	_uniqueArray(aliases);

	return aliases;
}

static sds aliases_to_query_string(const char **aliases,
								   const cypher_astnode_t *clause) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	ASSERT(t == CYPHER_AST_WITH || t == CYPHER_AST_RETURN);

	sds s                                     =  NULL  ;
	bool                    distinct          =  false ;
	const cypher_astnode_t  *skip_clause      =  NULL  ;
	const cypher_astnode_t  *limit_clause     =  NULL  ;
	const cypher_astnode_t  *order_clause     =  NULL  ;
	if(t == CYPHER_AST_WITH) {
		s             =  sdsnew("WITH ");
		distinct      =  cypher_ast_with_is_distinct(clause);
		skip_clause   =  cypher_ast_with_get_skip(clause);
		limit_clause  =  cypher_ast_with_get_limit(clause);
		order_clause  =  cypher_ast_with_get_order_by(clause);
	} else {
		s             =  sdsnew("RETURN ");
		distinct      =  cypher_ast_return_is_distinct(clause);
		skip_clause   =  cypher_ast_return_get_skip(clause);
		limit_clause  =  cypher_ast_return_get_limit(clause);
		order_clause  =  cypher_ast_return_get_order_by(clause);
	}
	if(distinct) s = sdscat(s, "DISTINCT ");
	uint alias_count = array_len(aliases);
	// hack to make empty projections work:
	// MATCH () WITH * CREATE ()
	if(alias_count == 0 && t == CYPHER_AST_WITH) return sdscat(s, "NULL AS a");

	for(uint i = 0; i < alias_count - 1; i ++) {
		// append string with comma-separated aliases
		s = sdscatprintf(s, "%s, ", aliases[i]);
	}
	// append the last alias with no trailing comma
	s = sdscat(s, aliases[alias_count - 1]);

	// append ORDER BY, SKIP, and LIMIT modifiers to string
	if(order_clause) {
		const char *query = QueryCtx_GetQuery();
		struct cypher_input_range range = cypher_astnode_range(order_clause);
		// copy the ORDER BY segment of the query
		uint length = range.end.offset - range.start.offset;
		s = sdscat(s, " ");
		s = sdscatlen(s, query + range.start.offset, length);
	}
	if(skip_clause) s = sdscatprintf(s, " SKIP %s",
										 cypher_ast_integer_get_valuestr(skip_clause));
	if(limit_clause) s = sdscatprintf(s, " LIMIT %s",
										  cypher_ast_integer_get_valuestr(limit_clause));
	return s;
}

static cypher_astnode_t *build_clause_node(sds s) {
	// generate a new parse result
	cypher_parse_result_t *parse_result =
		cypher_parse(s, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	ASSERT(parse_result != NULL);
	ASSERT(cypher_parse_result_nroots(parse_result) == 1);
	// retrieve the AST root node from parsed query
	const cypher_astnode_t *statement = cypher_parse_result_get_root(parse_result, 0);
	ASSERT(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);

	const cypher_astnode_t *query = cypher_ast_statement_get_body(statement);
	ASSERT(cypher_astnode_type(query) == CYPHER_AST_QUERY);
	ASSERT(cypher_ast_query_nclauses(query) == 1);
	// clone the AST clause node so that the result can be freed
	const cypher_astnode_t *new_clause = cypher_ast_query_get_clause(query, 0);
	ASSERT(cypher_astnode_type(new_clause) == CYPHER_AST_WITH ||
		   cypher_astnode_type(new_clause) == CYPHER_AST_RETURN);
	cypher_astnode_t *new_clause_clone = cypher_ast_clone(new_clause);
	cypher_parse_result_free(parse_result);

	return new_clause_clone;
}

static void replace_clause(cypher_astnode_t *root, cypher_astnode_t *clause,
						   int scope_start, int idx) {
	// collect all aliases defined in this scope
	const char **aliases = _collect_aliases_in_scope(root, scope_start, idx);
	if(array_len(aliases) == 0 &&
	   cypher_astnode_type(clause) == CYPHER_AST_RETURN) {
		// error if this is a RETURN clause with no aliases
		ErrorCtx_SetError("RETURN * is not allowed when there are no variables in scope");
		array_free(aliases);
		return;
	}
	sds s = aliases_to_query_string(aliases, clause);
	cypher_astnode_t *new_clause = build_clause_node(s);
	sdsfree(s);
	array_free(aliases);
	// replace original clause with fully populated one
	cypher_astnode_free(clause);
	cypher_ast_query_set_clause(root, new_clause, idx);
	cypher_astnode_set_child(root, new_clause, idx);

}

void AST_RewriteStarProjections(cypher_parse_result_t *result) {
	const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
	if(cypher_astnode_type(statement) != CYPHER_AST_STATEMENT) return;
	const cypher_astnode_t *root = cypher_ast_statement_get_body(statement);
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return;
	// rewrite all WITH * clauses to include all aliases
	uint clause_count = cypher_ast_query_nclauses(root);
	uint scope_start = 0;
	uint scope_end;
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);
		if(t == CYPHER_AST_WITH || t == CYPHER_AST_RETURN) {
			bool has_include_existing = (t == CYPHER_AST_WITH) ?
										cypher_ast_with_has_include_existing(clause) :
										cypher_ast_return_has_include_existing(clause);
			if(has_include_existing) {
				replace_clause((cypher_astnode_t *)root,
							   (cypher_astnode_t *)clause, scope_start, i);
			}
			scope_start = i;
		}
	}
}

