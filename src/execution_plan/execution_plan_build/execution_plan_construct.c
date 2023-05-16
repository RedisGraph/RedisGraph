/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "execution_plan_construct.h"
#include "../../ast/enrichment/annotate_projected_named_paths.h"
#include "execution_plan_modify.h"
#include "../execution_plan.h"
#include "../../RG.h"
#include "../ops/ops.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../../util/rax_extensions.h"
#include "../../ast/ast_build_filter_tree.h"
#include "../../ast/ast_build_op_contexts.h"
#include "../../arithmetic/arithmetic_expression_construct.h"

static inline void _PushDownPathFilters(ExecutionPlan *plan,
										OpBase *path_filter_op) {
	OpBase *relocate_to = path_filter_op;
	// Find the earliest filter op in the path filter op's chain of parents.
	while(relocate_to->parent && relocate_to->parent->type == OPType_FILTER) {
		relocate_to = relocate_to->parent;
	}
	/* If the filter op is part of a chain of filter ops, migrate it
	 * to be the topmost. This ensures that cheaper filters will be
	 * applied first. */
	if(relocate_to != path_filter_op) {
		ExecutionPlan_RemoveOp(plan, path_filter_op);
		ExecutionPlan_PushBelow(relocate_to, path_filter_op);
	}
}

static void _ExecutionPlan_PlaceApplyOps(ExecutionPlan *plan) {
	OpBase **filter_ops = ExecutionPlan_CollectOps(plan->root, OPType_FILTER);
	uint filter_ops_count = array_len(filter_ops);
	for(uint i = 0; i < filter_ops_count; i++) {
		OpFilter *op = (OpFilter *)filter_ops[i];
		FT_FilterNode *node;
		if(FilterTree_ContainsFunc(op->filterTree, "path_filter", &node)) {
			// If the path filter op has other filter ops above it,
			// migrate it to be the topmost.
			_PushDownPathFilters(plan, (OpBase *)op);
			// Convert the filter op to an Apply operation
			ExecutionPlan_ReduceFilterToApply(plan, op);
		}
	}
	array_free(filter_ops);
}

void ExecutionPlan_RePositionFilterOp(ExecutionPlan *plan, OpBase *lower_bound,
									  const OpBase *upper_bound, OpBase *filter) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(filter->type == OPType_FILTER);

	/* When placing filters, we should not recurse into certain operation's
	 * subtrees that would cause logical errors.
	 * The cases we currently need to be concerned with are:
	 * Merge - the results which should only be filtered after the entity
	 * is matched or created.
	 *
	 * Apply - which has an Optional child that should project results or NULL
	 * before being filtered.
	 *
	 * The family of SemiApply ops (including the Apply Multiplexers)
	 * does not require this restriction since they are always exclusively
	 * performing filtering. */

	OpBase *op = NULL; // Operation after which filter will be located.
	FT_FilterNode *filter_tree = ((OpFilter *)filter)->filterTree;

	// collect all filtered entities.
	rax *references = FilterTree_CollectModified(filter_tree);
	uint64_t references_count = raxSize(references);

	if(references_count > 0) {
		/* Scan execution plan, locate the earliest position where all
		 * references been resolved. */
		op = ExecutionPlan_LocateReferencesExcludingOps(lower_bound, upper_bound, FILTER_RECURSE_BLACKLIST,
														BLACKLIST_OP_COUNT, references);
		if(!op) {
			// Failed to resolve all filter references.
			Error_InvalidFilterPlacement(references);
			OpBase_Free(filter);
			return;
		}
	} else {
		/* The filter tree does not contain references, like:
		 * WHERE 1=1
		 * Place the op directly below the first projection if there is one,
		 * otherwise update the ExecutionPlan root. */
		op = plan->root;
		while(op && op->childCount > 0 && op->type != OPType_PROJECT && op->type != OPType_AGGREGATE) {
			op = op->children[0];
		}
		if(op == NULL || (op->type != OPType_PROJECT && op->type != OPType_AGGREGATE)) op = plan->root;
	}

	// In case this is a pre-existing filter (this function is not called out from ExecutionPlan_PlaceFilterOps)
	if(filter->childCount > 0) {
		// If the located op is not the filter child, re position the filter.
		if(op != filter->children[0]) {
			ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
			ExecutionPlan_PushBelow(op, (OpBase *)filter);
		}
	} else if(op == NULL) {
		// No root was found, place filter at the root.
		ExecutionPlan_UpdateRoot(plan, (OpBase *)filter);
		op = filter;
	} else {
		// This is a new filter.
		ExecutionPlan_PushBelow(op, (OpBase *)filter);
	}

	/* Filter may have migrated a segment, update the filter segment
	 * and check if the segment root needs to be updated.
	 * The filter should be associated with the op's segment. */
	filter->plan = op->plan;
	// Re-set the segment root if needed.
	if(op == op->plan->root) {
		ExecutionPlan *segment = (ExecutionPlan *)op->plan;
		segment->root = filter;
	}

	raxFree(references);
}

void ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan, OpBase *root, const OpBase *recurse_limit,
								  FT_FilterNode *ft) {
	/* Decompose the filter tree into an array of the smallest possible subtrees
	 * that do not violate the rules of AND/OR combinations. */
	const FT_FilterNode **sub_trees = FilterTree_SubTrees(ft);

	/* For each filter tree, find the earliest position in the op tree
	 * after which the filter tree can be applied. */
	uint nfilters = array_len(sub_trees);
	for(uint i = 0; i < nfilters; i++) {
		FT_FilterNode *tree = FilterTree_Clone(sub_trees[i]);
		OpBase *filter_op = NewFilterOp(plan, tree);
		ExecutionPlan_RePositionFilterOp(plan, root, recurse_limit, filter_op);
	}
	array_free(sub_trees);
	FilterTree_Free(ft);

	// Build ops in the Apply family to appropriately process path filters.
	_ExecutionPlan_PlaceApplyOps(plan);
}

static inline void _buildCreateOp
(
	GraphContext *gc,
	AST *ast,
	ExecutionPlan *plan,
	const cypher_astnode_t *clause
) {
	AST_CreateContext create_ast_ctx =
		AST_PrepareCreateOp(plan->query_graph, plan->record_map, clause);

	OpBase *op =
		NewCreateOp(plan, create_ast_ctx.nodes_to_create,
				create_ast_ctx.edges_to_create);

	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUnwindOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(clause);
	OpBase *op = NewUnwindOp(plan, unwind_ast_ctx.exp);
	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUpdateOp(GraphContext *gc, ExecutionPlan *plan,
								  const cypher_astnode_t *clause) {
	rax *update_exps = AST_PrepareUpdateOp(gc, clause);
	OpBase *op = NewUpdateOp(plan, update_exps);
	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildDeleteOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	if(plan->root == NULL) {
		// delete must operate on child data, prepare an error if there
		// is no child op
		ErrorCtx_SetError("Delete was constructed without a child operation");
	}
	AR_ExpNode **exps = AST_PrepareDeleteOp(clause);
	OpBase *op = NewDeleteOp(plan, exps);
	ExecutionPlan_UpdateRoot(plan, op);
}

static void _buildForeachOp
(
	ExecutionPlan *plan,             // execution plan to add operation to
	const cypher_astnode_t *clause,  // foreach clause
	GraphContext *gc                 // graph context
) {
	// construct the following sub execution plan structure
	// foreach
	//   loop body (foreach/create/update/remove/delete/merge)
	//		unwind
	//			argument list

	//--------------------------------------------------------------------------
	// Create embedded execution plan for the body of the Foreach clause
	//--------------------------------------------------------------------------
	// construct AST from Foreach body
	uint nclauses = cypher_ast_foreach_nclauses(clause);
	cypher_astnode_t **clauses = array_new(cypher_astnode_t *, nclauses);
	for(uint i = 0; i < nclauses; i++) {
		cypher_astnode_t *inner_clause =
			(cypher_astnode_t *)cypher_ast_foreach_get_clause(clause, i);
		array_append(clauses, inner_clause);
	}

	struct cypher_input_range range = {0};
	cypher_astnode_t *new_root = cypher_ast_query(
		NULL, 0, clauses, nclauses, clauses, nclauses, range
	);

	uint *ref_count = rm_malloc(sizeof(uint));
	*ref_count = 1;

	AST *body_ast = rm_malloc(sizeof(AST));
	body_ast->root = new_root;
	body_ast->free_root = true;
	body_ast->parse_result = NULL;
	body_ast->ref_count = ref_count;
	body_ast->params_parse_result = NULL;
	body_ast->anot_ctx_collection = plan->ast_segment->anot_ctx_collection;
	body_ast->referenced_entities =
		raxClone(plan->ast_segment->referenced_entities);

	ExecutionPlan *embedded_plan = ExecutionPlan_NewEmptyExecutionPlan();
	embedded_plan->ast_segment = body_ast;
	embedded_plan->record_map = raxClone(plan->record_map);

	//--------------------------------------------------------------------------
	// build Unwind op
	//--------------------------------------------------------------------------

	// unwind foreach list expression
	AR_ExpNode *exp = AR_EXP_FromASTNode(
		cypher_ast_foreach_get_expression(clause));
	exp->resolved_name = cypher_ast_identifier_get_name(
		cypher_ast_foreach_get_identifier(clause));
	OpBase *unwind = NewUnwindOp(embedded_plan, exp);

	//--------------------------------------------------------------------------
	// build ArgumentList op
	//--------------------------------------------------------------------------

	OpBase *argument_list = NewArgumentListOp(embedded_plan);
	// TODO: After refactoring the execution-plan freeing mechanism, bind the
	// ArgumentList op to the outer-scope plan (plan), and change the condition
	// for Unwind's 'free_rec' field to be whether the child plan is different
	// from the plan the Unwind is binded to.

	// add the op as a child of the unwind operation
	ExecutionPlan_AddOp(unwind, argument_list);

	// update the root of the (currently empty) embedded plan
	ExecutionPlan_UpdateRoot(embedded_plan, unwind);

	// build the execution-plan of the body of the clause
	AST *orig_ast = QueryCtx_GetAST();
	QueryCtx_SetAST(body_ast);
	ExecutionPlan_PopulateExecutionPlan(embedded_plan);
	QueryCtx_SetAST(orig_ast);

	// free the artificial body array (not its components)
	array_free(clauses);

	// create the Foreach op, and update (outer) plan root
	OpBase *foreach = NewForeachOp(plan);
	ExecutionPlan_UpdateRoot(plan, foreach);

	// connect the embedded plan to the Foreach op
	ExecutionPlan_AddOp(foreach, embedded_plan->root);
}

// look for the returning projections/aggregations from a given operation.
// do not recurse into nested Call {} clauses
static void _collectReturningProjections
(
	OpBase *root,            // root from which to start looking
	OpBase ***returning_ops  // [OUTPUT] returning projections/aggregations
) {
	OPType return_types[] = {OPType_PROJECT, OPType_AGGREGATE};

	// check if there is a Union operation (from UNION or UNION ALL)
	OpBase *union_op = NULL;
	if(root->type == OPType_JOIN) {
		union_op = root;
	} else if(root->children[0]->type == OPType_JOIN) {
		union_op = root->children[0];
	}

	// if there is a Union operation, we need to look at all branches
	if(union_op == NULL) {
		// only one returning projection/aggregation
		OpBase *returning_op =
			ExecutionPlan_LocateOpMatchingType(root, return_types, 2);
		array_append(*returning_ops, returning_op);
		return;
	}
	// multiple returning projections/aggregations
	for(uint i = 0; i < union_op->childCount; i++) {
		OpBase *child = union_op->children[i];
		OpBase *returning_op =
			ExecutionPlan_LocateOpMatchingType(child, return_types, 2);
		array_append(*returning_ops, returning_op);
	}
}

// fill `names` and `inner_names` with the bound vars and their internal
// representation, respectively
static void _get_vars_inner_rep
(
	rax *outer_mapping,  // rax containing bound vars
	char ***names,       // [OUTPUT] bound vars
	char ***inter_names  // [OUTPUT] internal representation of vars
) {
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

// returns true if the given node will result in an eager operation
static bool _nodeIsEager
(
	cypher_astnode_t *clause  // ast-node
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

static void _replace_with_clause
(
	cypher_astnode_t *query,         // query ast-node
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	cypher_astnode_t *clause,        // clause to replace
	uint clause_idx,                 // index of clause to replace
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {
	uint existing_projections_count = cypher_ast_with_nprojections(clause);
	uint n_projections = array_len(names) + existing_projections_count;
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	uint n_orig_names = n_projections - existing_projections_count;
	for(uint i = 0; i < n_orig_names; i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
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
	if(order_by)  order_by   = children[nchildren++] = cypher_ast_clone(order_by);
	if(skip)      skip       = children[nchildren++] = cypher_ast_clone(skip);
	if(limit)     limit      = children[nchildren++] = cypher_ast_clone(limit);
	if(predicate) predicate  = children[nchildren++] = cypher_ast_clone(predicate);

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
	cypher_ast_call_subquery_replace_clauses(callsubquery, new_clause,
		clause_idx, clause_idx);
	cypher_ast_query_set_clause(query, new_clause, clause_idx);
}

static void _replace_first_clause
(
	cypher_astnode_t *query,         // query ast-node
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint first_ind,                  // index of the clause to replace
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {
	// we know the first clause is a WITH clause, which we want to replace
	cypher_astnode_t *clause =
		(cypher_astnode_t *)cypher_ast_query_get_clause(query, first_ind);
	_replace_with_clause(query, callsubquery, clause, first_ind, names, inter_names);
}

// adds a first WITH clause to the query, projecting all bound vars (names) to
// their internal representation (inter_names)
static cypher_astnode_t *_add_first_clause
(
	cypher_astnode_t *query,         // query ast-node
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint first_ind,                  // index of the clause to replace
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {
	uint n_projections = array_len(names);
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	for(uint i = 0; i < array_len(names); i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		unsigned int nchildren  = 2;

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

	uint n_clauses = cypher_ast_call_subquery_nclauses(callsubquery);
	cypher_astnode_t *clauses[n_clauses + 1];
	for(uint i = 0; i < first_ind; i++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_call_subquery_get_clause(
			callsubquery, i);
	}
	clauses[first_ind] = new_clause;
	for(uint i = first_ind + 1; i < n_clauses + 1; i++) {
		clauses[i] = (cypher_astnode_t *)cypher_ast_call_subquery_get_clause(
			callsubquery, i-1);
	}

	cypher_astnode_t *new_callsubquery = cypher_ast_call_subquery(clauses,
		n_clauses + 1, clauses, n_clauses + 1, range);

	// find the index of the Call {} clause in the query
	uint callsubquery_ind = -1;
	AST *outer_ast = QueryCtx_GetAST();
	AST_GetClause(outer_ast, CYPHER_AST_CALL_SUBQUERY, &callsubquery_ind);
	ASSERT(callsubquery_ind != -1);

	// get the query node of the outer context
	cypher_astnode_t *outer_query = (cypher_astnode_t *)outer_ast->root;
	cypher_ast_query_set_clause(outer_query, new_callsubquery, callsubquery_ind);

	cypher_astnode_t *new_query = cypher_ast_query(NULL, 0, clauses,
		n_clauses + 1, clauses, n_clauses + 1, range);
	// cypher_ast_query_set_clause(query, new_clause, 0);
	return new_query;
}

// replace all intermediate WITH clauses in the query with new WITH clauses,
// containing projections from the internal representation of bound vars to
// themselves (inter_names)
static void _replace_intermediate_with_clauses
(
	cypher_astnode_t *query,         // query ast-node
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint first_ind,                  // index of first relevant clause
	uint last_ind,                   // index of last relevant clause
	char **inter_names               // internal representation of bound vars
) {
	// -------------------------------------------------------------------------
	// traverse the query clauses, collecting all WITH clauses, except the first
	// one
	// -------------------------------------------------------------------------
	uint n_clauses = cypher_ast_query_nclauses(query);
	for(uint i = first_ind + 1; i < last_ind; i++) {
		cypher_astnode_t *clause =
			(cypher_astnode_t *)cypher_ast_query_get_clause(query, i);
		if(cypher_astnode_type(clause) == CYPHER_AST_WITH) {
			_replace_with_clause(query, callsubquery, clause, i, inter_names,
				inter_names);
		}
	}
}

// replaces the RETURN clause in query to a new one, containing projections of
// inter_names to names
static void _replace_return_clause
(
	cypher_astnode_t *query,         // query ast-node
	cypher_astnode_t *callsubquery,  // call subquery ast-node
	uint last_ind,                   // index of last relevant clause
	char **names,                    // original bound vars
	char **inter_names               // internal representation of bound vars
) {
	// we know that the last clause in query is a RETURN clause, which we want
	// to replace
	cypher_astnode_t *clause =
		(cypher_astnode_t *)cypher_ast_query_get_clause(query, last_ind-1);

	uint existing_projections_count = cypher_ast_return_nprojections(clause);
	uint n_projections = array_len(names) + existing_projections_count;
	// TODO: Do we need proj_idx? Seems like we don't.
	uint proj_idx = 0;
	cypher_astnode_t *projections[n_projections + 1];

	// -------------------------------------------------------------------------
	// create projections for bound vars
	// -------------------------------------------------------------------------
	for(uint i = 0; i < array_len(names); i++) {
		// create a projection for the bound var
		struct cypher_input_range range = {0};
		cypher_astnode_t *exp = cypher_ast_identifier(inter_names[i],
			strlen(inter_names[i]), range);
		cypher_astnode_t *alias = cypher_ast_identifier(names[i],
			strlen(names[i]), range);
		cypher_astnode_t *children[2];
		children[0] = exp;
		children[1] = alias;
		unsigned int nchildren  = 2;

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
	if(order_by)  order_by   = children[nchildren++] = cypher_ast_clone(order_by);
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
	cypher_ast_call_subquery_replace_clauses(callsubquery, new_clause,
		last_ind - 1, last_ind - 1);
	cypher_ast_query_set_clause(query, new_clause, last_ind - 1);
	// cypher_ast_query_replace_clauses(query, new_clause, n_clauses-1, n_clauses-1);
}

// returns an AST containing the body of a subquery as its body (stripped from
// the CALL {} clause)
static AST *_CreateASTFromCallSubquery
(
	cypher_astnode_t *clause,  // CALL {} ast-node
	const AST *orig_ast,       // original AST with which to build new one
	rax *outer_mapping         // mapping of outer-scope bound vars
) {
	// create an AST from the body of the subquery
	uint *ref_count = rm_malloc(sizeof(uint));
	*ref_count = 1;

	AST *subquery_ast = rm_malloc(sizeof(AST));
	subquery_ast->free_root           = true;
	subquery_ast->ref_count           = ref_count;
	subquery_ast->parse_result        = NULL;
	subquery_ast->params_parse_result = NULL;
	subquery_ast->referenced_entities = NULL;
	subquery_ast->anot_ctx_collection = orig_ast->anot_ctx_collection;

	// build the query, to be the root of the temporary AST
	uint clause_count = cypher_astnode_nchildren(clause);
	cypher_astnode_t *clauses[clause_count];

	// explicitly collect all child nodes from the clause.
	bool is_eager = false;
	for(uint i = 0; i < clause_count; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_astnode_get_child(clause, i);
		is_eager |= _nodeIsEager(clauses[i]);
	}
	struct cypher_input_range range = {0};

	cypher_astnode_t *query = cypher_ast_query(NULL,
								0,
								clauses,
								clause_count,
								clauses,
								clause_count,
								range);
	subquery_ast->root = query;

	// check if the subquery is returning
	const cypher_astnode_t *last_clause =
		cypher_ast_call_subquery_get_clause(clause, clause_count-1);
	bool is_returning = cypher_astnode_type(last_clause) == CYPHER_AST_RETURN;

	if(is_eager && is_returning) {

		uint mapping_size = raxSize(outer_mapping);
		if(mapping_size == 0) {
			return subquery_ast;
		}

		// create an array containing coupled names
		// ("a1", "@a1", "a2", "@a2", ...) of the original names and the
		// internal (temporary) representation of them.
		char **names = array_new(char *, mapping_size);
		char **inter_names = array_new(char *, mapping_size);

		_get_vars_inner_rep(outer_mapping, &names, &inter_names);

		uint *union_indeces = AST_GetClauseIndices(subquery_ast, CYPHER_AST_UNION);
		array_append(union_indeces, clause_count);
		uint n_union_branches = array_len(union_indeces);

		uint first_ind = 0;
		for(uint i = 0; i < n_union_branches; i++) {
			uint last_ind = union_indeces[i];

			// check if first clause is a WITH clause
			bool first_is_with =
				cypher_astnode_type(clauses[first_ind]) == CYPHER_AST_WITH;
			if(first_is_with) {
				// replace first clause (WITH) with a WITH clause containing
				// "n->@n" projections for all bound vars in outer-scope context
				_replace_first_clause(query, clause, first_ind, names, inter_names);
			} else {
				// add a WITH clause containing "n->@n" projections
				// for all bound vars (in outer-scope context), to be the
				// first clause in the subquery
				query = _add_first_clause(query, clause, first_ind, names,
					inter_names);

				// update indeces
				for(uint j = 0; j < n_union_branches; j++) {
					union_indeces[j]++;
				}
				last_ind++;

				// update the query and call {} nodes
				subquery_ast->root = query;
				clause = (cypher_astnode_t *)AST_GetClause(orig_ast,
					CYPHER_AST_CALL_SUBQUERY, NULL);
			}

			// for every intermediate WITH clause (except the first one),
			// replace it with a clause that contains "@n->@n" projections for
			// all bound vars in outer-scope context
			_replace_intermediate_with_clauses(query, clause, first_ind,
				last_ind, inter_names);

			// replace the RETURN clause (last) with a clause containing the
			// projections "@n->n" for all bound vars in outer-scope context
			_replace_return_clause(query, clause, last_ind, names, inter_names);
			first_ind = union_indeces[i] + 1;
		}

		array_free(union_indeces);

		// free the names and inter_names, and corresponding arrays
		for(uint i = 0; i < mapping_size; i++) {
			rm_free(names[i]);
			rm_free(inter_names[i]);
		}
		array_free(names);
		array_free(inter_names);
	}

	return subquery_ast;
}

// adds an empty projection as the child of parent, such that the records passed
// to parent are "filtered" to contain no bound vars
static OpBase *_AddEmptyProjection
(
	OpBase *parent
) {
	OpBase *empty_proj =
		NewProjectOp(parent->plan, array_new(AR_ExpNode *, 0));

	if(OpBase_Type(parent) == OPType_CallSubquery) {
		ExecutionPlan_AddOpInd(parent, empty_proj, 0);
	} else {
		ExecutionPlan_AddOp(parent, empty_proj);
	}

	return empty_proj;
}

// returns the deepest operation in the given execution plan
// if there is a Join op, the deepest op of every branch is returned
static OpBase *_FindDeepestOp
(
	const ExecutionPlan *plan
) {
	OpBase *deepest = plan->root;
	while(deepest->childCount > 0) {
		deepest = deepest->children[0];
		// in case of a CallSubquery op with no lhs, we want to stop here, as
		// the added ops should be its first child (instead of the current one,
		// which will be moved to be the second child)
		if(OpBase_Type(deepest) == OPType_CallSubquery &&
			deepest->childCount == 1) {
				break;
		}
	}
	return deepest;
}

// binds Project/Aggregate ops to an execution plan
static void _BindReturningOpsToPlan
(
	OpBase **ops,        // ops to bind to plan
	ExecutionPlan *plan  // plan to bind ops to
) {
	const uint n_returning_ops = array_len(ops);
	for(uint i = 0; i < n_returning_ops; i++) {
		OPType returning_op_type = OpBase_Type(ops[0]);
		returning_op_type == OPType_PROJECT ?
			ProjectBindToPlan(ops[i], plan) :
			AggregateBindToPlan(ops[i], plan);
	}
}

static void _buildCallSubqueryPlan
(
	ExecutionPlan *plan,      // execution plan to add plan to
	cypher_astnode_t *clause  // call subquery clause
) {
	// -------------------------------------------------------------------------
	// build an AST from the subquery
	// -------------------------------------------------------------------------
	// save the original AST
	AST *orig_ast = QueryCtx_GetAST();

	// create an AST from the body of the subquery
	AST *subquery_ast = _CreateASTFromCallSubquery(clause, orig_ast,
		plan->record_map);

	// update the original AST
	clause = (cypher_astnode_t *)AST_GetClause(orig_ast,
					CYPHER_AST_CALL_SUBQUERY, NULL);

	// FOR DEBUGGING (to be removed): print the AST of the subquery
	// cypher_ast_fprint(subquery_ast->root, stdout, 0, NULL, 0);
	// printf("\n\n\n");

	// -------------------------------------------------------------------------
	// build the embedded execution plan corresponding to the subquery
	// -------------------------------------------------------------------------
	QueryCtx_SetAST(subquery_ast);
	ExecutionPlan *embedded_plan = NewExecutionPlan();
	AST_Free(subquery_ast);
	QueryCtx_SetAST(orig_ast);

	// find the deepest op in the embedded plan
	// TODO: for every branch of the Join op, if it exists
	OpBase *deepest = _FindDeepestOp(embedded_plan);

	// if no variables are imported, add an 'empty' projection so that the
	// records within the subquery will not carry unnecessary entries
	// TODO: for every branch of the Join op, if it exists
	const cypher_astnode_t *first_clause =
		cypher_ast_call_subquery_get_clause(clause, 0);
	if(cypher_astnode_type(first_clause) != CYPHER_AST_WITH) {
		deepest = _AddEmptyProjection(deepest);
	}

	// characterize whether the query is eager or not
	OPType eager_types[] = {OPType_CREATE, OPType_UPDATE, OPType_FOREACH,
					  OPType_MERGE, OPType_SORT, OPType_AGGREGATE};

	bool is_eager =
	  ExecutionPlan_LocateOpMatchingType(embedded_plan->root, eager_types, 6)
		!= NULL;

	bool is_returning = OpBase_Type(embedded_plan->root) == OPType_RESULTS;

	// -------------------------------------------------------------------------
	// Get rid of the Results op if it exists.
	// Bind returning projection\aggregation to the outer-scope execution-plan
	// -------------------------------------------------------------------------
	if(is_returning) {
		// remove the Results op from the execution-plan
		OpBase *results_op = embedded_plan->root;
		ExecutionPlan_RemoveOp(embedded_plan, embedded_plan->root);
		OpBase_Free(results_op);

		OpBase **returning_ops = array_new(OpBase *, 1);
		_collectReturningProjections(embedded_plan->root, &returning_ops);
		uint n_returning_ops = array_len(returning_ops);

		// bind the RETURN projections/aggregations to the outer plan ('plan')
		_BindReturningOpsToPlan(returning_ops, plan);
		array_free(returning_ops);
	}

	// -------------------------------------------------------------------------
	// create an ArgumentList\Argument op according to the eagerness of the op,
	// and plant it as the deepest child of the embedded plan
	// -------------------------------------------------------------------------
	if(is_eager) {
		// add an ArgumentList op
		OpBase *argument_list = NewArgumentListOp(plan);
		ExecutionPlan_AddOp(deepest, argument_list);
	} else {
		// add an Argument op
		OpBase *argument = NewArgumentOp(plan, NULL);
		ExecutionPlan_AddOp(deepest, argument);
	}

	// -------------------------------------------------------------------------
	// create a CallSubquery op, set it to be the root of the main plan
	// -------------------------------------------------------------------------
	OpBase *call_op = NewCallSubqueryOp(plan, is_eager, is_returning);
	ExecutionPlan_UpdateRoot(plan, call_op);

	// bind the embedded plan to be the rhs branch of the Call-Subquery op
	ExecutionPlan_AddOp(call_op, embedded_plan->root);
}

void ExecutionPlanSegment_ConvertClause
(
	GraphContext *gc,
	AST *ast,
	ExecutionPlan *plan,
	const cypher_astnode_t *clause
) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	// Because 't' is set using the offsetof() call
	// it cannot be used in switch statements
	if(t == CYPHER_AST_MATCH) {
		buildMatchOpTree(plan, ast, clause);
	} else if(t == CYPHER_AST_CALL) {
		buildCallOp(ast, plan, clause);
	} else if(t == CYPHER_AST_CREATE) {
		_buildCreateOp(gc, ast, plan, clause);
	} else if(t == CYPHER_AST_UNWIND) {
		_buildUnwindOp(plan, clause);
	} else if(t == CYPHER_AST_MERGE) {
		buildMergeOp(plan, ast, clause, gc);
	} else if(t == CYPHER_AST_SET || t == CYPHER_AST_REMOVE) {
		_buildUpdateOp(gc, plan, clause);
	} else if(t == CYPHER_AST_DELETE) {
		_buildDeleteOp(plan, clause);
	} else if(t == CYPHER_AST_RETURN) {
		// Converting a RETURN clause can create multiple operations.
		buildReturnOps(plan, clause);
	} else if(t == CYPHER_AST_WITH) {
		// Converting a WITH clause can create multiple operations.
		buildWithOps(plan, clause);
	} else if(t == CYPHER_AST_FOREACH) {
		_buildForeachOp(plan, clause, gc);
	} else if(t == CYPHER_AST_CALL_SUBQUERY) {
		_buildCallSubqueryPlan(plan, (cypher_astnode_t *)clause);
	} else {
		assert(false && "unhandeled clause");
	}
}
