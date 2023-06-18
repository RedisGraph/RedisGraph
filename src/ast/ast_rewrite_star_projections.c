/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast.h"
#include "../errors.h"
#include "ast_shared.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/sds/sds.h"
#include "../procedures/procedure.h"

static void replace_clause
(
	cypher_astnode_t *root,    // ast root
	cypher_astnode_t *clause,  // clause being replaced
	int scope_start,           // beginning of scope
	int scope_end              // ending of scope
) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);

	//--------------------------------------------------------------------------
	// collect identifiers
	//--------------------------------------------------------------------------
	rax *identifiers = raxNew();
	collect_aliases_in_scope(root, scope_start, scope_end, identifiers);

	//--------------------------------------------------------------------------
	// determine number of projections
	//--------------------------------------------------------------------------

	// `existing_projections_count` refers to explicit projections
	// e.g.
	// RETURN *, x, 1+2
	// `x`, `1+2` are explicit projections
	uint existing_projections_count = (t == CYPHER_AST_WITH) ?
		cypher_ast_with_nprojections(clause) :
		cypher_ast_return_nprojections(clause);

	//--------------------------------------------------------------------------
	// remove explicit identifiers
	//--------------------------------------------------------------------------

	for(uint i = 0; i < existing_projections_count; i ++) {
		const cypher_astnode_t *projection = (t == CYPHER_AST_WITH) ?
			cypher_ast_with_get_projection(clause, i) :
			cypher_ast_return_get_projection(clause, i);
		// if the projection has an alias use it,
		// otherwise the expression is the alias
		const cypher_astnode_t *exp =
			cypher_ast_projection_get_alias(projection);
		exp = exp ? exp : cypher_ast_projection_get_expression(projection);
		ASSERT(cypher_astnode_type(exp) == CYPHER_AST_IDENTIFIER);

		const char *identifier = cypher_ast_identifier_get_name(exp);
		raxRemove(identifiers, (unsigned char *)identifier, strlen(identifier), NULL);
	}

	// compute identifiers_count after duplication removal
	uint identifiers_count = raxSize(identifiers);

	// require atleast 1 projection
	uint nprojections = identifiers_count + existing_projections_count;
	uint proj_idx = 0; // projections will be added to projections[proj_idx];
	cypher_astnode_t *projections[nprojections + 1];

	//--------------------------------------------------------------------------
	// convert identifiers to expressions
	//--------------------------------------------------------------------------

	raxIterator it;
	raxStart(&it, identifiers);
	raxSeek(&it, "^", NULL, 0);

	while(raxNext(&it)) {
		cypher_astnode_t*         exp        = cypher_ast_clone(it.data);
		const cypher_astnode_t*   alias      = NULL;
		cypher_astnode_t**        children   = &exp;
		unsigned int              nchildren  = 1;
		struct cypher_input_range range      = cypher_astnode_range(it.data);

		projections[proj_idx++] = cypher_ast_projection(exp, alias, children,
				nchildren, range);
	}

	raxStop(&it);

	//--------------------------------------------------------------------------
	// handle no projections
	//--------------------------------------------------------------------------

	// e.g.
	// MATCH () RETURN *
	// MATCH () WITH * RETURN *
	if(nprojections == 0) {
		if(t == CYPHER_AST_RETURN) {
			// error if this is a RETURN clause with no aliases
			// e.g.
			// MATCH () RETURN *
			ErrorCtx_SetError("RETURN * is not allowed when there are no variables in scope");
			raxFree(identifiers);
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
			projections[proj_idx++] = cypher_ast_projection(expression,
					identifier, children, 2, range);
		}
	}

	//--------------------------------------------------------------------------
	// introduce explicit projections
	//--------------------------------------------------------------------------

	// clone explicit projections into projections array
	for(uint i = 0; i < existing_projections_count; i ++) {
		const cypher_astnode_t *projection = (t == CYPHER_AST_WITH) ?
			cypher_ast_with_get_projection(clause, i) :
			cypher_ast_return_get_projection(clause, i);
		// if the projection has an alias use it,
		// otherwise the expression is the alias
		const cypher_astnode_t *exp =
			cypher_ast_projection_get_alias(projection);
		exp = exp ? exp : cypher_ast_projection_get_expression(projection);
		ASSERT(cypher_astnode_type(exp) == CYPHER_AST_IDENTIFIER);

		// maintain expression
		projections[proj_idx++] = cypher_ast_clone(projection);
	}

	// update `nprojections` to actual number of projections
	// value might be reduced due to duplicates
	nprojections = proj_idx;
	raxFree(identifiers);

	// prepare arguments for new return clause node
	bool                    distinct   =  false ;
	const cypher_astnode_t  *skip      =  NULL  ;
	const cypher_astnode_t  *limit     =  NULL  ;
	const cypher_astnode_t  *order_by  =  NULL  ;
	const cypher_astnode_t  *predicate =  NULL  ;

	if(t == CYPHER_AST_WITH) {
		distinct      =  cypher_ast_with_is_distinct(clause);
		skip          =  cypher_ast_with_get_skip(clause);
		limit         =  cypher_ast_with_get_limit(clause);
		order_by      =  cypher_ast_with_get_order_by(clause);
		predicate     =  cypher_ast_with_get_predicate(clause);
	} else {
		distinct      =  cypher_ast_return_is_distinct(clause);
		skip          =  cypher_ast_return_get_skip(clause);
		limit         =  cypher_ast_return_get_limit(clause);
		order_by      =  cypher_ast_return_get_order_by(clause);
	}

	// copy projections to the children array
	cypher_astnode_t *children[nprojections + 4];
	for(uint i = 0; i < nprojections; i++) {
		children[i] = projections[i];
	}

	// clone any ORDER BY, SKIP, LIMIT, and WHERE modifiers to
	// add to the children array and populate the new clause
	uint nchildren = nprojections;
	if(order_by)  order_by   = children[nchildren++] = cypher_ast_clone(order_by);
	if(skip)      skip       = children[nchildren++] = cypher_ast_clone(skip);
	if(limit)     limit      = children[nchildren++] = cypher_ast_clone(limit);
	if(predicate) predicate  = children[nchildren++] = cypher_ast_clone(predicate);

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
									 predicate,
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
	cypher_ast_query_set_clause(root, new_clause, scope_end);
}

bool AST_RewriteStarProjections
(
	const cypher_astnode_t *root  // root for which to rewrite star projections
) {
	bool rewritten = false;

	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) {
		return false;
	}

	// rewrite all WITH * / RETURN * clauses to include all aliases
	uint scope_start  = 0;
	uint clause_count = cypher_ast_query_nclauses(root);

	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(root, i);
		cypher_astnode_type_t t = cypher_astnode_type(clause);

		if(t == CYPHER_AST_CALL_SUBQUERY) {
			// TODO: Manually check if the first clause of the subquery is a
			// `WITH *` clause, and if it is, rewrite it with the currently bound vars
			cypher_astnode_t *subquery =
				cypher_ast_call_subquery_get_query(clause);
			rewritten |= AST_RewriteStarProjections(subquery);
			continue;
		}

		if(t != CYPHER_AST_WITH && t != CYPHER_AST_RETURN) {
			continue;
		}

		bool has_include_existing = (t == CYPHER_AST_WITH) ?
									cypher_ast_with_has_include_existing(clause) :
									cypher_ast_return_has_include_existing(clause);

		if(has_include_existing) {
			// clause contains a star projection, replace it
			replace_clause((cypher_astnode_t *)root, (cypher_astnode_t *)clause,
						   scope_start, i);
			rewritten = true;
		}

		// update scope start
		scope_start = i;
	}

	return rewritten;
}

