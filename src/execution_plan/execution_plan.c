/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/rmalloc.h"
#include "../util/arr.h"
#include "../util/vector.h"
#include "../util/qsort.h"
#include "../graph/entities/edge.h"
#include "./optimizations/optimizer.h"
#include "./optimizations/optimizations.h"
#include "../arithmetic/algebraic_expression.h"
#include "../ast/ast_build_op_contexts.h"
#include "../ast/ast_build_filter_tree.h"

/* In a query with a RETURN *, build projections for all explicit aliases
 * in previous clauses. */
// static AR_ExpNode **_ReturnExpandAll(AST *ast) {
static ProjectedExpression *_ReturnExpandAll(AST *ast) {
	// Collect all unique aliases
	const char **aliases = AST_CollectElementNames(ast);
	uint count = array_len(aliases);

	// Build an expression for each alias
	ProjectedExpression *return_expressions = array_new(ProjectedExpression, count);
	for(int i = 0; i < count; i ++) {
		AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(aliases[i], NULL);
		exp->resolved_name = aliases[i];

		ProjectedExpression proj_exp;
		proj_exp.exp = exp;
		proj_exp.alias = NULL;
		proj_exp.exp_str = aliases[i];
		return_expressions = array_append(return_expressions, proj_exp);
	}

	array_free(aliases);
	return return_expressions;
}

// Handle ORDER entities
static AR_ExpNode **_BuildOrderExpressions(const cypher_astnode_t *order_clause) {
	uint count = cypher_ast_order_by_nitems(order_clause);
	AR_ExpNode **order_exps = array_new(AR_ExpNode *, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);
		order_exps = array_append(order_exps, exp);
		// TODO direction should be specifiable per order entity
		// ascending = cypher_ast_sort_item_is_ascending(item);
	}
	return order_exps;
}

// Handle RETURN entities
// (This function is not static because it is relied upon by unit tests)
// AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause, AST *ast) {
ProjectedExpression *_BuildReturnExpressions(const cypher_astnode_t *ret_clause, AST *ast) {
	// Query is of type "RETURN *",
	// collect all defined identifiers and create return elements for them
	if(cypher_ast_return_has_include_existing(ret_clause)) return _ReturnExpandAll(ast);

	uint count = cypher_ast_return_nprojections(ret_clause);
	ProjectedExpression *return_expressions = array_new(ProjectedExpression, count);

	for(uint i = 0; i < count; i++) {
		ProjectedExpression proj_exp;
		proj_exp.exp = NULL;
		proj_exp.alias = NULL;
		proj_exp.exp_str = NULL;

		const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
		// The AST expression can be an identifier, function call, or constant
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this return entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);
		proj_exp.exp = exp;

		// Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
		// the entity.prop combination ("a.val"), or the function call ("MAX(a.val)")
		const char *identifier = NULL;
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			identifier = cypher_ast_identifier_get_name(alias_node);
			proj_exp.alias = identifier;
		} else {
			// This expression did not have an alias, so it must be an identifier
			assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
			identifier = cypher_ast_identifier_get_name(ast_exp);
		}
		exp->resolved_name = identifier;
		proj_exp.exp_str = identifier;
		return_expressions = array_append(return_expressions, proj_exp);
	}

	return return_expressions;
}

static ProjectedExpression *_BuildWithExpressions(const cypher_astnode_t *with_clause) {
	uint count = cypher_ast_with_nprojections(with_clause);
	ProjectedExpression *with_expressions = array_new(ProjectedExpression, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);
		const char *identifier = cypher_ast_identifier_get_name(ast_exp);

		ProjectedExpression proj_exp;
		proj_exp.exp = exp;
		proj_exp.alias = NULL;
		proj_exp.exp_str = identifier;

		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			/// TODO should issue syntax failure in the latter 2 cases
			identifier = cypher_ast_identifier_get_name(alias_node);
			proj_exp.alias = identifier;
		}
		exp->resolved_name = identifier;

		with_expressions = array_append(with_expressions, proj_exp);
	}

	return with_expressions;
}

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
static const char **_BuildCallArguments(const cypher_astnode_t *call_clause) {
	// Handle argument entities
	uint arg_count = cypher_ast_call_narguments(call_clause);
	const char **arguments = array_new(const char *, arg_count);
	for(uint i = 0; i < arg_count; i ++) {
		/* For the timebeing we're only supporting procedures that accept
		 * string argument, as such we can quickly transfer AST procedure
		 * call arguments to strings.
		 * TODO: create an arithmetic expression for each argument
		 * evaluate it and pass SIValues as arguments to procedure. */
		const cypher_astnode_t *exp = cypher_ast_call_get_argument(call_clause, i);
		const cypher_astnode_type_t type = cypher_astnode_type(exp);

		if(type != CYPHER_AST_STRING) continue;

		const char *arg = cypher_ast_string_get_value(exp);
		arguments = array_append(arguments, arg);
	}

	return arguments;
}

static void _ExecutionPlan_ProcessQueryGraph(ExecutionPlan *plan, QueryGraph *qg,
											 AST *ast, FT_FilterNode *ft, Vector *ops) {
	GraphContext *gc = GraphContext_GetFromTLS();

	QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
	uint connectedComponentsCount = array_len(connectedComponents);
	plan->connected_components = connectedComponents;

	/* For every connected component.
	 * Incase we're dealing with multiple components
	 * we'll simply join them all together with a join operation. */
	OpBase *cartesianProduct = NULL;
	if(connectedComponentsCount > 1) {
		cartesianProduct = NewCartesianProductOp(plan);
		Vector_Push(ops, cartesianProduct);
	}

	// Keep track after all traversal operations along a pattern.
	Vector *traversals = NewVector(OpBase *, 1);
	OpBase *op;

	for(uint i = 0; i < connectedComponentsCount; i++) {
		QueryGraph *cc = connectedComponents[i];
		uint edge_count = array_len(cc->edges);
		if(edge_count == 0) {
			/* Node scan. */
			QGNode *n = cc->nodes[0];
			if(n->labelID != GRAPH_NO_LABEL) op = NewNodeByLabelScanOp(plan, n);
			else op = NewAllNodeScanOp(plan, gc->g, n);
			Vector_Push(traversals, op);
		} else {
			size_t expCount = 0;
			AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc, &expCount);

			// Reorder exps, to the most performant arrangement of evaluation.
			orderExpressions(exps, expCount, ft);

			AlgebraicExpression *exp = exps[0];
			selectEntryPoint(exp, ft);

			// Create SCAN operation.
			if(exp->src_node->label) {
				/* Resolve source node by performing label scan,
				 * in which case if the first algebraic expression operand
				 * is a label matrix (diagonal) remove it, otherwise
				 * the label matrix associated with source's label is located
				 * within another traversal operation, for the timebeing do not
				 * try to locate and remove it, there's no real harm except some performace hit
				 * in keeping that label matrix. */
				if(exp->operands[0].diagonal) AlgebraicExpression_RemoveTerm(exp, 0, NULL);
				op = NewNodeByLabelScanOp(plan, exp->src_node);
			} else {
				op = NewAllNodeScanOp(plan, gc->g, exp->src_node);
			}
			Vector_Push(traversals, op);

			for(int i = 0; i < expCount; i++) {
				exp = exps[i];
				if(exp->operand_count == 0) continue;

				if(exp->edge && QGEdge_VariableLength(exp->edge)) {
					op = NewCondVarLenTraverseOp(plan, gc->g, exp);
				} else {
					op = NewCondTraverseOp(plan, gc->g, exp, TraverseRecordCap(ast));
				}

				Vector_Push(traversals, op);
			}

			// Free the expressions array, as its parts have been converted into operations
			rm_free(exps);
		}

		if(connectedComponentsCount > 1) {
			// Connect traversal operations.
			OpBase *childOp;
			OpBase *parentOp;
			Vector_Pop(traversals, &parentOp);
			// Connect cartesian product to the root of traversal.
			ExecutionPlan_AddOp(cartesianProduct, parentOp);
			while(Vector_Pop(traversals, &childOp)) {
				ExecutionPlan_AddOp(parentOp, childOp);
				parentOp = childOp;
			}
		} else {
			for(int traversalIdx = 0; traversalIdx < Vector_Size(traversals); traversalIdx++) {
				Vector_Get(traversals, traversalIdx, &op);
				Vector_Push(ops, op);
			}
		}
		Vector_Clear(traversals);
	}

	Vector_Free(traversals);
}

static ExecutionPlan *_NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, AST *ast,
										ResultSet *result_set) {

	ExecutionPlan *plan = rm_calloc(1, sizeof(ExecutionPlan));
	plan->record_map = raxNew();
	plan->result_set = result_set;
	plan->connected_components = NULL;

	/* Build projections from this AST's WITH, RETURN, and ORDER clauses. */
	// Retrieve a RETURN clause if one is specified in this AST's range
	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	// Retrieve a WITH clause if one is specified in this AST's range
	const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
	// We cannot have both a RETURN and WITH clause
	assert(!(ret_clause && with_clause));

	const cypher_astnode_t *order_clause = NULL;
	if(ret_clause) {
		order_clause = cypher_ast_return_get_order_by(ret_clause);
	} else if(with_clause) {
		order_clause = cypher_ast_with_get_order_by(with_clause);
	}

	Vector *ops = NewVector(OpBase *, 1);

	// Build query graph
	QueryGraph *qg = BuildQueryGraph(gc, ast);
	plan->query_graph = qg;

	// Build filter tree
	FT_FilterNode *filter_tree = AST_BuildFilterTree(ast);
	plan->filter_tree = filter_tree;

	const cypher_astnode_t *call_clause = AST_GetClause(ast, CYPHER_AST_CALL);
	if(call_clause) {
		// A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		const char **arguments = _BuildCallArguments(call_clause);
		AR_ExpNode **yield_exps = _BuildCallProjections(call_clause, ast);
		uint yield_count = array_len(yield_exps);
		const char **yields = array_new(const char *, yield_count);

		for(uint i = 0; i < yield_count; i ++) {
			// Track the names of yielded variables.
			yields = array_append(yields, yield_exps[i]->operand.variadic.entity_alias);
		}
		array_free(yield_exps);
		OpBase *opProcCall = NewProcCallOp(plan, proc_name, arguments, yields);
		Vector_Push(ops, opProcCall);
	}

	// Build traversal operations for every connected component in the QueryGraph
	if(AST_ContainsClause(ast, CYPHER_AST_MATCH) || AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
		_ExecutionPlan_ProcessQueryGraph(plan, qg, ast, filter_tree, ops);
	}

	// If we are in a querying context, retrieve a pointer to the statistics for operations
	// like DELETE that only produce metadata.
	ResultSetStatistics *stats = (result_set) ? &result_set->stats : NULL;

	// Set root operation
	const cypher_astnode_t *unwind_clause = AST_GetClause(ast, CYPHER_AST_UNWIND);
	if(unwind_clause) {
		AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(unwind_clause);
		OpBase *opUnwind = NewUnwindOp(plan, unwind_ast_ctx.alias, unwind_ast_ctx.exps);
		Vector_Push(ops, opUnwind);
	}

	bool create_clause = AST_ContainsClause(ast, CYPHER_AST_CREATE);
	if(create_clause) {
		OpBase *opCreate = NewCreateOp(plan, stats, ast);
		Vector_Push(ops, opCreate);
	}

	const cypher_astnode_t *merge_clause = AST_GetClause(ast, CYPHER_AST_MERGE);
	if(merge_clause) {
		/* A merge clause provides a single path that must exist or be created.
		 * As with paths in a MATCH query, build the appropriate traversal operations
		 * and append them to the set of ops.
		 * Append a merge operation */
		OpBase *opMerge = NewMergeOp(plan, stats, ast);
		Vector_Push(ops, opMerge);
	}

	const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
	if(delete_clause) {
		char **deleted_entities = AST_PrepareDeleteOp(delete_clause);
		OpBase *opDelete = NewDeleteOp(plan, qg, deleted_entities, stats);
		array_free(deleted_entities);
		Vector_Push(ops, opDelete);
	}

	const cypher_astnode_t *set_clause = AST_GetClause(ast, CYPHER_AST_SET);
	if(set_clause) {
		// Create a context for each update expression.
		uint nitems;
		EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(set_clause, &nitems);
		OpBase *op_update = NewUpdateOp(plan, gc, update_exps, nitems, stats);
		Vector_Push(ops, op_update);
	}

	OpBase *op;

	if(with_clause) {
		if(AST_ClauseContainsAggregation(with_clause)) {
			op = NewAggregateOp(plan, _BuildWithExpressions(with_clause));
		} else {
			op = NewProjectOp(plan, _BuildWithExpressions(with_clause));
		}
		Vector_Push(ops, op);

		if(cypher_ast_with_is_distinct(with_clause)) {
			op = NewDistinctOp(plan);
			Vector_Push(ops, op);
		}

		const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(with_clause);
		const cypher_astnode_t *limit_clause = cypher_ast_with_get_limit(with_clause);

		uint skip = 0;
		uint limit = 0;
		if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
		if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

		if(order_clause) {
			const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(with_clause);
			int direction = AST_PrepareSortOp(order_clause);
			// The sort operation will obey a specified limit, but must account for skipped records
			uint sort_limit = (limit > 0) ? limit + skip : 0;
			op = NewSortOp(plan, _BuildOrderExpressions(order_clause), direction, sort_limit);
			Vector_Push(ops, op);
		}

		if(skip_clause) {
			OpBase *op_skip = NewSkipOp(plan, skip);
			Vector_Push(ops, op_skip);
		}

		if(limit_clause) {
			OpBase *op_limit = NewLimitOp(plan, limit);
			Vector_Push(ops, op_limit);
		}
	} else if(ret_clause) {
		/* TODO we may not need a new project op if the query is something like:
		 * MATCH (a) WITH a.val AS val RETURN val
		 * Though we would still need a new projection (barring later optimizations) for:
		 * MATCH (a) WITH a.val AS val RETURN val AS e */
		if(AST_ClauseContainsAggregation(ret_clause)) {
			op = NewAggregateOp(plan, _BuildReturnExpressions(ret_clause, ast));
		} else {
			op = NewProjectOp(plan, _BuildReturnExpressions(ret_clause, ast));
		}
		Vector_Push(ops, op);

		if(cypher_ast_return_is_distinct(ret_clause)) {
			op = NewDistinctOp(plan);
			Vector_Push(ops, op);
		}

		const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(ret_clause);
		const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(ret_clause);
		const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);

		uint skip = 0;
		uint limit = 0;
		if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
		if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

		if(order_clause) {
			int direction = AST_PrepareSortOp(order_clause);
			// The sort operation will obey a specified limit, but must account for skipped records
			uint sort_limit = (limit > 0) ? limit + skip : 0;
			op = NewSortOp(plan, _BuildOrderExpressions(order_clause), direction, sort_limit);
			Vector_Push(ops, op);
		}

		if(skip_clause) {
			OpBase *op_skip = NewSkipOp(plan, skip);
			Vector_Push(ops, op_skip);
		}

		if(limit_clause) {
			OpBase *op_limit = NewLimitOp(plan, limit);
			Vector_Push(ops, op_limit);
		}

		op = NewResultsOp(plan, result_set, qg);
		Vector_Push(ops, op);
	} else if(call_clause) {
		op = NewResultsOp(plan, result_set, qg);
		Vector_Push(ops, op);
	}

	OpBase *parent_op;
	OpBase *child_op;
	Vector_Pop(ops, &parent_op);
	plan->root = parent_op;

	while(Vector_Pop(ops, &child_op)) {
		ExecutionPlan_AddOp(parent_op, child_op);
		parent_op = child_op;
	}

	Vector_Free(ops);

	if(plan->filter_tree) {
		Vector *sub_trees = FilterTree_SubTrees(plan->filter_tree);

		/* For each filter tree find the earliest position along the execution
		 * after which the filter tree can be applied. */
		for(int i = 0; i < Vector_Size(sub_trees); i++) {
			FT_FilterNode *tree;
			Vector_Get(sub_trees, i, &tree);

			rax *references = FilterTree_CollectModified(tree);
			if(raxSize(references) > 0) {
				/* Scan execution segment, locate the earliest position where all
				 * references been resolved. */
				op = ExecutionPlan_LocateReferences(plan->root, references);
				assert(op);
				/* Create filter node.
				* Introduce filter op right below located op. */
				OpBase *filter_op = NewFilterOp(plan, tree);
				ExecutionPlan_PushBelow(op, filter_op);
			} else {
				/* The filter tree does not contain references, like:
				 * WHERE 1=1, Ignore filter. */
			}
			raxFree(references);
		}
		Vector_Free(sub_trees);
	}

	return plan;
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, ResultSet *result_set) {
	AST *ast = AST_GetFromTLS();

	/* Execution plans are created in 1 or more segments. Every WITH clause demarcates the end of
	 * a segment, and the next clause begins a new one. */
	uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
	uint return_clause_count = AST_GetClauseCount(ast, CYPHER_AST_RETURN);
	uint segment_count = with_clause_count + return_clause_count + 1;

	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	if(return_clause_count) {
		uint *return_indices = AST_GetClauseIndices(ast, CYPHER_AST_RETURN);
		segment_indices = array_append(segment_indices, return_indices[0]);
		array_free(return_indices);
	}
	segment_indices = array_append(segment_indices, cypher_astnode_nchildren(ast->root));

	uint start_offset = 0;
	ExecutionPlan *segments[segment_count];

	for(int i = 0; i < segment_count; i++) {
		uint end_offset = segment_indices[i];
		// Slice the AST to only include the clauses in the current segment.
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);

		// Construct a new ExecutionPlanSegment.
		ExecutionPlan *segment = _NewExecutionPlan(ctx, gc, ast_segment, result_set);

		// Free the AST segment (the AST tree is freed separately,
		// since this does not need to be performed the master AST)
		cypher_ast_free((cypher_astnode_t *)ast_segment->root);
		AST_Free(ast_segment); // Free all AST constructions scoped to this segment

		segments[i] = segment;
		start_offset = end_offset;
	}

	array_free(segment_indices);

	// Merge segments.
	for(int i = 1; i < segment_count; i++) {
		ExecutionPlan *prev_segment = segments[i - 1];
		ExecutionPlan *current_segment = segments[i];

		OpBase *prev_root = prev_segment->root;
		OpBase **taps = array_new(OpBase *, 1);
		ExecutionPlan_Taps(current_segment->root, &taps);
		bool has_taps = (array_len(taps) > 0);
		array_free(taps);

		// Does current execution plan contains taps?
		if(has_taps) {
			OpBase *op_cp = NewCartesianProductOp(current_segment);
			ExecutionPlan_PushBelow(taps[0], op_cp);
			ExecutionPlan_AddOp(op_cp, prev_root);
		} else {
			OpBase *leaf = ExecutionPlan_LocateLeaf(current_segment->root);
			ExecutionPlan_AddOp(leaf, prev_root);
		}
	}

	ExecutionPlan *plan = segments[segment_count - 1];

	// Optimize the operations in the ExecutionPlan.
	optimizePlan(gc, plan);

	// Prepare column names for the ResultSet if this query contains data in addition to statistics.
	// if(result_set) ResultSet_BuildColumns(result_set, plan->projections);

	return plan;
}

rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	assert(plan && plan->record_map);
	return plan->record_map;
}

void _ExecutionPlan_Print(const OpBase *op, RedisModuleCtx *ctx, char *buffer, int buffer_len,
						  int ident, int *op_count) {
	if(!op) return;

	*op_count += 1; // account for current operation.

	// Construct operation string representation.
	int bytes_written = snprintf(buffer, buffer_len, "%*s", ident, "");
	bytes_written += OpBase_ToString(op, buffer + bytes_written, buffer_len - bytes_written);

	RedisModule_ReplyWithStringBuffer(ctx, buffer, bytes_written);

	// Recurse over child operations.
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_Print(op->children[i], ctx, buffer, buffer_len, ident + 4, op_count);
	}
}

// Reply with a string representation of given execution plan.
void ExecutionPlan_Print(const ExecutionPlan *plan, RedisModuleCtx *ctx) {
	assert(plan && ctx);

	int op_count = 0;   // Number of operations printed.
	char buffer[1024];

	// No idea how many operation are in execution plan.
	RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
	_ExecutionPlan_Print(plan->root, ctx, buffer, 1024, 0, &op_count);

	RedisModule_ReplySetArrayLength(ctx, op_count);
}

void _ExecutionPlanInit(OpBase *root) {
	// Initialize the operation if necesary.
	if(root->init) root->init(root);

	// Continue initializing downstream operations.
	for(int i = 0; i < root->childCount; i++) _ExecutionPlanInit(root->children[i]);
}

void ExecutionPlanInit(ExecutionPlan *plan) {
	if(!plan) return;
	for(int i = 0; i < plan->segment_count; i ++) _ExecutionPlanInit(plan->segments[i]->root);
}

ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan) {
	Record r;
	OpBase *op = plan->root;

	ExecutionPlanInit(plan);
	while((r = OpBase_Consume(op)) != NULL) Record_Free(r);
	return plan->result_set;
}

static void _ExecutionPlan_InitProfiling(OpBase *root) {
	root->profile = root->consume;
	root->consume = OpBase_Profile;
	root->stats = rm_malloc(sizeof(OpStats));
	root->stats->profileExecTime = 0;
	root->stats->profileRecordCount = 0;

	if(root->childCount) {
		for(int i = 0; i < root->childCount; i++) {
			OpBase *child = root->children[i];
			_ExecutionPlan_InitProfiling(child);
		}
	}
}

static void _ExecutionPlan_FinalizeProfiling(OpBase *root) {
	if(root->childCount) {
		for(int i = 0; i < root->childCount; i++) {
			OpBase *child = root->children[i];
			root->stats->profileExecTime -= child->stats->profileExecTime;
			_ExecutionPlan_FinalizeProfiling(child);
		}
	}
	root->stats->profileExecTime *= 1000;   // Milliseconds.
}

ResultSet *ExecutionPlan_Profile(ExecutionPlan *plan) {
	_ExecutionPlan_InitProfiling(plan->root);
	ResultSet *rs = ExecutionPlan_Execute(plan);
	_ExecutionPlan_FinalizeProfiling(plan->root);
	return rs;
}

void _ExecutionPlan_FreeOperations(OpBase *op) {
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_FreeOperations(op->children[i]);
	}
	OpBase_Free(op);
}

void ExecutionPlan_Free(ExecutionPlan *plan) {
	if(plan == NULL) return;

	for(uint i = 0; i < plan->segment_count; i++) {
		ExecutionPlan_Free(plan->segments[i]);
	}
	rm_free(plan->segments);

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) {
			QueryGraph_Free(plan->connected_components[i]);
		}
		array_free(plan->connected_components);
	}

	QueryGraph_Free(plan->query_graph);
	_ExecutionPlan_FreeOperations(plan->root);
	rm_free(plan);
}
