/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/


#include "RG.h"
#include "./ops/ops.h"
#include "../query_ctx.h"
#include "../util/rax_extensions.h"
#include "execution_plan_build/execution_plan_modify.h"

OpBase* buildRollUpMatchStream(ExecutionPlan *plan, AR_ExpNode *exp) {
	ASSERT(plan != NULL);
	ASSERT(exp != NULL);
	
	// extract pattern path
	ASSERT(AR_EXP_IsConstant(exp->op.children[0]));
	SIValue val = AR_EXP_Evaluate(exp->op.children[0], NULL);
	ASSERT(SI_TYPE(val) & T_PTR);
	const cypher_astnode_t *path = (const cypher_astnode_t *)val.ptrval;

	// collect the variables that are bound at this point
	rax *bound_vars = raxNew();
	for(uint i = 1; i < exp->op.child_count; i ++) {
		AR_ExpNode *child = exp->op.children[i];
		ASSERT(child->type == AR_EXP_OPERAND);
		ASSERT(child->operand.type == AR_EXP_VARIADIC);
		const char *entity_alias = child->operand.variadic.entity_alias;
		raxTryInsert(bound_vars, (unsigned char *)entity_alias, strlen(entity_alias), NULL, NULL);
	}

	// collect the variable names from bound_vars to populate the Argument op
	const char **arguments = (const char **)raxKeys(bound_vars);

	// build the new Match stream

	QueryCtx_SetAST(plan->ast_segment);
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, path);

	// Create an Apply operator
	const char *rollup_alias = AR_EXP_BuildResolvedName(exp);
	OpBase *apply_op = NewRollUpApplyOp(plan, rollup_alias);
	ExecutionPlan_AddOp(apply_op, match_stream);

	// clean up
	raxFree(bound_vars);
	array_free(arguments);

	return apply_op;
}

void ExecutionPlan_PostBuild(ExecutionPlan *plan) {
	// search for 'topath' function calls within projection operations
	// transform these calls into a RoolUp_Apply operations

	int     op_count  =  0;
	OpBase  *root     =  plan->root;
	OpBase  **ops     =  NULL;
	OPType  type      =  OPType_PROJECT;

	ops = ExecutionPlan_CollectOps(root, type);
	op_count = array_len(ops);

	for(int i = 0; i < op_count; i++) {
		OpProject *project = (OpProject *)ops[i];	
		// see if project op contains a 'topath' call

		for(int j = 0; j < project->exp_count; j++) {
			AR_ExpNode *exp = project->exps[j];
			if(!AR_EXP_ContainsFunc(exp, "topath")) continue;

			// convert topath call into a RollUp_Apply operation
			ASSERT(((OpBase*)project)->childCount > 0);
			OpBase* child_op = ((OpBase*)project)->children[0];
			ASSERT(child_op != NULL);
			ExecutionPlan *child_plan = (ExecutionPlan *)child_op->plan;
			ASSERT(child_plan != NULL);

			OpBase *rollup = buildRollUpMatchStream(child_plan, exp);
			// connect rollup operation as the only child of project
			ExecutionPlan_PushBelow(child_op, rollup);

			// swap roolup children
			OpBase *tmp = rollup->children[0];
			rollup->children[0] = rollup->children[1];
			rollup->children[1] = tmp;

			// TODO: replace 'exp' with a simple variable lookup
		}
	}

	// clean up
	array_free(ops);
}
