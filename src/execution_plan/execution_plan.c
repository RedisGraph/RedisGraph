/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/qsort.h"
#include "../util/strcmp.h"
#include "../util/vector.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"
#include "../graph/entities/edge.h"
#include "../ast/ast_build_ar_exp.h"
#include "./optimizations/optimizer.h"
#include "../ast/ast_build_op_contexts.h"
#include "../ast/ast_build_filter_tree.h"
#include "./optimizations/optimizations.h"
#include "../arithmetic/algebraic_expression.h"

#include <assert.h>
#include <setjmp.h>

//----reduce_to_apply.c----//
/* Reduces a filter operation into an apply operation. */
void ExecutionPlan_ReduceFilterToApply(ExecutionPlan *plan, OpFilter *filter);

static inline void _ExecutionPlan_UpdateRoot(ExecutionPlan *plan, OpBase *new_root) {
	if(plan->root) ExecutionPlan_NewRoot(plan->root, new_root);
	plan->root = new_root;
}

// Allocate a new ExecutionPlan segment.
inline ExecutionPlan *ExecutionPlan_NewEmptyExecutionPlan(void) {
	return rm_calloc(1, sizeof(ExecutionPlan));
}

// Given a WITH/RETURN * clause, generate the array of expressions to populate.
static AR_ExpNode **_PopulateProjectAll(const cypher_astnode_t *clause) {
	// Retrieve the relevant aliases from the AST.
	const char **aliases = AST_GetProjectAll(clause);
	uint count = array_len(aliases);

	AR_ExpNode **project_exps = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		// Build an expression for each alias.
		AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(aliases[i], NULL);
		exp->resolved_name = aliases[i];
		project_exps = array_append(project_exps, exp);
	}

	return project_exps;
}

// Handle ORDER entities
static AR_ExpNode **_BuildOrderExpressions(AR_ExpNode **projections,
										   const cypher_astnode_t *order_clause) {
	AST *ast = QueryCtx_GetAST();
	uint projection_count = array_len(projections);
	uint count = cypher_ast_order_by_nitems(order_clause);
	AR_ExpNode **order_exps = array_new(AR_ExpNode *, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);
		// Build a string representation of the ORDER identity.
		char *constructed_name = AR_EXP_BuildResolvedName(exp);
		// If the constructed name refers to a QueryGraph entity, use its canonical name.
		char *canonical_name = raxFind(ast->canonical_entity_names, (unsigned char *)constructed_name,
									   strlen(constructed_name));
		if(canonical_name == raxNotFound) {
			// Otherwise, introduce a new canonical name.
			canonical_name = constructed_name;
			raxInsert(ast->canonical_entity_names, (unsigned char *)constructed_name, strlen(constructed_name),
					  constructed_name, NULL);
		} else {
			rm_free(constructed_name);
		}

		exp->resolved_name = canonical_name;
		AST_AttachName(ast, item, exp->resolved_name);

		order_exps = array_append(order_exps, exp);
	}

	return order_exps;
}

// Handle RETURN entities
// (This function is not static because it is relied upon by unit tests)
AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause) {
	// Query is of type "RETURN *"
	if(cypher_ast_return_has_include_existing(ret_clause)) return _PopulateProjectAll(ret_clause);

	uint count = cypher_ast_return_nprojections(ret_clause);
	AR_ExpNode **return_expressions = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
		// The AST expression can be an identifier, function call, or constant
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this return entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);

		// Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
		// the entity.prop combination ("a.val"), or the function call ("MAX(a.val)")
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
		return_expressions = array_append(return_expressions, exp);
	}

	return return_expressions;
}

static AR_ExpNode **_BuildWithExpressions(const cypher_astnode_t *with_clause) {
	// Clause is of type "WITH *"
	if(cypher_ast_with_has_include_existing(with_clause)) return _PopulateProjectAll(with_clause);

	uint count = cypher_ast_with_nprojections(with_clause);
	AR_ExpNode **with_expressions = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);

		// Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
		// the entity.prop combination ("a.val"), or the function call ("MAX(a.val)").
		// The WITH clause requires that the resolved name be an alias or identifier.
		const char *identifier = NULL;
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			/// TODO should issue syntax failure in the latter 2 cases
			identifier = cypher_ast_identifier_get_name(alias_node);
		} else {
			// This expression did not have an alias, so it must be an identifier
			const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);
			assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
			identifier = cypher_ast_identifier_get_name(ast_exp);
		}

		exp->resolved_name = identifier;

		with_expressions = array_append(with_expressions, exp);
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

static void _ExecutionPlan_ProcessQueryGraph(ExecutionPlan *plan, QueryGraph *qg,
											 AST *ast, FT_FilterNode *ft) {
	GraphContext *gc = QueryCtx_GetGraphCtx();

	QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
	uint connectedComponentsCount = array_len(connectedComponents);
	plan->connected_components = connectedComponents;
	// If we have already constructed any ops, the plan's record map contains all variables bound at this time.
	rax *bound_vars = plan->record_map;

	/* If we have multiple graph components, the root operation is a Cartesian Product.
	 * Each chain of traversals will be a child of this op. */
	OpBase *cartesianProduct = NULL;
	if(connectedComponentsCount > 1) {
		cartesianProduct = NewCartesianProductOp(plan);
		_ExecutionPlan_UpdateRoot(plan, cartesianProduct);
	}

	// Keep track after all traversal operations along a pattern.
	for(uint i = 0; i < connectedComponentsCount; i++) {
		QueryGraph *cc = connectedComponents[i];
		uint edge_count = array_len(cc->edges);
		OpBase *root = NULL; // The root of the traversal chain will be added to the ExecutionPlan.
		OpBase *tail = NULL;

		if(edge_count == 0) {
			/* If there are no edges in the component, we only need a node scan. */
			QGNode *n = cc->nodes[0];
			if(n->labelID != GRAPH_NO_LABEL) root = NewNodeByLabelScanOp(plan, n);
			else root = NewAllNodeScanOp(plan, n);
		} else {
			/* The component has edges, so we'll build a node scan and a chain of traversals. */
			AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc);
			uint expCount = array_len(exps);

			// Reorder exps, to the most performant arrangement of evaluation.
			orderExpressions(qg, exps, expCount, ft, bound_vars);

			/* Create the SCAN operation that will be the tail of the traversal chain. */
			QGNode *src = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Source(exps[0]));
			if(src->label) {
				/* Resolve source node by performing label scan,
				 * in which case if the first algebraic expression operand
				 * is a label matrix (diagonal) remove it. */
				if(AlgebraicExpression_DiagonalOperand(exps[0], 0)) {
					AlgebraicExpression_Free(AlgebraicExpression_RemoveLeftmostNode(&exps[0]));
				}
				root = tail = NewNodeByLabelScanOp(plan, src);
			} else {
				root = tail = NewAllNodeScanOp(plan, src);
			}

			/* For each expression, build the appropriate traversal operation. */
			for(int j = 0; j < expCount; j++) {
				AlgebraicExpression *exp = exps[j];
				// Empty expression, already freed.
				if(AlgebraicExpression_OperandCount(exp) == 0) continue;

				QGEdge *edge = NULL;
				if(AlgebraicExpression_Edge(exp)) edge = QueryGraph_GetEdgeByAlias(qg,
																					   AlgebraicExpression_Edge(exp));
				if(edge && QGEdge_VariableLength(edge)) {
					root = NewCondVarLenTraverseOp(plan, gc->g, exp);
				} else {
					root = NewCondTraverseOp(plan, gc->g, exp);
				}
				// Insert the new traversal op at the root of the chain.
				ExecutionPlan_AddOp(root, tail);
				tail = root;
			}

			// Free the expressions array, as its parts have been converted into operations
			array_free(exps);
		}

		if(cartesianProduct) {
			// We have multiple disjoint traversal chains.
			// Add each chain as a child under the Cartesian Product.
			ExecutionPlan_AddOp(cartesianProduct, root);
		} else {
			// We've built the only necessary traversal chain, update the ExecutionPlan root.
			_ExecutionPlan_UpdateRoot(plan, root);
		}
	}
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
	assert(filter->type == OPType_FILTER);
	rax *references = FilterTree_CollectModified(((OpFilter *)filter)->filterTree);
	OpBase *op;
	if(raxSize(references) > 0) {
		/* Scan execution plan, locate the earliest position where all
		 * references been resolved. */
		op = ExecutionPlan_LocateReferences(lower_bound, upper_bound, references);
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
	assert(op);
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

void ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan, const OpBase *recurse_limit) {
	Vector *sub_trees = FilterTree_SubTrees(plan->filter_tree);

	/* For each filter tree find the earliest position along the execution
	 * after which the filter tree can be applied. */
	for(int i = 0; i < Vector_Size(sub_trees); i++) {
		FT_FilterNode *tree;
		Vector_Get(sub_trees, i, &tree);
		OpBase *filter_op = NewFilterOp(plan, tree);
		ExecutionPlan_RePositionFilterOp(plan, plan->root, recurse_limit, filter_op);
	}
	Vector_Free(sub_trees);
	_ExecutionPlan_PlaceApplyOps(plan);
}

// Merge all order expressions into the projections array without duplicates,
static void _combine_projection_arrays(AR_ExpNode ***exps_ptr, AR_ExpNode **order_exps) {
	rax *projection_names = raxNew();
	AR_ExpNode **project_exps = *exps_ptr;
	uint order_count = array_len(order_exps);
	uint project_count = array_len(project_exps);

	// Add all WITH/RETURN projection names to rax.
	for(uint i = 0; i < project_count; i ++) {
		const char *name = project_exps[i]->resolved_name;
		raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
	}

	// Merge non-duplicate order expressions into projection array.
	for(uint i = 0; i < order_count; i ++) {
		const char *name = order_exps[i]->resolved_name;
		int new_name = raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
		// If it is a new projection, add a clone to the array.
		if(new_name) project_exps = array_append(project_exps, AR_EXP_Clone(order_exps[i]));
	}

	raxFree(projection_names);
	*exps_ptr = project_exps;
}

// Build an aggregate or project operation and any required modifying operations.
// This logic applies for both WITH and RETURN projections.
static inline void _buildProjectionOps(ExecutionPlan *plan, AR_ExpNode **projections,
									   AR_ExpNode **order_exps, int *sort_directions, bool aggregate, bool distinct) {

	// Merge order expressions into the projections array.
	if(order_exps) _combine_projection_arrays(&projections, order_exps);

	// Our fundamental operation will be a projection or aggregation.
	OpBase *op;
	if(aggregate) {
		// An aggregate op's caching policy depends on whether its results will be sorted.
		bool sorting_after_aggregation = (order_exps != NULL);
		op = NewAggregateOp(plan, projections, sorting_after_aggregation);
	} else {
		op = NewProjectOp(plan, projections);
	}
	_ExecutionPlan_UpdateRoot(plan, op);

	/* Add modifier operations in order such that the final execution plan will follow the sequence:
	 * Limit -> Skip -> Sort -> Distinct -> Project/Aggregate */
	if(distinct) {
		OpBase *op = NewDistinctOp(plan);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	AST *ast = QueryCtx_GetAST();

	if(sort_directions) {
		// The sort operation will obey a specified limit, but must account for skipped records
		OpBase *op = NewSortOp(plan, order_exps, sort_directions);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	if(AST_GetSkipExpr(ast)) {
		OpBase *op = NewSkipOp(plan);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	if(AST_GetLimitExpr(ast)) {
		OpBase *op = NewLimitOp(plan);
		_ExecutionPlan_UpdateRoot(plan, op);
	}
}

// The RETURN logic is identical to WITH-culminating segments,
// though a different set of API endpoints must be used for each.
static void _buildReturnOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildReturnExpressions(clause);

	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_return_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(clause);

	int *sort_directions = NULL;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(clause);
	if(order_clause) {
		AST_PrepareSortOp(order_clause, &sort_directions);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, sort_directions, aggregate, distinct);
}

static void _buildWithOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildWithExpressions(clause);
	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_with_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(clause);

	int *sort_directions = NULL;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(clause);
	if(order_clause) {
		AST_PrepareSortOp(order_clause, &sort_directions);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, sort_directions, aggregate, distinct);
}

// Convert a CALL clause into a procedure call operation.
static inline void _buildCallOp(AST *ast, ExecutionPlan *plan,
								const cypher_astnode_t *call_clause) {
	// A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
	AR_ExpNode **arguments = _BuildCallArguments(call_clause);
	AR_ExpNode **yield_exps = _BuildCallProjections(call_clause, ast); // TODO only need strings
	OpBase *op = NewProcCallOp(plan, proc_name, arguments, yield_exps);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildCreateOp(GraphContext *gc, AST *ast, ExecutionPlan *plan) {
	AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(plan->query_graph, plan->record_map);
	OpBase *op = NewCreateOp(plan, create_ast_ctx.nodes_to_create, create_ast_ctx.edges_to_create);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUnwindOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(clause);
	OpBase *op = NewUnwindOp(plan, unwind_ast_ctx.exp);
	_ExecutionPlan_UpdateRoot(plan, op);
}


static void _buildMergeCreateStream(ExecutionPlan *plan, AST_MergeContext *merge_ctx,
									const char **arguments) {
	/* If we have bound variables, we must ensure that all of our created entities are unique. Consider:
	 * UNWIND [1, 1] AS x MERGE ({val: x})
	 * Exactly one node should be created in the UNWIND...MERGE query. */
	OpBase *merge_create = NewMergeCreateOp(plan, merge_ctx->nodes_to_merge, merge_ctx->edges_to_merge);
	ExecutionPlan_AddOp(plan->root, merge_create); // Add MergeCreate op to stream.

	// If we have bound variables, push an Argument tap beneath the Create op.
	if(arguments) {
		OpBase *create_argument = NewArgumentOp(plan, arguments);
		ExecutionPlan_AddOp(merge_create, create_argument); // Add Argument op to stream.
	}
}

static void _buildMergeOp(GraphContext *gc, AST *ast, ExecutionPlan *plan,
						  const cypher_astnode_t *clause) {
	/*
	 * A MERGE clause provides a single path that must exist or be created.
	 * If we have built ops already, they will form the first stream into the Merge op.
	 * A clone of the Record produced by this stream will be passed into the other Merge streams
	 * so that they properly work with bound variables.
	 *
	 * As with paths in a MATCH query, build the appropriate traversal operations
	 * and add them as another stream into Merge.
	 *
	 * Finally, we'll add a last stream that creates the pattern if it did not get matched.
	 *
	 * Simple case (2 streams, no bound variables):
	 * MERGE (:A {val: 5})
	 *                           Merge
	 *                          /     \
	 *                     Filter    Create
	 *                      /
	 *                Label Scan
	 *
	 * Complex case:
	 * MATCH (a:A) MERGE (a)-[:E]->(:B)
	 *                                  Merge
	 *                           /        |        \
	 *                    LabelScan CondTraverse  Create
	 *                                    |          \
	 *                                Argument     Argument
	 */

	// Collect the variables that are bound at this point, as MERGE shouldn't construct them.
	rax *bound_vars = NULL;
	const char **arguments = NULL;
	if(plan->root) {
		bound_vars = raxNew();
		// Rather than cloning the record map, collect the bound variables along with their
		// parser-generated constant strings.
		ExecutionPlan_BoundVariables(plan->root, bound_vars);
		// Collect the variable names from bound_vars to populate the Argument ops we will build.
		arguments = (const char **)raxValues(bound_vars);
	}

	// Convert all the AST data required to populate our operations tree.
	AST_MergeContext merge_ctx = AST_PrepareMergeOp(clause, gc, plan->query_graph, bound_vars);

	// Create a Merge operation. It will store no information at this time except for any graph updates
	// it should make due to ON MATCH and ON CREATE SET directives in the query.
	OpBase *merge_op = NewMergeOp(plan, merge_ctx.on_match, merge_ctx.on_create);

	// Set Merge op as new root and add previously-built ops, if any, as Merge's first stream.
	_ExecutionPlan_UpdateRoot(plan, merge_op);

	// Build the Match stream as a Merge child.
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, path);
	ExecutionPlan_AddOp(plan->root, match_stream); // Add Match stream to Merge op.

	// Build the Create stream as a Merge child.
	_buildMergeCreateStream(plan, &merge_ctx, arguments);

	if(bound_vars) raxFree(bound_vars);
	array_free(arguments);
}

static void _buildOptionalMatchOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	OpBase *optional = NewOptionalOp(plan);
	const char **arguments = NULL;
	// The root will be non-null unless the first clause is an OPTIONAL MATCH.
	if(plan->root) {
		// Collect the variables that are bound at this point.
		rax *bound_vars = raxNew();
		// Rather than cloning the record map, collect the bound variables along with their
		// parser-generated constant strings.
		ExecutionPlan_BoundVariables(plan->root, bound_vars);
		// Collect the variable names from bound_vars to populate the Argument op we will build.
		arguments = (const char **)raxValues(bound_vars);
		raxFree(bound_vars);

		// Create an Apply operator and make it the new root.
		OpBase *apply_op = NewApplyOp(plan);
		_ExecutionPlan_UpdateRoot(plan, apply_op);

		// Create an Optional op and add it as an Apply child as a right-hand stream.
		ExecutionPlan_AddOp(apply_op, optional);
	}

	// Build the new Match stream and add it to the Optional stream.
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, clause);
	ExecutionPlan_AddOp(optional, match_stream);

	// If no root has been set (OPTIONAL was the first clause), set it to the Optional op.
	if(!plan->root) _ExecutionPlan_UpdateRoot(plan, optional);

	array_free(arguments);
}

static inline void _buildUpdateOp(GraphContext *gc, ExecutionPlan *plan,
								  const cypher_astnode_t *clause) {
	EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(gc, clause);
	OpBase *op = NewUpdateOp(plan, update_exps);
	array_free(update_exps);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildDeleteOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **exps = AST_PrepareDeleteOp(clause);
	OpBase *op = NewDeleteOp(plan, exps);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static void _ExecutionPlanSegment_ConvertClause(GraphContext *gc, AST *ast, ExecutionPlan *plan,
												const cypher_astnode_t *clause) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	// Because 't' is set using the offsetof() call, it cannot be used in switch statements.
	if(t == CYPHER_AST_MATCH) {
		if(cypher_ast_match_is_optional(clause)) {
			_buildOptionalMatchOps(plan, clause);
			return;
		}
		// Only add at most one set of traversals per plan. TODO Revisit and improve this logic.
		if(plan->root && ExecutionPlan_LocateOpMatchingType(plan->root, SCAN_OPS, SCAN_OP_COUNT)) {
			return;
		}
		_ExecutionPlan_ProcessQueryGraph(plan, plan->query_graph, ast, plan->filter_tree);
	} else if(t == CYPHER_AST_CALL) {
		_buildCallOp(ast, plan, clause);
	} else if(t == CYPHER_AST_CREATE) {
		// Only add at most one Create op per plan. TODO Revisit and improve this logic.
		if(ExecutionPlan_LocateOp(plan->root, OPType_CREATE)) return;
		_buildCreateOp(gc, ast, plan);
	} else if(t == CYPHER_AST_UNWIND) {
		_buildUnwindOp(plan, clause);
	} else if(t == CYPHER_AST_MERGE) {
		_buildMergeOp(gc, ast, plan, clause);
	} else if(t == CYPHER_AST_SET) {
		_buildUpdateOp(gc, plan, clause);
	} else if(t == CYPHER_AST_DELETE) {
		_buildDeleteOp(plan, clause);
	} else if(t == CYPHER_AST_RETURN) {
		// Converting a RETURN clause can create multiple operations.
		_buildReturnOps(plan, clause);
	} else if(t == CYPHER_AST_WITH) {
		// Converting a WITH clause can create multiple operations.
		_buildWithOps(plan, clause);
	}
}

void ExecutionPlan_PopulateExecutionPlan(ExecutionPlan *plan) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Initialize the plan's record mapping if necessary.
	// It will already be set if this ExecutionPlan has been created to populate a single stream.
	if(plan->record_map == NULL) plan->record_map = raxNew();

	// Build query graph
	plan->query_graph = BuildQueryGraph(gc, ast);

	// Build filter tree
	plan->filter_tree = AST_BuildFilterTree(ast);

	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		// Build the appropriate operation(s) for each clause in the query.
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		_ExecutionPlanSegment_ConvertClause(gc, ast, plan, clause);
	}
}

static ExecutionPlan *_ExecutionPlan_UnionPlans(AST *ast) {
	uint end_offset = 0;
	uint start_offset = 0;
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	uint *union_indices = AST_GetClauseIndices(ast, CYPHER_AST_UNION);
	union_indices = array_append(union_indices, clause_count);
	int union_count = array_len(union_indices);
	assert(union_count > 1);

	/* Placeholder for each execution plan, these all will be joined
	 * via a single UNION operation. */
	ExecutionPlan **plans = array_new(ExecutionPlan *, union_count);

	for(int i = 0; i < union_count; i++) {
		// Create an AST segment from which we will build an execution plan.
		end_offset = union_indices[i];
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);
		plans = array_append(plans, NewExecutionPlan());
		AST_Free(ast_segment); // Free the AST segment.

		// Next segment starts where this one ends.
		start_offset = union_indices[i] + 1;
	}

	QueryCtx_SetAST(ast); // AST segments have been freed, set master AST in QueryCtx.

	array_free(union_indices);

	/* Join streams:
	 * MATCH (a) RETURN a UNION MATCH (a) RETURN a ....
	 * left stream:     [Scan]->[Project]->[Results]
	 * right stream:    [Scan]->[Project]->[Results]
	 *
	 * Joined:
	 * left stream:     [Scan]->[Project]
	 * right stream:    [Scan]->[Project]
	 *                  [Union]->[Distinct]->[Result] */

	ExecutionPlan *plan = rm_calloc(1, sizeof(ExecutionPlan));
	plan->root = NULL;
	plan->segments = plans;
	plan->query_graph = NULL;
	plan->record_map = raxNew();
	plan->connected_components = NULL;
	plan->is_union = true;

	OpBase *results_op = NewResultsOp(plan);
	OpBase *parent = results_op;
	_ExecutionPlan_UpdateRoot(plan, results_op);

	// Introduce distinct only if `ALL` isn't specified.
	const cypher_astnode_t *union_clause = AST_GetClause(ast, CYPHER_AST_UNION);
	if(!cypher_ast_union_has_all(union_clause)) {
		OpBase *distinct_op = NewDistinctOp(plan);
		ExecutionPlan_AddOp(results_op, distinct_op);
		parent = distinct_op;
	}

	OpBase *join_op = NewJoinOp(plan);
	ExecutionPlan_AddOp(parent, join_op);

	// Join execution plans.
	for(int i = 0; i < union_count; i++) {
		ExecutionPlan *sub_plan = plans[i];
		assert(sub_plan->root->type == OPType_RESULTS);

		// Remove OP_Result.
		OpBase *op_result = sub_plan->root;
		ExecutionPlan_RemoveOp(sub_plan, sub_plan->root);
		OpBase_Free(op_result);

		ExecutionPlan_AddOp(join_op, sub_plan->root);
	}

	return plan;
}

static OpBase *_ExecutionPlan_FindLastWriter(OpBase *root) {
	if(OpBase_IsWriter(root)) return root;
	for(int i = root->childCount - 1; i >= 0; i--) {
		OpBase *child = root->children[i];
		OpBase *res = _ExecutionPlan_FindLastWriter(child);
		if(res) return res;
	}
	return NULL;
}

ExecutionPlan *NewExecutionPlan(void) {
	AST *ast = QueryCtx_GetAST();
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	/* Handle UNION if there are any. */
	if(AST_ContainsClause(ast, CYPHER_AST_UNION)) return _ExecutionPlan_UnionPlans(ast);

	uint start_offset = 0;
	uint end_offset = 0;

	/* Execution plans are created in 1 or more segments. Every WITH clause demarcates
	 * the beginning of a new segment, and a RETURN clause (if present) forms its own segment. */
	const cypher_astnode_t *last_clause = cypher_ast_query_get_clause(ast->root, clause_count - 1);
	cypher_astnode_type_t last_clause_type = cypher_astnode_type(last_clause);
	bool query_has_return = (last_clause_type == CYPHER_AST_RETURN);

	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);

	// If the first clause of the query is WITH, remove its index from the segment list.
	if(array_len(segment_indices) > 0 && segment_indices[0] == 0) {
		segment_indices = array_del(segment_indices, 0);
	}

	/* The RETURN clause is converted into an independent final segment.
	 * If the query is exclusively composed of a RETURN clause, only one segment is constructed
	 * so this step is skipped. */
	if(query_has_return && clause_count > 1) {
		segment_indices = array_append(segment_indices, clause_count - 1);
	}

	// Add the clause count as a final value so that the last segment will read to the end of the query.
	segment_indices = array_append(segment_indices, clause_count);

	uint segment_count = array_len(segment_indices);
	ExecutionPlan **segments = array_new(ExecutionPlan *, segment_count);
	AST *ast_segments[segment_count];
	start_offset = 0;
	for(int i = 0; i < segment_count; i++) {
		uint end_offset = segment_indices[i];
		// Slice the AST to only include the clauses in the current segment.
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);
		ast_segments[i] = ast_segment;
		// Construct a new ExecutionPlanSegment.
		ExecutionPlan *segment = ExecutionPlan_NewEmptyExecutionPlan();
		ExecutionPlan_PopulateExecutionPlan(segment);
		segment->ast_segment = ast_segment;
		segments = array_append(segments, segment);
		start_offset = end_offset;
	}

	// Place filter ops required by first ExecutionPlan segment.
	QueryCtx_SetAST(ast_segments[0]);
	if(segments[0]->filter_tree) ExecutionPlan_PlaceFilterOps(segments[0], NULL);

	OpBase *connecting_op = NULL;
	OpBase *prev_scope_end = NULL;
	// Merge segments.
	for(int i = 1; i < segment_count; i++) {
		ExecutionPlan *prev_segment = segments[i - 1];
		ExecutionPlan *current_segment = segments[i];

		OpBase *prev_root = prev_segment->root;
		connecting_op = ExecutionPlan_LocateOpMatchingType(current_segment->root, PROJECT_OPS,
														   PROJECT_OP_COUNT);
		assert(connecting_op->childCount == 0);

		ExecutionPlan_AddOp(connecting_op, prev_root);

		// Place filter ops required by current segment.
		QueryCtx_SetAST(ast_segments[i]);
		if(current_segment->filter_tree) {
			ExecutionPlan_PlaceFilterOps(current_segment, prev_scope_end);
			current_segment->filter_tree = NULL;
		}

		prev_scope_end = prev_root; // Track the previous scope's end so filter placement doesn't overreach.
	}

	QueryCtx_SetAST(ast); // AST segments have been freed, set master AST in QueryCtx.

	array_free(segment_indices);

	ExecutionPlan *plan = array_pop(segments);
	// The root operation is OpResults only if the query culminates in a RETURN or CALL clause.
	if(query_has_return || last_clause_type == CYPHER_AST_CALL) {
		OpBase *results_op = NewResultsOp(plan);
		_ExecutionPlan_UpdateRoot(plan, results_op);
	}

	plan->segments = segments;

	return plan;
}

// Sets an AST segment in the execution plan.
inline void ExecutionPlan_SetAST(ExecutionPlan *plan, AST *ast) {
	plan->ast_segment = ast;
}

// Gets the AST segment from the execution plan.
inline AST *ExecutionPlan_GetAST(const ExecutionPlan *plan) {
	return plan->ast_segment;
}

void ExecutionPlan_PreparePlan(ExecutionPlan *plan) {
	// Plan should be prepared only once.
	assert(!plan->prepared);
	optimizePlan(plan);
	QueryCtx_SetLastWriter(_ExecutionPlan_FindLastWriter(plan->root));
	plan->prepared = true;
}

inline rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	assert(plan && plan->record_map);
	return plan->record_map;
}

Record ExecutionPlan_BorrowRecord(ExecutionPlan *plan) {
	rax *mapping = ExecutionPlan_GetMappings(plan);
	assert(plan->record_pool);

	// Get a Record from the pool and set its owner and mapping.
	Record r = ObjectPool_NewItem(plan->record_pool);
	r->owner = plan;
	r->mapping = plan->record_map;
	return r;
}

void ExecutionPlan_ReturnRecord(ExecutionPlan *plan, Record r) {
	assert(plan && r);
	ObjectPool_DeleteItem(plan->record_pool, r);
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

static inline void _ExecutionPlan_InitRecordPool(ExecutionPlan *plan) {
	if(plan->record_pool) return;
	/* Initialize record pool.
	 * Determine Record size to inform ObjectPool allocation. */
	uint entries_count = raxSize(plan->record_map);
	uint rec_size = sizeof(_Record) + (sizeof(Entry) * entries_count);

	// Create a data block with initial capacity of 256 records.
	plan->record_pool = ObjectPool_New(256, rec_size, (fpDestructor)Record_FreeEntries);
}

static void _ExecutionPlanInit(OpBase *root) {
	// If the ExecutionPlan associated with this op hasn't built a record pool yet, do so now.
	_ExecutionPlan_InitRecordPool((ExecutionPlan *)root->plan);

	// Initialize the operation if necessary.
	if(root->init) root->init(root);

	// Continue initializing downstream operations.
	for(int i = 0; i < root->childCount; i++) {
		_ExecutionPlanInit(root->children[i]);
	}
}

void ExecutionPlan_Init(ExecutionPlan *plan) {
	_ExecutionPlanInit(plan->root);
}

ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan) {
	assert(plan->prepared);
	/* Set an exception-handling breakpoint to capture run-time errors.
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	int encountered_error = SET_EXCEPTION_HANDLER();

	// Encountered a run-time error - return immediately.
	if(encountered_error) return QueryCtx_GetResultSet();

	ExecutionPlan_Init(plan);

	Record r = NULL;
	// Execute the root operation and free the processed Record until the data stream is depleted.
	while((r = OpBase_Consume(plan->root)) != NULL) ExecutionPlan_ReturnRecord(r->owner, r);

	return QueryCtx_GetResultSet();
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

static void _ExecutionPlan_FreeOperations(OpBase *op) {
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_FreeOperations(op->children[i]);
	}
	OpBase_Free(op);
}

static void _ExecutionPlan_FreeSubPlan(ExecutionPlan *plan) {
	if(plan == NULL) return;

	if(plan->segments) {
		uint segment_count = array_len(plan->segments);
		for(int i = 0; i < segment_count; i++) _ExecutionPlan_FreeSubPlan(plan->segments[i]);
		array_free(plan->segments);
	}

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
		plan->connected_components = NULL;
	}

	QueryGraph_Free(plan->query_graph);
	if(plan->record_map) raxFree(plan->record_map);
	if(plan->record_pool) ObjectPool_Free(plan->record_pool);
	if(plan->ast_segment) AST_Free(plan->ast_segment);
	rm_free(plan);
}

void ExecutionPlan_Free(ExecutionPlan *plan) {
	if(plan == NULL) return;

	if(plan->root) {
		_ExecutionPlan_FreeOperations(plan->root);
		plan->root = NULL;
	}

	/* All segments but the last should have everything but
	 * their operation chain freed.
	 * The last segment is the actual plan passed as an argument to this function.
	 * TODO this logic isn't ideal, try to improve. */
	if(plan->segments) {
		uint segment_count = array_len(plan->segments);
		for(int i = 0; i < segment_count; i++) _ExecutionPlan_FreeSubPlan(plan->segments[i]);
		array_free(plan->segments);
	}

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
		plan->connected_components = NULL;
	}

	QueryGraph_Free(plan->query_graph);
	if(plan->record_map) raxFree(plan->record_map);
	if(plan->record_pool) ObjectPool_Free(plan->record_pool);
	if(plan->ast_segment) AST_Free(plan->ast_segment);
	rm_free(plan);
}

