/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast_rewrite_call_subquery.h"

// returns true if the given node will result in an eager operation
static bool _nodeIsEager
(
	const cypher_astnode_t *clause  // ast-node
) {
	bool is_eager = false;

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

	// -------------------------------------------------------------------------
	// check if clause is a WITH or RETURN clause with an aggregation
	// -------------------------------------------------------------------------
	if(type == CYPHER_AST_RETURN || type == CYPHER_AST_WITH) {
		is_eager = AST_ClauseContainsAggregation(clause);
	}

	return is_eager;
}

// rewrites the projections of a Call {} clause to contain the wanted
// projections as depicted in the main function
static void _rewrite_projections
(
    cypher_astnode_t *clause,
    cypher_astnode_t *wrapping_clause
) {
    
}

// rewrites the subquery to contain the projections needed in case of an
// eager and returning execution-plan (see specification in main function), and
// recursively rewrites embedded Call {} clauses in the subquery
static void _AST_RewriteCallSubqueryClause
(
    cypher_astnode_t *clause,           // the clause to rewrite
    cypher_astnode_t *wrapping_clause  // outer-context clause (query/call {})
) {

    uint subclauses_count = cypher_ast_call_subquery_nclauses(clause);

    // check if the subquery will result in an eager and returning execution-plan
	const cypher_astnode_t *last_clause =
		cypher_ast_call_subquery_get_clause(clause, subclauses_count - 1);
	bool is_returning = cypher_astnode_type(last_clause) == CYPHER_AST_RETURN;

    bool is_eager = false;
	for(uint i = 0; i < subclauses_count; i ++) {
		is_eager |= _nodeIsEager(cypher_astnode_get_child(clause, i));
	}

    if(is_eager && is_returning) {
        _rewrite_projections(clause, wrapping_clause);
    }

    uint wrapping_subclauses_count =
        cypher_astnode_type(wrapping_clause) == CYPHER_AST_QUERY ?
            cypher_ast_query_nclauses(wrapping_clause) :
            cypher_ast_call_subquery_nclauses(wrapping_clause);

    // rewrite embedded Call {} clauses
    for(uint i = 0; i < wrapping_subclauses_count; i++) {
        cypher_astnode_t *sub_clause =
            (cypher_astnode_t *)cypher_ast_call_subquery_get_clause(clause, i);
        if(cypher_astnode_type(sub_clause) == CYPHER_AST_CALL) {
            _AST_RewriteCallSubqueryClause(sub_clause, clause);
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
bool AST_RewriteCallSubquery
(
	const cypher_astnode_t *root // root of AST
) {
    bool rewritten = false;

	if(cypher_astnode_type(root) != CYPHER_AST_STATEMENT) {
		return rewritten;
	}

    // retrieve the root's body
	cypher_astnode_t *query =
        (cypher_astnode_t *)cypher_ast_statement_get_body(root);
	if(cypher_astnode_type(query) != CYPHER_AST_QUERY) {
		return rewritten;
	}

    // go over clauses. Upon finding a Call {} clause, rewrite it.
    uint clause_count = cypher_ast_query_nclauses(query);
    for(uint i = 0; i < clause_count; i++) {
        cypher_astnode_t *clause =
            (cypher_astnode_t *)cypher_ast_query_get_clause(query, i);
        if(cypher_astnode_type(clause) == CYPHER_AST_CALL) {
            rewritten = true;
            _AST_RewriteCallSubqueryClause(clause, query);
        }
    }

    return rewritten;    
}
