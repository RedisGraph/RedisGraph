/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast_shared.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "ast_rewrite_call_subquery.h"

static bool _node_is_eager(const cypher_astnode_t *node);

// returns true if the given ast-node will result in an eager operation
static bool _clause_is_eager
(
	const cypher_astnode_t *clause
) {
	// -------------------------------------------------------------------------
	// check if clause type is one of: CREATE, MERGE, SET or REMOVE
	// -------------------------------------------------------------------------
	cypher_astnode_type_t type = cypher_astnode_type(clause);
	if(type == CYPHER_AST_CREATE_NODE_PROPS_INDEX     ||
	   type == CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT ||
	   type == CYPHER_AST_CREATE_REL_PROP_CONSTRAINT  ||
	   type == CYPHER_AST_CREATE                      ||
	   type == CYPHER_AST_MERGE                       ||
	   type == CYPHER_AST_SET                         ||
	   type == CYPHER_AST_REMOVE) {
		return true;
	}

	if(type == CYPHER_AST_CALL_SUBQUERY) {
		return _node_is_eager(clause);
	}

	// -------------------------------------------------------------------------
	// check if clause is a WITH or RETURN clause with an aggregation
	// -------------------------------------------------------------------------
	if(type == CYPHER_AST_RETURN || type == CYPHER_AST_WITH) {
		return AST_ClauseContainsAggregation(clause);
	}

	return false;
}

// returns true if the given node will result in an execution-plan that will
// contain an eager operation
static bool _node_is_eager
(
	const cypher_astnode_t *node  // ast-node
) {
	uint n_clauses;
	const cypher_astnode_t *(*get_clause)(const cypher_astnode_t *, uint);

	cypher_astnode_type_t type = cypher_astnode_type(node);
	if(type == CYPHER_AST_QUERY) {
		n_clauses = cypher_ast_query_nclauses(node);
		// get_clause = cypher_ast_query_get_clause;
	} else {
		n_clauses = cypher_ast_query_nclauses(
			cypher_ast_call_subquery_get_query(node));
		// get_clause = cypher_ast_call_subquery_get_clause;
	}

	for(uint i = 0; i < n_clauses; i++) {
		const cypher_astnode_t *curr_clause = type == CYPHER_AST_QUERY ?
			cypher_ast_query_get_clause(node, i) :
			cypher_ast_query_get_clause(
				cypher_ast_call_subquery_get_query(node), i);
		if(_clause_is_eager(curr_clause)) {
			return true;
		}
	}

	return false;
}

// fill `names` and `inner_names` with the bound vars and their internal
// representation, respectively
static void _get_vars_inner_rep
(
	rax *outer_mapping,  // rax containing bound vars
	char ***names,       // [OUTPUT] bound vars
	char ***inter_names  // [OUTPUT] internal representation of vars
) {
	ASSERT(outer_mapping != NULL);

	raxIterator it;
	raxStart(&it, outer_mapping);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		// avoid multiple internal representations of the same alias
		if(it.key[0] == '@') {
			continue;
		}
		char *curr = rm_strndup((const char *)it.key, it.key_len);
		// think about working with sds
		char *internal_rep = rm_malloc(it.key_len + 2);
		sprintf(internal_rep, "@%.*s", (int)it.key_len, it.key);
		// append original name
		array_append(*names, curr);
		// append internal (temporary) name
		array_append(*inter_names, internal_rep);
	}
}

// replaces a `WITH` clause, placed in clause_idx within `callsubquery`, with a
// new `WITH` clause containing projections of `names` to `inter_names`
static void _replace_with_clause
(
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint clause_idx,                 // index of clause to replace
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {

	const cypher_astnode_t *clause =
		cypher_ast_query_get_clause(
			cypher_ast_call_subquery_get_query(callsubquery), clause_idx);

	uint existing_projections_count = cypher_ast_with_nprojections(clause);
	uint n_projections = array_len(inter_names) + existing_projections_count;
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	uint n_names = array_len(names);
	uint n_inter_names = array_len(inter_names);
	for(uint i = 0; i < n_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
		int ind = n_inter_names - n_names + i;
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[ind],
			strlen(inter_names[ind]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// create projections for bound vars from outer context
	// -------------------------------------------------------------------------
	uint n_outer_names = n_inter_names - n_names;
	for(uint i = 0; i < n_outer_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// introduce explicit projections
	//--------------------------------------------------------------------------

	// clone explicit projections into projections array
	for(uint i = 0; i < existing_projections_count; i++) {
		const cypher_astnode_t *projection =
			cypher_ast_with_get_projection(clause, i);
		projections[proj_idx++] = cypher_ast_clone(projection);
	}

	// copy projections to the children array
	cypher_astnode_t *children[n_projections + 4];
	for(uint i = 0; i < n_projections; i++) {
		children[i] = projections[i];
	}

	// -------------------------------------------------------------------------
	// prepare additional arguments
	//--------------------------------------------------------------------------
	bool                    distinct   =  false ;
	const cypher_astnode_t  *skip      =  NULL  ;
	const cypher_astnode_t  *limit     =  NULL  ;
	const cypher_astnode_t  *order_by  =  NULL  ;
	const cypher_astnode_t  *predicate =  NULL  ;

	distinct      =  cypher_ast_with_is_distinct(clause);
	skip          =  cypher_ast_with_get_skip(clause);
	limit         =  cypher_ast_with_get_limit(clause);
	order_by      =  cypher_ast_with_get_order_by(clause);
	predicate     =  cypher_ast_with_get_predicate(clause);

	// clone any ORDER BY, SKIP, LIMIT, and WHERE modifiers to
	// add to the children array and populate the new clause
	uint nchildren = n_projections;
	if(order_by)  order_by   = children[nchildren++] =
		cypher_ast_clone(order_by);
	if(skip)      skip       = children[nchildren++] = cypher_ast_clone(skip);
	if(limit)     limit      = children[nchildren++] = cypher_ast_clone(limit);
	if(predicate) predicate  = children[nchildren++] =
		cypher_ast_clone(predicate);

	struct cypher_input_range range = cypher_astnode_range(clause);

	// build the replacement clause
	cypher_astnode_t *new_clause;
	new_clause = cypher_ast_with(distinct,
								false,
								projections,
								n_projections,
								order_by,
								skip,
								limit,
								predicate,
								children,
								nchildren,
								range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(
		cypher_ast_call_subquery_get_query(callsubquery), new_clause,
		clause_idx, clause_idx);
}

// adds a leading WITH clause to the query, projecting all bound vars (names) to
// their internal representation (inter_names)
static void _add_first_clause
(
	cypher_astnode_t *wrapping_clause,  // query ast-node
	cypher_astnode_t *callsubquery,     // call subquery ast-node
	uint callsubquery_ind,              // index of the call subquery node
	uint first_ind,                     // index of the clause to replace
	char **names,                       // original bound vars
	char **inter_names                  // internal representation of bound vars
) {
	uint n_names = array_len(names);
	uint n_inter_names = array_len(inter_names);

	uint n_projections = n_inter_names;
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	for(uint i = 0; i < n_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
		int ind = n_inter_names - n_names + i;
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[ind],
			strlen(inter_names[ind]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// create projections for bound vars from outer context
	// -------------------------------------------------------------------------
	uint n_outer_names = n_inter_names - n_names;
	for(uint i = 0; i < n_outer_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// prepare additional arguments
	//--------------------------------------------------------------------------
	bool                    distinct   =  false ;
	const cypher_astnode_t  *skip      =  NULL  ;
	const cypher_astnode_t  *limit     =  NULL  ;
	const cypher_astnode_t  *order_by  =  NULL  ;
	const cypher_astnode_t  *predicate =  NULL  ;

	// copy projections to the children array
	cypher_astnode_t *children[n_projections + 4];
	for(uint i = 0; i < n_projections; i++) {
		children[i] = projections[i];
	}

	struct cypher_input_range range = {0};
	uint nchildren = n_projections;

	// build the replacement clause
	cypher_astnode_t *new_clause;
	new_clause = cypher_ast_with(distinct,
								false,
								projections,
								n_projections,
								order_by,
								skip,
								limit,
								predicate,
								children,
								nchildren,
								range);

	// -------------------------------------------------------------------------
	// replace original clause with fully populated one
	// -------------------------------------------------------------------------

	cypher_astnode_t *subquery = cypher_ast_call_subquery_get_query(
		callsubquery);

	uint n_clauses = cypher_ast_query_nclauses(subquery);
	cypher_astnode_t *clauses[n_clauses + 1];
	for(uint i = 0; i < first_ind; i++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_clone(
			cypher_ast_query_get_clause(subquery, i));
	}
	clauses[first_ind] = new_clause;
	for(uint i = first_ind + 1; i < n_clauses + 1; i++) {
		clauses[i] =(cypher_astnode_t *)cypher_ast_clone(
			cypher_ast_query_get_clause(subquery, i-1));
	}

	cypher_astnode_t *q = cypher_ast_query(NULL, 0, clauses, n_clauses, clauses,
		n_clauses, range);

	cypher_astnode_t *new_callsubquery = cypher_ast_call_subquery(q, range);

	// replace the old subquery with the new one
	// TODO: Consider using replace_clause instead of set_clause (seal leaks)
	cypher_astnode_type_t type = cypher_astnode_type(wrapping_clause);
	if(type == CYPHER_AST_QUERY) {
		cypher_ast_query_replace_clauses(wrapping_clause, new_callsubquery,
			callsubquery_ind, callsubquery_ind);
	} else if (type == CYPHER_AST_CALL_SUBQUERY){
		cypher_ast_query_replace_clauses(
			cypher_ast_call_subquery_get_query(wrapping_clause),
			new_callsubquery, callsubquery_ind, callsubquery_ind);
	} else {
		// shouldn't get here
		ASSERT(false);
	}
}

// replace all intermediate WITH clauses in the query with new WITH clauses,
// containing projections from the internal representation of bound vars to
// themselves (inter_names)
static void _replace_intermediate_with_clauses
(
	cypher_astnode_t *callsubquery,     // call subquery ast-node
	uint first_ind,                     // index of first relevant clause
	uint last_ind,                      // index of last relevant clause
	char **inter_names                  // internal representation of bound vars
) {
	cypher_astnode_t *subquery = cypher_ast_call_subquery_get_query(callsubquery);
	for(uint i = first_ind + 1; i < last_ind; i++) {
		cypher_astnode_t *clause =
			(cypher_astnode_t *)cypher_ast_query_get_clause(
				subquery, i);
		if(cypher_astnode_type(clause) == CYPHER_AST_WITH) {
			_replace_with_clause(callsubquery, i, inter_names,
				inter_names);
		}
	}
}

// replaces the RETURN clause in query to a new one, containing projections of
// inter_names to names
static void _replace_return_clause
(
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint last_ind,                   // index of last relevant clause
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {
	// we know that the last clause in query is a RETURN clause, which we want
	// to replace
	cypher_astnode_t *clause =
		(cypher_astnode_t *)cypher_ast_query_get_clause(
			cypher_ast_call_subquery_get_query(callsubquery), last_ind-1);

	uint n_names = array_len(names);
	uint n_inter_names = array_len(inter_names);

	uint existing_projections_count = cypher_ast_return_nprojections(clause);
	uint n_projections = n_inter_names + existing_projections_count;
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	for(uint i = 0; i < n_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		int ind = n_inter_names - n_names + i;
		cypher_astnode_t *exp = cypher_ast_identifier(inter_names[ind],
			strlen(inter_names[ind]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// create projections for bound vars from outer context
	// -------------------------------------------------------------------------
	uint n_outer_names = n_inter_names - n_names;
	for(uint i = 0; i < n_outer_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		uint nchildren = 2;

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	// -------------------------------------------------------------------------
	// introduce explicit projections
	//--------------------------------------------------------------------------
	// clone explicit projections into projections array
	for(uint i = 0; i < existing_projections_count; i++) {
		const cypher_astnode_t *projection =
			cypher_ast_return_get_projection(clause, i);
		projections[proj_idx++] = cypher_ast_clone(projection);
	}

	// copy projections to the children array
	cypher_astnode_t *children[n_projections + 4];
	for(uint i = 0; i < n_projections; i++) {
		children[i] = projections[i];
	}

	// -------------------------------------------------------------------------
	// prepare additional arguments
	//--------------------------------------------------------------------------
	bool                    distinct   =  false;
	const cypher_astnode_t  *skip      =  NULL ;
	const cypher_astnode_t  *limit     =  NULL ;
	const cypher_astnode_t  *order_by  =  NULL ;

	distinct      =  cypher_ast_return_is_distinct(clause);
	skip          =  cypher_ast_return_get_skip(clause);
	limit         =  cypher_ast_return_get_limit(clause);
	order_by      =  cypher_ast_return_get_order_by(clause);

	// clone any ORDER BY, SKIP, LIMIT, and WHERE modifiers to
	// add to the children array and populate the new clause
	uint nchildren = n_projections;
	if(order_by)  order_by   = children[nchildren++] =
		cypher_ast_clone(order_by);
	if(skip)      skip       = children[nchildren++] = cypher_ast_clone(skip);
	if(limit)     limit      = children[nchildren++] = cypher_ast_clone(limit);

	struct cypher_input_range range = cypher_astnode_range(clause);

	// build the replacement clause
	cypher_astnode_t *new_clause;
	new_clause = cypher_ast_return(distinct,
								false,
								projections,
								n_projections,
								order_by,
								skip,
								limit,
								children,
								nchildren,
								range);

	// replace original clause with fully populated one
	cypher_ast_query_replace_clauses(
		cypher_ast_call_subquery_get_query(callsubquery), new_clause,
		last_ind - 1, last_ind - 1);
}

// rewrites the projections of a Call {} clause to contain the wanted
// projections as depicted in the main function
static void _rewrite_projections
(
	cypher_astnode_t *wrapping_clause,  // outer-context clause (query/call {})
	cypher_astnode_t *clause,           // call {} clause to rewrite
	uint clause_ind,                    // index of call {} clause in query
	uint start,                         // start ind of outer-scope bound vars
	uint end,                           // end ind of outer-scope bound vars
	char ***inter_names                 // internal representation of bound vars
) {
	// -------------------------------------------------------------------------
	// collect outer scope bound vars
	// -------------------------------------------------------------------------
	rax *outer_mapping = raxNew();
	if(start != end) {
		collect_aliases_in_scope(wrapping_clause, start, end, outer_mapping);
	}

	uint mapping_size = raxSize(outer_mapping);
	if(mapping_size == 0) {
		raxFree(outer_mapping);
		return;
	}

	// create an array for the outer scope bound vars
	char **names = array_new(char *, mapping_size);

	_get_vars_inner_rep(outer_mapping, &names, inter_names);
	raxFree(outer_mapping);

	// -------------------------------------------------------------------------
	// transform relevant clauses (initial WITH, intermediate WITHs and RETURN)
	// -------------------------------------------------------------------------

	// if there is a UNION clause in the subquery, each branch must be handled
	cypher_astnode_t *subquery = cypher_ast_call_subquery_get_query(clause);
	uint *union_indices = AST_SubqueryGetClauseIndices(clause,
		CYPHER_AST_UNION);
	uint clause_count = cypher_ast_query_nclauses(subquery);
	array_append(union_indices, clause_count);
	uint n_union_branches = array_len(union_indices);

	uint first_ind = 0;
	for(uint i = 0; i < n_union_branches; i++) {
		uint last_ind = union_indices[i];

		// check if first clause is a WITH clause
		const cypher_astnode_t *first_clause =
			cypher_ast_query_get_clause(subquery, first_ind);
		bool first_is_with =
			cypher_astnode_type(first_clause) == CYPHER_AST_WITH;
		if(first_is_with) {
			// replace first clause (WITH) with a WITH clause containing
			// "n->@n" projections for all bound vars in outer-scope context
			_replace_with_clause(clause, first_ind,
				names, *inter_names);
		} else {
			// add a leading WITH clause containing "n->@n" projections
			// for all bound vars (in outer-scope context)
			_add_first_clause(wrapping_clause, clause, clause_ind, first_ind,
				names, *inter_names);

			// update union indeces
			for(uint j = i; j < n_union_branches; j++) {
				union_indices[j]++;
			}
			last_ind++;

			// update the query and call {} nodes
			clause = (cypher_astnode_t *)
				(cypher_astnode_type(wrapping_clause) == CYPHER_AST_QUERY ?
					cypher_ast_query_get_clause(wrapping_clause, clause_ind) :
					cypher_ast_query_get_clause(
						cypher_ast_call_subquery_get_query(wrapping_clause),
						clause_ind));
			subquery = cypher_ast_call_subquery_get_query(clause);
		}

		// for every intermediate WITH clause (all but first one),
		// replace it with a clause that contains "@n->@n" projections for
		// all bound vars in outer-scope context
		_replace_intermediate_with_clauses(clause, first_ind, last_ind,
			*inter_names);

		// replace the RETURN clause (last) with a clause containing the
		// projections "@n->n" for all bound vars in outer-scope context
		_replace_return_clause(clause, last_ind, names, *inter_names);
		first_ind = union_indices[i] + 1;
	}

	array_free(union_indices);

	// free the names and inter_names, and corresponding arrays
	uint n_names = array_len(names);
	for(uint i = 0; i < n_names; i++) {
		rm_free(names[i]);
	}
	array_free(names);
}

// rewrites the subquery to contain the projections needed in case of an
// eager and returning execution-plan (see specification in main function), and
// recursively rewrites embedded Call {} clauses in the subquery
static bool _AST_RewriteCallSubqueryClause
(
	cypher_astnode_t *wrapping_clause,  // outer-context clause (query/call {})
	cypher_astnode_t *clause,           // the clause to rewrite
	uint clause_ind,                    // index of clause in query
	uint start,                         // start ind from which to collect bound vars
	uint end,                           // end ind until from which to collect bound vars
	char ***inter_names                 // internal representation of bound vars
) {
	uint wrapping_subclauses_count =
		cypher_astnode_type(wrapping_clause) == CYPHER_AST_QUERY ?
			cypher_ast_query_nclauses(wrapping_clause) :
			cypher_ast_query_nclauses(
				cypher_ast_call_subquery_get_query(wrapping_clause));

	cypher_astnode_t *subquery = cypher_ast_call_subquery_get_query(clause);
	uint subclauses_count = cypher_ast_query_nclauses(subquery);

	// check if the subquery will result in an eager and returning
	// execution-plan
	const cypher_astnode_t *last_clause =
		cypher_ast_query_get_clause(subquery, subclauses_count - 1);
	bool is_returning = cypher_astnode_type(last_clause) == CYPHER_AST_RETURN;

	bool is_eager = _node_is_eager(clause);

	if(is_eager && is_returning) {
		_rewrite_projections(wrapping_clause, clause, clause_ind, start, end,
			inter_names);
		return true;
	}

	return false;
}

// frees the added names over from the context
static void _restore_outer_internal_names
(
	char ***outer_names,  // expanded context
	int orig_size         // original context size
) {
	// free added names
	int n_added_names = array_len(*outer_names) - orig_size;
	for(int i = 0; i < n_added_names; i++) {
		char *name = array_pop(*outer_names);
		rm_free(name);
	}
}

static void _AST_RewriteCallSubqueryClauses
(
	cypher_astnode_t *wrapping_clause,  // clause (query/call {})
	char ***outer_inter_names           // internal names of bound vars in outer-scope ('@n', '@m', etc.)
) {
	cypher_astnode_type_t type = cypher_astnode_type(wrapping_clause);

	uint nclauses;
	const cypher_astnode_t *(*get_clause)(const cypher_astnode_t *, uint);

	if(type == CYPHER_AST_QUERY) {
		nclauses = cypher_ast_query_nclauses(wrapping_clause);
	} else {
		nclauses = cypher_ast_query_nclauses(
			cypher_ast_call_subquery_get_query(wrapping_clause));
	}

	// go over clauses. Upon finding a Call {} clause, rewrite it recursively
	uint start_scope = 0;
	int orig_len = array_len(*outer_inter_names);

	for(uint i = 0; i < nclauses; i++) {
		cypher_astnode_t *clause = (cypher_astnode_t *)
			(type == CYPHER_AST_QUERY ?
				cypher_ast_query_get_clause(wrapping_clause, i) :
				cypher_ast_query_get_clause(
					cypher_ast_call_subquery_get_query(wrapping_clause), i));
		if(cypher_astnode_type(clause) == CYPHER_AST_CALL_SUBQUERY) {
			// if the subquery is returning & eager, rewrite its projections
			bool clause_rewritten = _AST_RewriteCallSubqueryClause(
				wrapping_clause, clause, i, start_scope, i, outer_inter_names);

			// update clause, in case it was replaced
			if(clause_rewritten) {
				cypher_astnode_t *clause = (cypher_astnode_t *)
					(type == CYPHER_AST_QUERY ?
						cypher_ast_query_get_clause(wrapping_clause, i) :
						cypher_ast_query_get_clause(
							cypher_ast_call_subquery_get_query(wrapping_clause),
							i));
			}

			// `outer_inter_names` now contains the internal representation of
			// the current context as well

			// recursively rewrite embedded Call {} clauses, and update clause
			// in case it was rewritten
			_AST_RewriteCallSubqueryClauses(clause, outer_inter_names);

			// restore `outer_inter_names` to its original state
			_restore_outer_internal_names(outer_inter_names, orig_len);
		}

		// update start_scope if needed
		cypher_astnode_type_t type = cypher_astnode_type(clause);
		if(type == CYPHER_AST_WITH || type == CYPHER_AST_RETURN) {
			start_scope = i;
		}
	}
}

// if the subquery will result in an eager and returning execution-plan
// rewrites it to contain the projections needed:
// 1. "n"  -> "@n" in the initial WITH clause if exists. Otherwise, creates it.
// 2. "@n" -> "@n" in the intermediate WITH clauses.
// 3. "@n" -> "n" in the final RETURN clause.
// if the subquery will not result in an eager & returning execution-plan, does
// nothing
void AST_RewriteCallSubquery
(
	const cypher_astnode_t *root  // root of AST
) {
	bool rewritten = false;

	if(cypher_astnode_type(root) != CYPHER_AST_STATEMENT) {
		return;
	}

	// retrieve the root's body
	cypher_astnode_t *query =
		(cypher_astnode_t *)cypher_ast_statement_get_body(root);
	if(cypher_astnode_type(query) != CYPHER_AST_QUERY) {
		return;
	}

	char **inter_names = array_new(char *, 0);
	_AST_RewriteCallSubqueryClauses(query, &inter_names);
	uint n_inter_names = array_len(inter_names);
	for(uint i = 0; i < n_inter_names; i++) {
		rm_free(inter_names[i]);
	}
	array_free(inter_names);
}
