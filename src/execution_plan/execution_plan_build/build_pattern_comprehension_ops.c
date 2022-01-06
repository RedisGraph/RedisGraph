/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "execution_plan_construct.h"
#include "RG.h"
#include "../ops/ops.h"
#include "../../query_ctx.h"
#include "../../util/rax_extensions.h"
#include "../../ast/ast_build_filter_tree.h"
#include "../execution_plan_build/execution_plan_modify.h"
#include "../../arithmetic/arithmetic_expression_construct.h"

// build pattern comprehension plan operations for example:
// RETURN [p = (n)-->() | p] AS ps
// Results
//     Project
//         Optional
//             Aggregate
//                 Conditional Traverse | (n)-[anon_0]->(anon_1)
//                     All Node Scan | (n)
//
// MATCH (n) RETURN [p = (n)-->() | p] AS ps
// Results
//     Project
//         Apply
//             All Node Scan | (n)
//             Optional
//                 Aggregate
//                     Conditional Traverse | (n)-[anon_1]->(anon_2)
//                         Argument
void buildPatternComprehensionOps(
	ExecutionPlan *plan,
	OpBase *root,
	const cypher_astnode_t *ast
) {
	const cypher_astnode_t **pcs =
		AST_GetTypedNodes(ast, CYPHER_AST_PATTERN_COMPREHENSION);
	uint count = array_len(pcs);

	AST *prev_ast = QueryCtx_GetAST();
	QueryCtx_SetAST(plan->ast_segment);

	const char **arguments = NULL;
	if(root->childCount > 0) {
		// get the bound variable to use when building the traversal ops
		rax *bound_vars = raxNew();
		ExecutionPlan_BoundVariables(root->children[0], bound_vars);
		arguments = (const char **)raxValues(bound_vars);
		raxFree(bound_vars);
	}

	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *path =
			cypher_ast_pattern_comprehension_get_pattern(pcs[i]);

		OpBase *match_stream =
			ExecutionPlan_BuildOpsFromPath(plan, arguments, path);

		const cypher_astnode_t *eval_node =
			cypher_ast_pattern_comprehension_get_eval(pcs[i]);
		
		AR_ExpNode *eval_exp = AR_EXP_FromASTNode(eval_node);

		AR_ExpNode *collect_exp = AR_EXP_NewOpNode("collect", 1);
		collect_exp->op.children[0] = eval_exp;
		collect_exp->resolved_name = AST_ToString(pcs[i]);

		AR_ExpNode **exps = array_new(AR_ExpNode *, 1);
		array_append(exps, collect_exp);
		OpBase *aggregate = NewAggregateOp(plan, exps, false);
		OpBase *optional = NewOptionalOp(plan);
		ExecutionPlan_AddOp(optional, aggregate);

		const cypher_astnode_t *predicate =
			cypher_ast_pattern_comprehension_get_predicate(pcs[i]);
		if(predicate == NULL) {
			ExecutionPlan_AddOp(aggregate, match_stream);
		} else {
			FT_FilterNode *filter_tree = NULL;
			AST_ConvertFilters(&filter_tree, predicate);
			OpBase *filter = NewFilterOp(plan, filter_tree);
			ExecutionPlan_AddOp(filter, match_stream);
			ExecutionPlan_AddOp(aggregate, filter);
		}

		if(root->childCount > 0) {
			OpBase *apply_op = NewApplyOp(plan);
			ExecutionPlan_PushBelow(root->children[0], apply_op);
			ExecutionPlan_AddOp(apply_op, optional);
		} else {
			ExecutionPlan_AddOp(root, optional);
		}
	}

	QueryCtx_SetAST(prev_ast);
	if(arguments != NULL) array_free(arguments);
	array_free(pcs);
}

// build pattern path plan operations for example:
// RETURN ()-->() AS ps
// Results
//     Project
//         Optional
//             Aggregate
//                 Conditional Traverse | (anon_0)-[anon_1]->(anon_2)
//                     All Node Scan | (anon_0)
//
// MATCH (n) RETURN (n)-->() AS ps
// Results
//     Project
//         Apply
//             All Node Scan | (n)
//             Optional
//                 Aggregate
//                     Conditional Traverse | (n)-[anon_1]->(anon_2)
//                         Argument
void buildPatternPathOps(
	ExecutionPlan *plan,
	OpBase *root,
	const cypher_astnode_t *ast
) {
	const cypher_astnode_t **pcs =
		AST_GetTypedNodes(ast, CYPHER_AST_PATTERN_COMPREHENSION);
	const cypher_astnode_t **pps =
		AST_GetTypedNodes(ast, CYPHER_AST_PATTERN_PATH);
	uint count = array_len(pps);
	uint pcs_count = array_len(pcs);

	AST *prev_ast = QueryCtx_GetAST();
	QueryCtx_SetAST(plan->ast_segment);

	const char **arguments = NULL;
	if(root->childCount > 0) {
		// get the bound variable to use when building the traversal ops
		rax *bound_vars = raxNew();
		ExecutionPlan_BoundVariables(root->children[0], bound_vars);
		arguments = (const char **)raxValues(bound_vars);
		raxFree(bound_vars);
	}

	for (uint i = 0; i < count; i++) {
		const cypher_astnode_t *path = pps[i];
		
		bool is_in_pattern_comprehension = false;
		for (uint j = 0; j < pcs_count; j++) {
			if(cypher_ast_pattern_comprehension_get_pattern(pcs[j]) == path) {
				is_in_pattern_comprehension = true;
				break;
			}
		}

		// if this pattern path is in pattern comprehension
		// we already built the ops in _BuildPatternComprehensionOps
		if(is_in_pattern_comprehension) continue;

		OpBase *match_stream =
			ExecutionPlan_BuildOpsFromPath(plan, arguments, path);

		uint path_len = cypher_ast_pattern_path_nelements(path);
		AR_ExpNode *path_exp = AR_EXP_NewOpNode("topath", 1 + path_len);
		path_exp->op.children[0] =
			AR_EXP_NewConstOperandNode(SI_PtrVal((void *)path));
		for(uint j = 0; j < path_len; j ++) {
			path_exp->op.children[j + 1] =
				AR_EXP_FromASTNode(cypher_ast_pattern_path_get_element(path, j));
		}

		AR_ExpNode *collect_exp = AR_EXP_NewOpNode("collect", 1);
		collect_exp->op.children[0] = path_exp;
		collect_exp->resolved_name = AST_ToString(path);

		AR_ExpNode **exps = array_new(AR_ExpNode *, 1);
		array_append(exps, collect_exp);
		OpBase *aggregate = NewAggregateOp(plan, exps, false);
		OpBase *optional = NewOptionalOp(plan);
		ExecutionPlan_AddOp(optional, aggregate);
		ExecutionPlan_AddOp(aggregate, match_stream);

		if(root->childCount > 0) {
			OpBase *apply_op = NewApplyOp(plan);
			ExecutionPlan_PushBelow(root->children[0], apply_op);
			ExecutionPlan_AddOp(apply_op, optional);
		} else {
			ExecutionPlan_AddOp(root, optional);
		}
	}

	QueryCtx_SetAST(prev_ast);
	if(arguments != NULL) array_free(arguments);
	array_free(pps);
	array_free(pcs);
}
