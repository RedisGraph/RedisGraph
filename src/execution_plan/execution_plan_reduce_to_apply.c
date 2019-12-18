#include "execution_plan.h"
#include "ops/ops.h"
#include "../query_ctx.h"
#include "../ast/ast_mock.h"
#include "./optimizations/optimizer.h"

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

	// We have bound variables, build an Argument op that represents them.
	OpBase *argument = NewArgumentOp(plan, arguments);
	match_branch_plan->root = argument;

	// Build a temporary AST holding a MATCH clause.
	AST *match_branch_ast = AST_MockMatchPattern(ast, path);

	ExecutionPlan_PopulateExecutionPlan(match_branch_plan, NULL);

	AST_MockFree(match_branch_ast);
	QueryCtx_SetAST(ast); // Reset the AST.

	OpBase *branch_match_root = match_branch_plan->root;
	ExecutionPlan_BindPlanToOps(branch_match_root, plan);
	/* Don't lose information when optimizing this branch. The optimizations are lookgin for
	* specific operation types and if the result of the operations is achievable by one of its
	* descendants, this op is redundant. The argument op "modifies" the execution plan by
	* showing the bounded arguments. Given that, once optimized the label scan op is
	* so called redundant and will be removed, since its result is achievable by the argument op.
	* But this is wrong since the argument op is not modifing anything.
	* The next sequence should avoid the removal of the label scan op, and yet optimize the rest
	* of the branch. */
	if(argument->parent->type == OPType_NODE_BY_LABEL_SCAN) {
		OpBase *parent = argument->parent;
		// Temporary remove label scan op. Now argument op is the top op.
		ExecutionPlan_RemoveOp(plan, parent);
		// Optimize the rest of the plan. Unfortunately, this currently needs to be done in place.
		optimizePlan(match_branch_plan);
		// Add back the scan op in its proper place.
		ExecutionPlan_ReplaceOp(match_branch_plan, argument, parent);
		ExecutionPlan_AddOp(parent, argument);
		// Remove modifies from argument op since no modifications are required in this branch.
		array_free(argument->modifies);
		argument->modifies = NULL;
	}

	// NULL-set variables shared between the match_branch_plan and the overall plan.
	match_branch_plan->root = NULL;
	match_branch_plan->record_map = NULL;
	// Free the temporary plan.
	ExecutionPlan_Free(match_branch_plan);

	return branch_match_root;
}

/* Swap operation on operation children. If two valid indices, a and b, are given, this operation
 * swap the child in index a with the child in index b. */
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
	ExecutionPlan_AddOp(op, arg);

	if(op->type & (OPType_APPLY_MULTIPLEXER | OPType_SEMI_APPLY))
		_OpBaseSwapChildren(op, 0, op->childCount - 1);
}

/* This method reduces a filter tree into an OpBase. The method perfrom post-order traversal over the
 * filter tree, and checks if if the current subtree rooted at the visited node contains path filter or not,
 * and either reduces the root or continue traversal and reduction.
 * The three possible operations could be:
 * 1. OpFilter - In the case the subtree originated from the root does not contains any path filters.
 * 2. OpSemiApply - In case of the current root is an expression which contains a path filter.
 * 3. ApplyMultiplexer - In case the current root is an operator (OR or AND) and at least one of its children
 * has been reduced to OpSemiApply or ApplyMultiplexer. */
static OpBase *_ReduceFilterToOpBase(ExecutionPlan *plan,
									 FT_FilterNode *filter_root) {
	FT_FilterNode *node;
	// Case of an expression which contains path filter.
	if(filter_root->t == FT_N_EXP && FilterTree_ContainsFunc(filter_root, "path_filter", &node)) {
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
	if(filter_root->t == FT_N_COND && FilterTree_ContainsFunc(filter_root, "path_filter", &node)) {
		// Create the relevant LHS branch and set a bounded branch for it.
		OpBase *lhs = _ReduceFilterToOpBase(plan, filter_root->cond.left);
		if(lhs->type == OPType_FILTER) filter_root->cond.left = NULL;
		_CreateBoundBranch(lhs, plan);
		// Create the relevant RHS branch and set a bounded branch for it.
		OpBase *rhs = _ReduceFilterToOpBase(plan, filter_root->cond.right);
		if(rhs->type == OPType_FILTER) filter_root->cond.right = NULL;
		_CreateBoundBranch(rhs, plan);
		// Create multiplexer op and set the branches as its children.
		OpBase *apply_multiplexer = NewApplyMultiplexerOp(plan, filter_root->cond.op);
		ExecutionPlan_AddOp(apply_multiplexer, lhs);
		ExecutionPlan_AddOp(apply_multiplexer, rhs);
		return apply_multiplexer;
	}
	// In the case of a simple filter.
	return NewFilterOp(plan, filter_root);
}

/* Reduces a filter operation to a SemiApply/ApplyMultiplexer operation, according to the filter tree */
void ExecutionPlan_ReduceFilterToApply(ExecutionPlan *plan, OpBase *op) {
	assert(op->type == OPType_FILTER);
	OpFilter *filter = (OpFilter *) op;
	// Reduce.
	OpBase *apply_op = _ReduceFilterToOpBase(plan, filter->filterTree);
	ExecutionPlan_BindPlanToOps(apply_op, plan);
	// Replace operations.
	ExecutionPlan_ReplaceOp(plan, op, apply_op);
	// Bounded branch is now the last child (after ops replacement). Make it the first.
	_OpBaseSwapChildren(apply_op, 0, apply_op->childCount - 1);
	if(op == plan->root) plan->root = apply_op;
	// Free filter op.
	OpBase_Free(op);
}
