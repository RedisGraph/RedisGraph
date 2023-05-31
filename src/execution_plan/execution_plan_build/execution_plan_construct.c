/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "../ops/ops.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../execution_plan.h"
#include "execution_plan_modify.h"
#include "execution_plan_construct.h"
#include "../../util/rax_extensions.h"
#include "../../ast/ast_build_filter_tree.h"
#include "../../ast/ast_build_op_contexts.h"
#include "../../arithmetic/arithmetic_expression_construct.h"
#include "../../ast/enrichment/annotate_projected_named_paths.h"

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

// looks for a Join operation at root or root->children[0] and returns it, or
// NULL if not found
static OpBase *_getJoin
(
	OpBase *root  // root op from which to look for the Join op
) {
	// check if there is a Join operation (from UNION or UNION ALL)
	OpBase *join_op = NULL;
	if(root->type == OPType_JOIN) {
		join_op = root;
	} else if(root->childCount > 0 && root->children[0]->type == OPType_JOIN) {
		join_op = root->children[0];
	}

	return join_op;
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
	for(uint i = 0; i < clause_count; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_astnode_get_child(clause, i);
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

	OPType type = OpBase_Type(parent);
	if(type == OPType_CallSubquery || type == OPType_FOREACH) {
		ExecutionPlan_AddOpInd(parent, empty_proj, 0);
	} else {
		ExecutionPlan_AddOp(parent, empty_proj);
	}

	return empty_proj;
}

// returns true if op is effectively a deepest op (i.e., no lhs)
static bool _is_deepest_call_foreach
(
	OpBase *op  // op to check
) {
	OPType type = OpBase_Type(op);
	return (type == OPType_CallSubquery || type == OPType_FOREACH) &&
			op->childCount == 1;
}

// finds the deepest operation starting from root, and appends it to deepest_ops
// if a call {} op with one child is found, it is appended to deepest_ops
static void _get_deepest
(
	OpBase *root,          // root op from which to look for the deepest op
	OpBase ***deepest_ops  // target array to which the deepest op is appended
) {
	OpBase *deepest = root;

	// check root
	if(_is_deepest_call_foreach(deepest)) {
		array_append(*deepest_ops, deepest);
		return;
	}

	// traverse children
	while(OpBase_ChildCount(deepest) > 0) {
		deepest = deepest->children[0];
		// in case of a CallSubquery op with no lhs, we want to stop
		// here, as the added op should be its first child (instead of
		// the current child, which will be moved to be the second)
		// Example:
		// "CALL {CALL {RETURN 1 AS one} RETURN one} RETURN one"
		OPType type = OpBase_Type(deepest);
		if(_is_deepest_call_foreach(deepest)){
			array_append(*deepest_ops, deepest);
			return;
		}
	}

	array_append(*deepest_ops, deepest);
}

// returns an array of with the deepest ops of an execution plan
static OpBase **_FindDeepestOps
(
	const ExecutionPlan *plan
) {
	OpBase *deepest = plan->root;
	OpBase **deepest_ops = array_new(OpBase *, 1);

	// check root and its first child for a Join op
	OpBase *join = _getJoin(deepest);
	// if didn't find, check for a Join op in the first child of the first child
	if(join == NULL                                       &&
	   OpBase_ChildCount(deepest) > 0                     &&
	   OpBase_ChildCount(OpBase_GetChild(deepest, 0)) > 0 &&
	   (OpBase_Type(OpBase_GetChild(OpBase_GetChild(deepest, 0), 0)) ==
		OPType_JOIN)) {
			join = OpBase_GetChild(OpBase_GetChild(deepest, 0), 0);
	}

	if(join != NULL) {
		uint n_branches = OpBase_ChildCount(join);
		for(uint i = 0; i < n_branches; i++) {
			deepest = OpBase_GetChild(join, i);
			_get_deepest(deepest, &deepest_ops);
		}
	} else {
		_get_deepest(deepest, &deepest_ops);
	}

	return deepest_ops;
}

static void _bind_returning_op
(
	OpBase *op,  // op to bind
	ExecutionPlan *plan  // plan to bind op to
) {
	OPType type = OpBase_Type(op);
	if(type == OPType_PROJECT) {
		ProjectBindToPlan(op, plan);
	} else if(type == OPType_AGGREGATE) {
		AggregateBindToPlan(op, plan);
	} else {
		OpBase_UpdatePlan(op, plan);
	}
}

// binds the returning ops (effectively, all ops between the first
// Project\Aggregate and CallSubquery in every branch, inclusive) in
// embedded_plan to plan
static void _BindReturningOpsToPlan
(
	ExecutionPlan *embedded_plan,  // plan containing the returning ops
	ExecutionPlan *plan            // plan to bind the returning ops to
) {
	OPType return_types[] = {OPType_PROJECT, OPType_AGGREGATE};

	// check if there is a Join operation (from UNION or UNION ALL)
	OpBase *root = embedded_plan->root;
	OpBase *join_op = _getJoin(root);

	// if there is a Union operation, we need to look at all branches
	if(join_op == NULL) {
		// only one returning projection/aggregation
		OpBase *returning_op =
			ExecutionPlan_LocateOpMatchingType(root, return_types, 2);
		while(returning_op != NULL) {
			_bind_returning_op(returning_op, plan);
			returning_op = returning_op->parent;
		}
	} else {
		// multiple returning projections/aggregations
		for(uint i = 0; i < join_op->childCount; i++) {
			OpBase *child = join_op->children[i];
			OpBase *returning_op =
				ExecutionPlan_LocateOpMatchingType(child, return_types, 2);
			while(returning_op != NULL) {
				OPType type = OpBase_Type(returning_op);

				_bind_returning_op(returning_op, plan);
				returning_op = returning_op->parent;
			}
		}
	}
}

// add empty projections in the branches which do not contain an importing WITH
// clause, in order to 'reset' the bound-vars environment
static void _add_empty_projections
(
	AST *subquery_ast,         // subquery AST
	cypher_astnode_t *clause,  // call subquery clause
	OpBase **deepest_ops       // deepest op in each of the UNION branches
) {
	uint clause_count = cypher_ast_call_subquery_nclauses(clause);
	uint *union_indices = AST_GetClauseIndices(subquery_ast,
		CYPHER_AST_UNION);
	array_append(union_indices, clause_count);
	uint n_union_branches = array_len(union_indices);
	uint first_ind = 0;
	const cypher_astnode_t *first_clause;
	OpBase **deepest;

	for(uint i = 0; i < n_union_branches; i++) {
		// find first clause in the relevant branch
		first_clause = cypher_ast_call_subquery_get_clause(clause, first_ind);
		if(cypher_astnode_type(first_clause) != CYPHER_AST_WITH) {
			deepest = array_elem(deepest_ops, i);
			*deepest = _AddEmptyProjection(*deepest);
		}

		first_ind = union_indices[i] + 1;
	}

	array_free(union_indices);
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

	// -------------------------------------------------------------------------
	// build the embedded execution plan corresponding to the subquery
	// -------------------------------------------------------------------------
	QueryCtx_SetAST(subquery_ast);
	ExecutionPlan *embedded_plan = NewExecutionPlan();
	QueryCtx_SetAST(orig_ast);

	// find the deepest ops, to which we will add the projections and connectors
	OpBase **deepest_ops = _FindDeepestOps(embedded_plan);

	// if no variables are imported, add an 'empty' projection so that the
	// records within the subquery will not carry unnecessary entries
	_add_empty_projections(subquery_ast, clause, deepest_ops);
	AST_Free(subquery_ast);

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

		_BindReturningOpsToPlan(embedded_plan, plan);
	}

	// set Join op to not modify the ResultSet mapping
	OpBase *join_op = _getJoin(embedded_plan->root);
	if(join_op != NULL) {
		JoinSetUpdateColumnMap(join_op, false);
	}

	// -------------------------------------------------------------------------
	// create an ArgumentList\Argument op according to the eagerness of the op,
	// and plant it as the deepest child of the embedded plan
	// -------------------------------------------------------------------------
	uint n_deepest_ops = array_len(deepest_ops);
	if(is_eager) {
		for(uint i = 0; i < n_deepest_ops; i++) {
			OpBase *argument_list = NewArgumentListOp(plan);
			ExecutionPlan_AddOp(deepest_ops[i], argument_list);
		}
	} else {
		for(uint i = 0; i < n_deepest_ops; i++) {
			OpBase *argument = NewArgumentOp(plan, NULL);
			ExecutionPlan_AddOp(deepest_ops[i], argument);
		}
	}

	array_free(deepest_ops);

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
