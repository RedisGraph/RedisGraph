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

static void _buildCallSubqueryPlan
(
	ExecutionPlan *plan,            // execution plan to add plan to
	const cypher_astnode_t *clause  // call subquery clause
) {
	// -------------------------------------------------------------------------
	// build an AST from the subquery
	// -------------------------------------------------------------------------
	// save the original AST
	AST *orig_ast = QueryCtx_GetAST();

	// create an AST from the body of the subquery
	uint *ref_count = rm_malloc(sizeof(uint));
	*ref_count = 1;

	AST *subquery_ast = rm_malloc(sizeof(AST));
	subquery_ast->free_root = true;
	subquery_ast->parse_result = NULL;
	subquery_ast->ref_count = ref_count;
	subquery_ast->params_parse_result = NULL;
	subquery_ast->referenced_entities = NULL;
	subquery_ast->anot_ctx_collection = orig_ast->anot_ctx_collection;

	// build the query, to be the root of the temporary AST
	uint clause_count = cypher_astnode_nchildren(clause);
	cypher_astnode_t *clauses[clause_count];

	// Explicitly collect all child nodes from the clause.
	for(uint i = 0; i < clause_count; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_astnode_get_child(clause, i);
	}
	struct cypher_input_range range = {0};

	subquery_ast->root = cypher_ast_query(NULL,
							0,
							clauses,
							clause_count,
							clauses,
							clause_count,
							range);

	// -------------------------------------------------------------------------
	// build the embedded execution plan corresponding to the subquery AST
	// -------------------------------------------------------------------------
	QueryCtx_SetAST(subquery_ast);
	ExecutionPlan *embedded_plan = NewExecutionPlan();
	QueryCtx_SetAST(orig_ast);

	// // release artificial clauses array
	// array_free(clauses);

	// find the deepest op in the embedded plan
	OpBase *deepest = embedded_plan->root;
	while(deepest->childCount > 0) {
		deepest = deepest->children[0];
	}

	// if no variables are imported, add an 'empty' projection so that the
	// records within the subquery will not carry unnecessary entries
	if(cypher_astnode_type(cypher_astnode_get_child(clause, 0)) !=
		CYPHER_AST_WITH) {
		OpBase *implicit_proj =
			NewProjectOp(embedded_plan, array_new(AR_ExpNode *, 0));
		ExecutionPlan_AddOp(deepest, implicit_proj);
		deepest = implicit_proj;
	}

	// characterize whether the query is eager or not
	OPType types[] = {OPType_CREATE, OPType_UPDATE, OPType_DELETE,
					  OPType_FOREACH, OPType_MERGE, OPType_SORT};
	bool is_eager =
	  ExecutionPlan_LocateOpMatchingType(embedded_plan->root, types, 4) != NULL;
	// characterize whether the query is returning or not
	bool is_returning = OpBase_Type(embedded_plan->root) == OPType_RESULTS;

	// -------------------------------------------------------------------------
	// Get rid of the Results op if it exists.
	// Bind returning projection to the outer-scope execution-plan.
	// -------------------------------------------------------------------------
	if(is_returning) {
		// remove the Results op from the execution-plan
		ExecutionPlan_RemoveOp(embedded_plan, embedded_plan->root);

		// if the embedded plan is not eager, do not propagate input records
		if(!is_eager) {
			// bind the returning projection op to the outer plan
			OpBase *returning_proj = ExecutionPlan_LocateOp(embedded_plan->root,
				OPType_PROJECT);

			// bind the returning projection to the outer plan
				// TODO: Do this later (after affecting the projections), and modify this so that the Project record_offsets
				// change accordingly to the new plan, and it calls OpBase_Modifies() on all its projections!!
				// TODO: Add support for UNIONs (at the moment assumes ONLY ONE returning projection)
			ProjectBindToPlan(returning_proj, plan);
			goto skip_projections_modification;
		}

		// Project all existing entries according to the following transformation: 'alias' --> '@alias'.
		// If an alias is imported, it needs to stay in the record mapping as well.

		uint mapping_size = raxSize(ExecutionPlan_GetMappings(plan));
		// create an array containing coupled names ("a1", "@a1", "a2", "@a2", ...)
		// of the original names and the internal (temporary) representation of them.
		char **names = array_new(char *, mapping_size);
		AR_ExpNode **new_exps = array_new(AR_ExpNode *, mapping_size);
		raxIterator it;
		raxStart(&it, plan->record_map);
		raxSeek(&it, "^", NULL, 0);
		while(raxNext(&it)) {
			// const char *curr = (const char *)it.key;
			char *curr = calloc(1, it.key_len + 1);
			sprintf(curr, "%.*s", (int)it.key_len, it.key);
			// think about working with sds
			char *internal_rep = calloc(1, it.key_len + 2);
			sprintf(internal_rep, "@%.*s", (int)it.key_len, it.key);
			// append original name
			array_append(names, curr);
			// append internal (temporary) name
			array_append(names, internal_rep);
		}

		// create AR_EXPNodes for the intermediate projections
		AR_ExpNode ** intermediate_proj_exps = array_new(AR_ExpNode *, mapping_size);
		for(uint i = 0; i < mapping_size; i++) {
			// advance index to the internal name index
			i++;

			// get alias
			char *intermediate_alias = names[i];

			// create the AR_EXPNode from it
			struct cypher_input_range range = {0};
			AR_ExpNode *new_node =
				AR_EXP_FromASTNode(cypher_ast_identifier(intermediate_alias,
					strlen(intermediate_alias), range));
			new_node->resolved_name = intermediate_alias;
			array_append(intermediate_proj_exps, new_node);
		}

		// modify the first projection (imports) to contain the transformation `n`-->`_n`
		OpBase *import_proj = deepest;

		// add the internal names of the outer-scope aliases to the outer-scope record-mapping
		for(uint i = 0; i < mapping_size; i++) {
			// skip the original name
			i++;

			OpBase_AliasModifier(plan->root, names[i-1], names[i]);
		}

		// modify to the intermidiate project_exps
		ProjectAddProjections(import_proj, intermediate_proj_exps);

		// find 'first' Project op in the embedded execution-plan (return)
		OpBase *returning_proj = ExecutionPlan_LocateOp(embedded_plan->root,
			OPType_PROJECT);

		// create AR_EXPNodes for the return projection
		AR_ExpNode ** return_proj_exps = array_new(AR_ExpNode *, mapping_size);
		for(uint i = 0; i < mapping_size; i++) {
			// get alias
			char *orig_alias = names[i];

			// advance index to the internal name index
			i++;

			// create the AR_EXPNode from it
			struct cypher_input_range range = {0};
			AR_ExpNode *new_node = AR_EXP_FromASTNode(cypher_ast_identifier(names[i], strlen(names[i]), range));
			new_node->resolved_name = orig_alias;
			array_append(return_proj_exps, new_node);
		}

		// Add to the RETURN projection (the 'first' projection in the embedded plan)
		// the mapping of the internal representation of the outer-scope
		// variables to their original names ('_alias' --> 'alias')
			// TODO: Add support for UNIONs (at the moment assumes ONLY ONE returning projection)
		ProjectAddProjections(returning_proj, return_proj_exps);


		// modify intermediate projections in the body of the subquery, except the last Return projection (first in exec-plan),
		// to contain projections of the outer scope variables to themselves ('_alias' --> '_alias')
		OpBase **intermediate_projections = array_new(OpBase *, 1);

		// collect all the intermediate Project ops (from the child of the
		// return projection)
		if(returning_proj->childCount > 0) {
			ExecutionPlan_LocateOps(intermediate_projections,
				returning_proj->children[0], OPType_PROJECT);

			// the 'last' projection is the importing projection, pop it
			array_pop(intermediate_projections);

			// modify projected expressions of the intermediate projections if exist
			for(uint i = 0; i < array_len(intermediate_projections); i++) {
				ProjectAddProjections(intermediate_projections[i],
					intermediate_proj_exps);
						// TODO: Add support for UNIONs
			}
		}

		// bind the RETURN projection (last one) to the outer plan ('plan')
			// TODO: Add support for UNIONs (at the moment assumes ONLY ONE returning projection)
		ExecutionPlan_bindOpToPlan(returning_proj, plan);

		// TODO: Extend this to support UNION as well (unique exec-plan, which
		// this mechanism is not yet good for)
	}

skip_projections_modification:
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
		_buildCallSubqueryPlan(plan, clause);
	} else {
		assert(false && "unhandeled clause");
	}
}
