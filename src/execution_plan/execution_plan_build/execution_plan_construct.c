/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "execution_plan_construct.h"
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
	FT_FilterNode **sub_trees = FilterTree_SubTrees(ft);

	/* For each filter tree, find the earliest position in the op tree
	 * after which the filter tree can be applied. */
	uint nfilters = array_len(sub_trees);
	for(uint i = 0; i < nfilters; i++) {
		FT_FilterNode *tree = sub_trees[i];
		OpBase *filter_op = NewFilterOp(plan, tree);
		ExecutionPlan_RePositionFilterOp(plan, root, recurse_limit, filter_op);
	}
	array_free(sub_trees);
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
	} else {
		assert(false && "unhandeled clause");
	}
}

