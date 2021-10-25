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

static OpBase *buildRollUpMatchStream(ExecutionPlan *plan, AR_ExpNode *exp,
									  AR_ExpNode *path_exp, const char **arguments) {
	ASSERT(plan != NULL);
	ASSERT(exp != NULL);
	ASSERT(path_exp != NULL);

	// extract pattern path
	SIValue val = AR_EXP_Evaluate(path_exp->op.children[0], NULL);
	ASSERT(SI_TYPE(val) & T_PTR);
	const cypher_astnode_t *path = (const cypher_astnode_t *)val.ptrval;

	// build the new Match stream
	QueryCtx_SetAST(plan->ast_segment);
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, path);

	// Build a Project op to project the path expression being matched.
	uint arg_count = array_len(arguments);
	AR_ExpNode **exps = array_new(AR_ExpNode *, 1);
	array_append(exps, exp);
	OpBase *project = NewProjectOp(plan, exps);
	// Make the Project op the root of the Match stream.
	ExecutionPlan_AddOp(project, match_stream);

	// Create an Apply operator
	OpBase *apply_op = NewRollUpApplyOp(plan, exp->resolved_name);
	ExecutionPlan_AddOp(apply_op, project);

	// clean up
	array_free(arguments);

	return apply_op;
}

static void replaceProjectionExps(ExecutionPlan *plan, OpBase *op,
								  AR_ExpNode **exps) {
	uint exp_count = array_len(exps);
	for(uint i = 0; i < exp_count; i++) {
		AR_ExpNode *path_exp = AR_EXP_ContainsFunc(exps[i], "pattern_path");
		if(path_exp == NULL) continue;
		ASSERT(path_exp->op.child_count == 1);
		path_exp = path_exp->op.children[0];
		AR_EXP_RemovePlaceholderFuncs(NULL, &exps[i]);

		// convert topath call into a RollUp_Apply operation
		ASSERT(op->childCount > 0);
		OpBase *child_op = op->children[0];
		ASSERT(child_op != NULL);
		ExecutionPlan *child_plan = (ExecutionPlan *)child_op->plan;
		ASSERT(child_plan != NULL);

		// replace path expression with variable lookup in the current projection
		AR_ExpNode *comprehension_exp = AR_EXP_ContainsFunc(exps[i],
															"pattern_comprehension");
		AR_ExpNode *container_exp = NULL;
		if(comprehension_exp == NULL) {
			// if we're replacing a path projection,
			// add an identifier to the path exp
			// and replace it within the Project op's expression
			// with a variable lookup
			const char *identifier = exps[i]->resolved_name;
			path_exp->resolved_name = identifier;
			path_exp = AR_EXP_Clone(path_exp);
			AR_ExpNode *replacement = AR_EXP_NewVariableOperandNode(identifier);
			replacement->resolved_name = identifier;
			AR_EXP_ReplaceFunc(&exps[i], "topath", replacement);
			container_exp = path_exp;
		} else {
			// if we're replacing a pattern comprehension,
			// extract the path exp,
			// add an identifier to the comprehension,
			// replace it within the Project op's expression
			// with a variable lookup,
			// and place just the pattern comprehension into the RollUp's
			// Project child.
			comprehension_exp = AR_EXP_Clone(comprehension_exp);
			path_exp = comprehension_exp->op.children[0];
			AR_EXP_RemoveChild(comprehension_exp, 0);
			const char *identifier = comprehension_exp->resolved_name;
			if(identifier == NULL) {
				// Build an identifier for the path expression
				identifier = AR_EXP_BuildResolvedName(comprehension_exp);
				comprehension_exp->resolved_name = identifier;
				// Ensure the new alias will be freed
				AST *ast = QueryCtx_GetAST();
				raxInsert(ast->canonical_entity_names, (unsigned char *)identifier,
						  strlen(identifier), (char *)identifier, NULL);
			}
			comprehension_exp->resolved_name = identifier;
			AR_ExpNode *replacement = AR_EXP_NewVariableOperandNode(identifier);
			replacement->resolved_name = identifier;
			AR_EXP_ReplaceFunc(&exps[i], "pattern_comprehension", replacement);
			container_exp = comprehension_exp;
		}

		// build arguments array to pass to RollUpApply subtree
		const char **arguments = NULL;
		if(plan->root) {
			// Collect the variables that are bound at this point.
			rax *bound_vars = raxNew();
			// Rather than cloning the record map, collect the bound variables along with their
			// parser-generated constant strings.
			ExecutionPlan_BoundVariables(plan->root, bound_vars);
			ExecutionPlan_BoundVariables(child_op, bound_vars);
			// Collect the variable names from bound_vars to populate the Argument op we will build.
			arguments = (const char **)raxValues(bound_vars);
			raxFree(bound_vars);
		}

		OpBase *rollup = buildRollUpMatchStream(child_plan, container_exp,
			   	path_exp, arguments);
		// connect rollup operation as the only child of project
		ExecutionPlan_PushBelow(child_op, rollup);

		// swap rollup children
		OpBase *tmp = rollup->children[0];
		rollup->children[0] = rollup->children[1];
		rollup->children[1] = tmp;
	}
}

void ExecutionPlan_PostBuild(ExecutionPlan *plan) {
	// search for 'topath' function calls within projection operations
	// transform these calls into a RollUp_Apply operations
	uint    op_count  =  0;
	OpBase  *root     =  plan->root;
	OpBase  **ops     =  NULL;

	// check all Project ops
	ops = ExecutionPlan_CollectOps(root, OPType_PROJECT);
	op_count = array_len(ops);
	for(uint i = 0; i < op_count; i++) {
		AR_ExpNode **exps = ((OpProject *)ops[i])->exps;
		replaceProjectionExps(plan, ops[i], exps);
	}
	array_free(ops);

	// check all Aggregate ops
	ops = ExecutionPlan_CollectOps(root, OPType_AGGREGATE);
	op_count = array_len(ops);
	for(uint i = 0; i < op_count; i++) {
		AR_ExpNode **exps = ((OpAggregate *)ops[i])->key_exps;
		replaceProjectionExps(plan, ops[i], exps);
		exps = ((OpAggregate *)ops[i])->aggregate_exps;
		replaceProjectionExps(plan, ops[i], exps);
	}
	array_free(ops);
}

