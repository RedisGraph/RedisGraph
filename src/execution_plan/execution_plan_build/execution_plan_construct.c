#include "execution_plan_construct.h"
#include "execution_plan_modify.h"
#include "../execution_plan.h"
#include "../../RG.h"
#include "../ops/ops.h"
#include "../../query_ctx.h"
#include "../../util/rax_extensions.h"
#include "../../ast/ast_build_ar_exp.h"
#include "../../ast/ast_build_filter_tree.h"
#include "../../ast/ast_build_op_contexts.h"

/* _BuildCallProjections creates an array of expression nodes to populate a Project operation with.
 * All Strings in the YIELD block of a CALL clause are represented, or the procedure-registered
 * outputs if the YIELD block is missing. */
static AR_ExpNode **_BuildCallProjections(const cypher_astnode_t *call_clause, AST *ast) {
	// Handle yield entities
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	AR_ExpNode **expressions = array_new(AR_ExpNode *, yield_count);

	for(uint i = 0; i < yield_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);

		const char *identifier = NULL;
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			identifier = cypher_ast_identifier_get_name(alias_node);
		} else {
			// This expression did not have an alias, so it must be an identifier
			assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
			identifier = cypher_ast_identifier_get_name(ast_exp);
		}

		exp->resolved_name = identifier;
		expressions = array_append(expressions, exp);
	}

	// If the procedure call is missing its yield part, include procedure outputs.
	if(yield_count == 0) {
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		ProcedureCtx *proc = Proc_Get(proc_name);
		assert(proc);

		unsigned int output_count = Procedure_OutputCount(proc);
		for(uint i = 0; i < output_count; i++) {
			const char *name = Procedure_GetOutput(proc, i);
			AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(name, NULL);
			exp->resolved_name = name;
			expressions = array_append(expressions, exp);
		}
		Proc_Free(proc);
	}

	return expressions;
}

/* Strings enclosed in the parentheses of a CALL clause represent the arguments to the procedure.
 * _BuildCallArguments creates a string array holding all of these arguments. */
static AR_ExpNode **_BuildCallArguments(const cypher_astnode_t *call_clause) {
	// Handle argument entities
	uint arg_count = cypher_ast_call_narguments(call_clause);
	AR_ExpNode **arguments = array_new(AR_ExpNode *, arg_count);
	for(uint i = 0; i < arg_count; i ++) {
		const cypher_astnode_t *exp = cypher_ast_call_get_argument(call_clause, i);
		AR_ExpNode *arg = AR_EXP_FromExpression(exp);
		arguments = array_append(arguments, arg);
	}

	return arguments;
}

static void _ExecutionPlan_PlaceApplyOps(ExecutionPlan *plan) {
	OpBase **filter_ops = ExecutionPlan_CollectOps(plan->root, OPType_FILTER);
	uint filter_ops_count = array_len(filter_ops);
	for(uint i = 0; i < filter_ops_count; i++) {
		OpFilter *op = (OpFilter *)filter_ops[i];
		FT_FilterNode *node;
		if(FilterTree_ContainsFunc(op->filterTree, "path_filter", &node)) {
			ExecutionPlan_ReduceFilterToApply(plan, op);
		}
	}
	array_free(filter_ops);
}

void ExecutionPlan_RePositionFilterOp(ExecutionPlan *plan, OpBase *lower_bound,
									  const OpBase *upper_bound, OpBase *filter) {
	ASSERT(filter->type == OPType_FILTER);
	OpBase *op = NULL;
	rax *references = FilterTree_CollectModified(((OpFilter *)filter)->filterTree);
	uint64_t references_count = raxSize(references);

	if(references_count > 0) {
		/* Scan execution plan, locate the earliest position where all
		 * references been resolved. */
		op = ExecutionPlan_LocateReferencesExcludingOps(lower_bound, upper_bound, FILTER_RECURSE_BLACKLIST,
														FILTER_RECURSE_BLACKLIST_COUNT, references);
		if(!op) {
			// Something is wrong - could not find a matching op where all references are solved.
			unsigned char **entities = raxKeys(references);
			char *entities_str;
			asprintf(&entities_str, "%s", entities[0]);
			for(uint64_t i = 1; i < references_count; i++) {
				asprintf(&entities_str, "%s, %s", entities_str, entities[i]);
			}
			// Build-time error - execution plan will not run.
			QueryCtx_SetError("Unable to place filter op for entities: %s", entities_str);
			// Cleanup.
			OpBase_Free(filter);
			free(entities_str);
			for(uint64_t i = 0; i < references_count; i++) rm_free(entities[i]);
			array_free(entities);
			raxFree(references);
			return;
		}
	} else {
		/* The filter tree does not contain references, like:
		 * WHERE 1=1
		 * TODO This logic is inadequate. For now, we'll place the op
		 * directly below the first projection (hopefully there is one!). */
		op = plan->root;
		while(op->childCount > 0 && op->type != OPType_PROJECT && op->type != OPType_AGGREGATE) {
			op = op->children[0];
		}
	}
	ASSERT(op != NULL);

	// In case this is a pre-existing filter (this function is not called out from ExecutionPlan_PlaceFilterOps)
	if(filter->childCount > 0) {
		// If the located op is not the filter child, re position the filter.
		if(op != filter->children[0]) {
			ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
			ExecutionPlan_PushBelow(op, (OpBase *)filter);
		}
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
	FT_FilterNode **sub_trees = FilterTree_SubTrees(ft);

	/* For each filter tree find the earliest position along the execution
	 * after which the filter tree can be applied. */
	while(array_len(sub_trees) > 0) {
		FT_FilterNode *tree = array_pop(sub_trees);
		OpBase *filter_op = NewFilterOp(plan, tree);
		ExecutionPlan_RePositionFilterOp(plan, root, recurse_limit, filter_op);
	}
	array_free(sub_trees);
	_ExecutionPlan_PlaceApplyOps(plan);
}

// Convert a CALL clause into a procedure call operation.
static inline void _buildCallOp(AST *ast, ExecutionPlan *plan,
								const cypher_astnode_t *call_clause) {
	// A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
	AR_ExpNode **arguments = _BuildCallArguments(call_clause);
	AR_ExpNode **yield_exps = _BuildCallProjections(call_clause, ast); // TODO only need strings
	OpBase *op = NewProcCallOp(plan, proc_name, arguments, yield_exps);
	ExecutionPlan_UpdateRoot(plan, op);
	// Build the FilterTree to model any WHERE predicates on this clause and place ops appropriately.
	FT_FilterNode *sub_ft = AST_BuildFilterTreeFromClauses(ast, &call_clause, 1);
	ExecutionPlan_PlaceFilterOps(plan, plan->root, NULL, sub_ft);
}

static inline void _buildCreateOp(GraphContext *gc, AST *ast, ExecutionPlan *plan) {
	AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(plan->query_graph, plan->record_map);
	OpBase *op = NewCreateOp(plan, create_ast_ctx.nodes_to_create, create_ast_ctx.edges_to_create);
	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUnwindOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(clause);
	OpBase *op = NewUnwindOp(plan, unwind_ast_ctx.exp);
	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUpdateOp(GraphContext *gc, ExecutionPlan *plan,
								  const cypher_astnode_t *clause) {
	EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(gc, clause);
	OpBase *op = NewUpdateOp(plan, update_exps);
	array_free(update_exps);
	ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildDeleteOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **exps = AST_PrepareDeleteOp(clause);
	OpBase *op = NewDeleteOp(plan, exps);
	ExecutionPlan_UpdateRoot(plan, op);
}

void ExecutionPlanSegment_ConvertClause(GraphContext *gc, AST *ast, ExecutionPlan *plan,
										const cypher_astnode_t *clause) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	// Because 't' is set using the offsetof() call, it cannot be used in switch statements.
	if(t == CYPHER_AST_MATCH) {
		buildMatchOpTree(plan, ast, clause);
	} else if(t == CYPHER_AST_CALL) {
		_buildCallOp(ast, plan, clause);
	} else if(t == CYPHER_AST_CREATE) {
		// Only add at most one Create op per plan. TODO Revisit and improve this logic.
		if(ExecutionPlan_LocateOp(plan->root, OPType_CREATE)) return;
		_buildCreateOp(gc, ast, plan);
	} else if(t == CYPHER_AST_UNWIND) {
		_buildUnwindOp(plan, clause);
	} else if(t == CYPHER_AST_MERGE) {
		buildMergeOp(plan, ast, clause, gc);
	} else if(t == CYPHER_AST_SET) {
		_buildUpdateOp(gc, plan, clause);
	} else if(t == CYPHER_AST_DELETE) {
		_buildDeleteOp(plan, clause);
	} else if(t == CYPHER_AST_RETURN) {
		// Converting a RETURN clause can create multiple operations.
		buildReturnOps(plan, clause);
	} else if(t == CYPHER_AST_WITH) {
		// Converting a WITH clause can create multiple operations.
		buildWithOps(plan, clause);
	}
}

