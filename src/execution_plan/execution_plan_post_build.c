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

OpBase *buildRollUpMatchStream(ExecutionPlan *plan, AR_ExpNode *exp,
							   AR_ExpNode *path_exp, const char **arguments) {
	ASSERT(plan != NULL);
	ASSERT(exp != NULL);
	ASSERT(path_exp != NULL);

	// extract pattern path
	SIValue val = AR_EXP_Evaluate(path_exp->op.children[0], NULL);
	ASSERT(SI_TYPE(val) & T_PTR);
	const cypher_astnode_t *path = (const cypher_astnode_t *)val.ptrval;

	AR_ExpNode *comprehension_exp = AR_EXP_ContainsFunc(exp, "pattern_comprehension");
	if(comprehension_exp) {
		AR_EXP_RemoveChild(comprehension_exp, 0);
	}

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

void ExecutionPlan_PostBuild(ExecutionPlan *plan) {
	// search for 'topath' function calls within projection operations
	// transform these calls into a RollUp_Apply operations

	uint    op_count  =  0;
	OpBase  *root     =  plan->root;
	OpBase  **ops     =  NULL;
	OPType  type      =  OPType_PROJECT;

	ops = ExecutionPlan_CollectOps(root, type);
	op_count = array_len(ops);

	for(uint i = 0; i < op_count; i++) {
		OpProject *project = (OpProject *)ops[i];

		// see if project op contains a 'topath' call
		for(uint j = 0; j < project->exp_count; j++) {
			AR_ExpNode *path_exp = AR_EXP_ContainsFunc(project->exps[j],
													   "pattern_path");
			if(path_exp == NULL) continue;
			ASSERT(path_exp->op.child_count == 1);
			path_exp = path_exp->op.children[0];
			AR_EXP_RemovePlaceholderFuncs(NULL, &project->exps[j]);
			AR_ExpNode *exp = project->exps[j];
			/*

			// Retrieve or build the identifier for this path projection
			const char *identifier = path_exp->resolved_name;
			if(identifier == NULL) {
			    // Build an identifier for the path expression
			    identifier = AR_EXP_BuildResolvedName(path_exp);
			    path_exp->resolved_name = identifier;
			    // Ensure the new alias will be freed
			    AST *ast = QueryCtx_GetAST();
			    raxInsert(ast->canonical_entity_names, (unsigned char *)identifier,
			              strlen(identifier), (char *)identifier, NULL);
			}

			// Replace the placeholder node with its child
			AR_ExpNode *tmp_exp = AR_EXP_Clone(path_exp->op.children[0]);
			AR_EXP_ReplaceFunc(&exp, "pattern_path", tmp_exp);
			path_exp = tmp_exp;
			path_exp->resolved_name = identifier;

			*/
			// convert topath call into a RollUp_Apply operation
			ASSERT(project->op.childCount > 0);
			OpBase *child_op = project->op.children[0];
			ASSERT(child_op != NULL);
			ExecutionPlan *child_plan = (ExecutionPlan *)child_op->plan;
			ASSERT(child_plan != NULL);

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
			const char *alias = exp->resolved_name;
			OpBase *rollup = buildRollUpMatchStream(child_plan, exp, path_exp, arguments);
			// connect rollup operation as the only child of project
			ExecutionPlan_PushBelow(child_op, rollup);

			// swap rollup children
			OpBase *tmp = rollup->children[0];
			rollup->children[0] = rollup->children[1];
			rollup->children[1] = tmp;

			// replace path expression with variable lookup in the current projection
			AR_ExpNode *replacement = AR_EXP_NewVariableOperandNode(alias);
			replacement->resolved_name = alias;
			project->exps[j] = replacement;
			// AR_ExpNode *replacement = AR_EXP_NewVariableOperandNode(resolved_name);
			// replacement->resolved_name = identifier;
			// AR_ExpNode *clone = AR_EXP_Clone(exp);
			// AR_EXP_ReplaceFunc(&exp, "topath", replacement);
			// project->exps[j] = exp;
		}
	}

	// clean up
	array_free(ops);
}

