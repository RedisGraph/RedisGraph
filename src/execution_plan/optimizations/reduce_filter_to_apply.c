/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_filter_to_apply.h"
#include "../ops/op_filter.h"
#include "../ops/apply_ops/op_semi_apply.h"
#include "../../query_ctx.h"
#include "../../ast/ast_mock.h"


static OpBase *_buildRHS(ExecutionPlan *plan, const cypher_astnode_t *path) {
	AST *ast = QueryCtx_GetAST();

	rax *bound_vars = NULL;
	const char **arguments = NULL;
	if(plan->root) {
		bound_vars = raxNew();
		// Rather than cloning the record map, collect the bound variables along with their
		// parser-generated constant strings.
		ExecutionPlan_BoundVariables(plan->root, bound_vars);
		// Prepare the variables for populating the Argument ops we will build.
		arguments = ExecutionPlan_BuildArgumentModifiesArray(bound_vars);
	}

	// Initialize an ExecutionPlan that shares this plan's Record mapping.
	ExecutionPlan *rhs_plan = ExecutionPlan_NewEmptyExecutionPlan();
	rhs_plan->record_map = plan->record_map;

	// If we have bound variables, build an Argument op that represents them.
	if(arguments) rhs_plan->root = NewArgumentOp(plan, arguments);

	// Build a temporary AST holding a MATCH clause.
	AST *rhs_ast = AST_MockMatchPattern(ast, path);

	ExecutionPlan_PopulateExecutionPlan(rhs_plan, NULL);

	AST_MockFree(rhs_ast);
	QueryCtx_SetAST(ast); // Reset the AST.

	OpBase *rhs_root = rhs_plan->root;
	// NULL-set variables shared between the rhs_plan and the overall plan.
	rhs_plan->root = NULL;
	rhs_plan->record_map = NULL;
	// We can't free the plan's individual QueryGraphs, as operations like label scans
	// may later try to access their entities.
	array_free(rhs_plan->connected_components);
	rhs_plan->connected_components = NULL;

	// Free the temporary plan.
	ExecutionPlan_Free(rhs_plan);

	return rhs_root;
}

static OpBase *_reduction(ExecutionPlan *plan, FT_FilterNode *filter_root) {
	switch(filter_root->t) {
	case FT_N_EXP: {
		AR_ExpNode *expression = filter_root->exp.exp;
		bool anti = false;
		if(strcasecmp(expression->op.func_name, "not") == 0) {
			anti = true;
			expression = expression->op.children[0];
		}
		OpBase *op_semi_apply = NewSemiApplyOp(plan, anti);
		const cypher_astnode_t *path = expression->op.children[0]->operand.constant.ptrval;
		ExecutionPlan_AddOp(op_semi_apply, _buildRHS(plan, path));
		return op_semi_apply;
	}

	default:
		return NewFilterOp(plan, filter_root);
	}

}

void _reduceFilterToApply(ExecutionPlan *plan, OpFilter *filter) {
	OpBase *apply_op = _reduction(plan, filter->filterTree);
	ExecutionPlan_ReplaceOp(plan, (OpBase *)filter, apply_op);
}

void reduceFilterIntoApply(ExecutionPlan *plan) {
	OpBase **filter_ops = ExecutionPlan_LocateOps(plan->root, OPType_FILTER);
	uint filter_ops_count = array_len(filter_ops);
	for(uint i = 0; i < filter_ops_count; i++) {
		OpFilter *filter = (OpFilter *) filter_ops[i];
		FT_FilterNode *node;
		if(FilterTree_containsFunc(filter->filterTree, "path_filter", &node)) {
			_reduceFilterToApply(plan, filter);
		}
	}
}
