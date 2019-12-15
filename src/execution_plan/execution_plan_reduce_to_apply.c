#include "execution_plan.h"
#include "ops/ops.h"
#include "../query_ctx.h"
#include "../ast/ast_mock.h"

static OpBase *_buildMatchBranch(ExecutionPlan *plan, const cypher_astnode_t *path) {
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
	ExecutionPlan *match_branch_plan = ExecutionPlan_NewEmptyExecutionPlan();
	match_branch_plan->record_map = plan->record_map;

	// If we have bound variables, build an Argument op that represents them.
	if(arguments) match_branch_plan->root = NewArgumentOp(plan, arguments);

	// Build a temporary AST holding a MATCH clause.
	AST *match_branch_ast = AST_MockMatchPattern(ast, path);

	ExecutionPlan_PopulateExecutionPlan(match_branch_plan, NULL);

	AST_MockFree(match_branch_ast);
	QueryCtx_SetAST(ast); // Reset the AST.

	OpBase *branch_match_root = match_branch_plan->root;
	ExecutionPlan_BindPlanToOps(branch_match_root, plan);
	// NULL-set variables shared between the match_branch_plan and the overall plan.
	match_branch_plan->root = NULL;
	match_branch_plan->record_map = NULL;

	// Free the temporary plan.
	ExecutionPlan_Free(match_branch_plan);

	return branch_match_root;
}

// Swap operation on operation children.
static void _OpBaseSwapChildren(OpBase *op, int a, int b) {
	assert(a >= 0 && b >= 0 && a < op->childCount && b < op->childCount);
	OpBase *tmp = op->children[a];
	op->children[a] = op->children[b];
	op->children[b] = tmp;
}

static void _CreateBoundBranch(OpBase *op, ExecutionPlan *plan) {
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
	OpBase *arg = NewArgumentOp(plan, arguments);

	/* In case of Apply operations, we need to append, the new argument branch and
	 * set it as the first child. */
	if(op->type & (OPType_APPLY_MULTIPLEXER | OPType_SEMI_APPLY)) {
		ExecutionPlan_AddOp(op, arg);
		_OpBaseSwapChildren(op, 0, op->childCount - 1);
	} else {
		// -1 & everything is true;
		op = ExecutionPlan_LocateFirstOp(op, -1);
		ExecutionPlan_AddOp(op, arg);
	}
}

/* This method reduces a filter tree into an OpBase. The method perfrom post-order traversal over the
 * filter tree, and checks if if the current subtree rooted at the visited node contains path filter or not,
 * and either reduces the root or continue traversal and reduction.
 * The three possible operations could be:
 * 1. OpFilter - In the case the subtree originated from the root does not contains any path filters.
 * 2. OpSemiApply - In case of the current root is an expression which contains a path filter.
 * 3. ApplyMultiplexer - In case the current root is an operator (OR or AND) and at least one of its children
 * has been reduced to OpSemiApply or ApplyMultiplexer. */
static OpBase *_ExecutionPlan_FilterTreeToOpBaseReduction(ExecutionPlan *plan,
														  FT_FilterNode *filter_root) {
	FT_FilterNode *node;
	// Case of an expression which contains path filter.
	if(filter_root->t == FT_N_EXP && FilterTree_containsFunc(filter_root, "path_filter", &node)) {
		AR_ExpNode *expression = filter_root->exp.exp;
		bool anti = false;
		if(strcasecmp(expression->op.func_name, "not") == 0) {
			anti = true;
			expression = expression->op.children[0];
		}
		// Build new Semi Apply op.
		OpBase *op_semi_apply = NewSemiApplyOp(plan, anti);
		const cypher_astnode_t *path = expression->op.children[0]->operand.constant.ptrval;
		// Add a match branch as a Semi Apply op child.
		ExecutionPlan_AddOp(op_semi_apply, _buildMatchBranch(plan, path));
		return op_semi_apply;
	}
	// Case of an operator (Or or And) which its subtree contains path filter
	if(filter_root->t == FT_N_COND && FilterTree_containsFunc(filter_root, "path_filter", &node)) {
		// Create the relevant LHS branch and set a bounded branch for it.
		OpBase *lhs = _ExecutionPlan_FilterTreeToOpBaseReduction(plan, filter_root->cond.left);
		_CreateBoundBranch(lhs, plan);
		// Create the relevant RHS branch and set a bounded branch for it.
		OpBase *rhs = _ExecutionPlan_FilterTreeToOpBaseReduction(plan, filter_root->cond.right);
		_CreateBoundBranch(rhs, plan);
		// Create multiplexer op and set the branches as its children.
		OpBase *apply_multiplexer = NewApplyMultiplexerOp(plan, filter_root->cond.op);
		ExecutionPlan_AddOp(apply_multiplexer, lhs);
		ExecutionPlan_AddOp(apply_multiplexer, rhs);
		return apply_multiplexer;
	}
	return NewFilterOp(plan, FilterTree_Clone(filter_root));
}

/* Reduces a filter operation to a SemiApply/ApplyMultiplexer operation, according to the filter tree */
void ExecutionPlan_ReduceFilterToApply(ExecutionPlan *plan, OpBase *op) {
	assert(op->type == OPType_FILTER);
	OpFilter *filter = (OpFilter *) op;
	// Reduce.
	OpBase *apply_op = _ExecutionPlan_FilterTreeToOpBaseReduction(plan, filter->filterTree);
	ExecutionPlan_BindPlanToOps(apply_op, plan);
	// Replace operations.
	ExecutionPlan_ReplaceOp(plan, op, apply_op);
	// Bounded branch is now the last child (after ops replacement). Make it the first.
	_OpBaseSwapChildren(apply_op, 0, apply_op->childCount - 1);
	if(op == plan->root) plan->root = apply_op;
	// Free filter op.
	OpBase_Free(op);
}
