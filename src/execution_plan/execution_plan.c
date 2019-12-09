/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "execution_plan.h"
#include "./ops/ops.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/qsort.h"
#include "../util/vector.h"
#include "../util/rmalloc.h"
#include "../ast/ast_mock.h"
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

// Forward declaration
static void _PopulateExecutionPlan(ExecutionPlan *plan, ResultSet *result_set);

static inline void _ExecutionPlan_UpdateRoot(ExecutionPlan *plan, OpBase *new_root) {
	if(plan->root) ExecutionPlan_NewRoot(plan->root, new_root);
	plan->root = new_root;
}

// For all ops that refer to QG entities, rebind them with the matching entity
// in the provided QueryGraph.
// (This logic is ugly, but currently necessary.)
static void _RebindQueryGraphReferences(OpBase *op, const QueryGraph *qg) {
	switch(op->type) {
	case OPType_INDEX_SCAN:
		((IndexScan *)op)->n = QueryGraph_GetNodeByAlias(qg, ((IndexScan *)op)->n->alias);
		return;
	case OPType_ALL_NODE_SCAN:
		((AllNodeScan *)op)->n = QueryGraph_GetNodeByAlias(qg, ((AllNodeScan *)op)->n->alias);
		return;
	case OPType_NODE_BY_LABEL_SCAN:
		((NodeByLabelScan *)op)->n = QueryGraph_GetNodeByAlias(qg, ((NodeByLabelScan *)op)->n->alias);
		return;
	case OPType_NODE_BY_ID_SEEK:
		((NodeByIdSeek *)op)->n = QueryGraph_GetNodeByAlias(qg, ((NodeByIdSeek *)op)->n->alias);
		return;
	default:
		return;
	}
}

// For all ops in the given tree, assocate the provided ExecutionPlan.
// This is for use for updating ops that have been built with a temporary ExecutionPlan.
static void _BindPlanToOps(OpBase *root, ExecutionPlan *plan) {
	if(!root) return;
	root->plan = plan;
	_RebindQueryGraphReferences(root, plan->query_graph);
	for(int i = 0; i < root->childCount; i ++) {
		_BindPlanToOps(root->children[i], plan);
	}
}


// Allocate a new ExecutionPlan segment.
static inline ExecutionPlan *_NewEmptyExecutionPlan(void) {
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
		if(edge_count == 0) {
			/* If there are no edges in the component, we only need a node scan. */
			QGNode *n = cc->nodes[0];
			if(n->labelID != GRAPH_NO_LABEL) root = NewNodeByLabelScanOp(plan, n);
			else root = NewAllNodeScanOp(plan, gc->g, n);
		} else {
			/* The component has edges, so we'll build a node scan and a chain of traversals. */
			uint expCount = 0;
			AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc, &expCount);

			// Reorder exps, to the most performant arrangement of evaluation.
			orderExpressions(qg, exps, expCount, ft, bound_vars);

			AlgebraicExpression *exp = exps[0];

			OpBase *tail = NULL;
			/* Create the SCAN operation that will be the tail of the traversal chain. */
			QGNode *src = QueryGraph_GetNodeByAlias(qg, exp->src);
			if(src->label) {
				/* Resolve source node by performing label scan,
				 * in which case if the first algebraic expression operand
				 * is a label matrix (diagonal) remove it, otherwise
				 * the label matrix associated with source's label is located
				 * within another traversal operation, for the timebeing do not
				 * try to locate and remove it, there's no real harm except some performace hit
				 * in keeping that label matrix. */
				if(exp->operands[0].diagonal) AlgebraicExpression_RemoveTerm(exp, 0, NULL);
				tail = NewNodeByLabelScanOp(plan, src);
			} else {
				tail = NewAllNodeScanOp(plan, gc->g, src);
			}

			/* For each expression, build the appropriate traversal operation. */
			for(int j = 0; j < expCount; j++) {
				exp = exps[j];
				if(exp->operand_count == 0) {
					AlgebraicExpression_Free(exp);
					continue;
				}

				QGEdge *edge = NULL;
				if(exp->edge) edge = QueryGraph_GetEdgeByAlias(qg, exp->edge);
				if(edge && QGEdge_VariableLength(edge)) {
					root = NewCondVarLenTraverseOp(plan, gc->g, exp);
				} else {
					root = NewCondTraverseOp(plan, gc->g, exp, TraverseRecordCap(ast));
				}
				// Insert the new traversal op at the root of the chain.
				ExecutionPlan_AddOp(root, tail);
				tail = root;
			}

			// Free the expressions array, as its parts have been converted into operations
			rm_free(exps);
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

static void _ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan) {
	Vector *sub_trees = FilterTree_SubTrees(plan->filter_tree);

	/* For each filter tree find the earliest position along the execution
	 * after which the filter tree can be applied. */
	for(int i = 0; i < Vector_Size(sub_trees); i++) {
		OpBase *op;
		FT_FilterNode *tree;
		Vector_Get(sub_trees, i, &tree);
		rax *references = FilterTree_CollectModified(tree);

		if(raxSize(references) > 0) {
			/* Scan execution plan, locate the earliest position where all
			 * references been resolved. */
			op = ExecutionPlan_LocateReferences(plan->root, references);
			assert(op);
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

		/* Create filter node.
		 * Introduce filter op right below located op. */
		OpBase *filter_op = NewFilterOp(plan, tree);
		ExecutionPlan_PushBelow(op, filter_op);
		if(op == plan->root) plan->root = filter_op;
		raxFree(references);
	}
	Vector_Free(sub_trees);
}

// Merge all order expressions into the projections array without duplicates,
// returning an array of expressions to free after building projection ops.
static AR_ExpNode **_combine_projection_arrays(AR_ExpNode ***exps_ptr, AR_ExpNode **order_exps) {
	rax *projection_names = raxNew();
	AR_ExpNode **project_exps = *exps_ptr;
	uint order_count = array_len(order_exps);
	uint project_count = array_len(project_exps);
	AR_ExpNode **free_list = array_new(AR_ExpNode *, order_count);

	// Add all WITH/RETURN projection names to rax.
	for(uint i = 0; i < project_count; i ++) {
		const char *name = project_exps[i]->resolved_name;
		raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
	}

	// Merge non-duplicate order expressions into projection array, add duplicate expressions to free list.
	for(uint i = 0; i < order_count; i ++) {
		const char *name = order_exps[i]->resolved_name;
		int new_name = raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
		if(new_name) {
			// New projection, add to array.
			project_exps = array_append(project_exps, order_exps[i]);
		} else {
			// Duplicate projection, add to free list.
			free_list = array_append(free_list, order_exps[i]);
		}
	}

	raxFree(projection_names);

	*exps_ptr = project_exps;
	return free_list;
}

// Build an aggregate or project operation and any required modifying operations.
// This logic applies for both WITH and RETURN projections.
static inline void _buildProjectionOps(ExecutionPlan *plan, AR_ExpNode **projections,
									   AR_ExpNode **order_exps, uint skip, uint sort, bool aggregate, bool distinct) {

	AR_ExpNode **free_list = NULL;
	// Merge order expressions into the projections array.
	if(order_exps) free_list = _combine_projection_arrays(&projections, order_exps);

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
	uint limit = ast->limit;

	if(sort) {
		// The sort operation will obey a specified limit, but must account for skipped records
		uint sort_limit = (limit != UNLIMITED) ? limit + skip : 0;
		OpBase *op = NewSortOp(plan, order_exps, sort, sort_limit);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	if(skip) {
		OpBase *op = NewSkipOp(plan, skip);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	if(limit != UNLIMITED) {
		OpBase *op = NewLimitOp(plan, limit);
		_ExecutionPlan_UpdateRoot(plan, op);
	}

	// Free any order expressions that have not been migrated into the projections array.
	if(free_list) {
		uint free_count = array_len(free_list);
		for(uint i = 0; i < free_count; i ++) {
			AR_EXP_Free(free_list[i]);
		}
		array_free(free_list);
	}
	if(order_exps) array_free(order_exps);
}

// The RETURN logic is identical to WITH-culminating segments,
// though a different set of API endpoints must be used for each.
static void _buildReturnOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildReturnExpressions(clause);

	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_return_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(clause);

	uint skip = 0;
	if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);

	int sort_direction = 0;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(clause);
	if(order_clause) {
		sort_direction = AST_PrepareSortOp(order_clause);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, skip, sort_direction, aggregate, distinct);
}

static void _buildWithOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildWithExpressions(clause);
	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_with_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(clause);

	uint skip = 0;
	uint limit = RESULTSET_UNLIMITED;
	if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);

	int sort_direction = 0;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(clause);
	if(order_clause) {
		sort_direction = AST_PrepareSortOp(order_clause);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, skip, sort_direction, aggregate, distinct);
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

static inline void _buildCreateOp(GraphContext *gc, AST *ast, ExecutionPlan *plan,
								  ResultSetStatistics *stats) {
	AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(plan->query_graph, plan->record_map);
	OpBase *op = NewCreateOp(plan, stats, create_ast_ctx.nodes_to_create,
							 create_ast_ctx.edges_to_create);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildUnwindOp(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(clause);
	OpBase *op = NewUnwindOp(plan, unwind_ast_ctx.exp);
	_ExecutionPlan_UpdateRoot(plan, op);
}

/* Given a MERGE clause, build the stream of operations required to match its pattern
 * and add it as a child of the Merge op. */
static void _buildMergeMatchStream(ExecutionPlan *plan, const cypher_astnode_t *clause,
								   const char **arguments) {
	AST *ast = QueryCtx_GetAST();
	// Initialize an ExecutionPlan that shares this plan's Record mapping.
	ExecutionPlan *rhs_plan = _NewEmptyExecutionPlan();
	rhs_plan->record_map = plan->record_map;

	// If we have bound variables, build an Argument op that represents them.
	if(arguments) rhs_plan->root = NewArgumentOp(plan, arguments);

	// Build a temporary AST holding the MERGE path within a MATCH clause.
	const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
	AST *rhs_ast = AST_MockMatchPattern(ast, path);

	_PopulateExecutionPlan(rhs_plan, NULL);

	AST_MockFree(rhs_ast);
	QueryCtx_SetAST(ast); // Reset the AST.

	ExecutionPlan_AddOp(plan->root, rhs_plan->root); // Add Match stream to Merge op.

	OpBase *rhs_root = rhs_plan->root;
	// Associate all new ops with the correct ExecutionPlan.
	_BindPlanToOps(rhs_root, plan);

	// NULL-set variables shared between the rhs_plan and the overall plan.
	rhs_plan->root = NULL;
	rhs_plan->record_map = NULL;
	// Free the temporary plan.
	ExecutionPlan_Free(rhs_plan);

}

static void _buildMergeCreateStream(ExecutionPlan *plan, AST_MergeContext *merge_ctx,
									ResultSetStatistics *stats, const char **arguments) {
	OpBase *tail = plan->root; // Where to append ops generated by this routine.

	if(merge_ctx->on_create) {
		// We have an ON CREATE directive, convert it into an Update op.
		OpBase *on_create_set_op = NewUpdateOp(plan, merge_ctx->on_create, stats);
		ExecutionPlan_AddOp(tail, on_create_set_op); // Add Update op to stream.
		tail = on_create_set_op;
	}

	/* If we have bound variables, we must ensure that all of our created entities are unique. Consider:
	 * UNWIND [1, 1] AS x MERGE ({val: x})
	 * Exactly one node should be created in the UNWIND...MERGE query. */
	OpBase *merge_create = NewMergeCreateOp(plan, stats, merge_ctx->nodes_to_merge,
											merge_ctx->edges_to_merge);
	ExecutionPlan_AddOp(tail, merge_create); // Add MergeCreate op to stream.

	// If we have bound variables, push an Argument tap beneath the Create op.
	if(arguments) {
		OpBase *create_argument = NewArgumentOp(plan, arguments);
		ExecutionPlan_AddOp(merge_create, create_argument); // Add Argument op to stream.
	}
}

// Build an array of const strings to populate the 'modifies' arrays of Argument ops.
static inline const char **_buildArgumentModifiesArray(rax *bound_vars) {
	const char **arguments = array_new(const char *, raxSize(bound_vars));
	raxIterator it;
	raxStart(&it, bound_vars);
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) { // For each bound variable
		// Copy the const string variable name into the array.
		arguments = array_append(arguments, it.data);
	}
	raxStop(&it);

	return arguments;
}

static void _buildMergeOp(GraphContext *gc, AST *ast, ExecutionPlan *plan,
						  const cypher_astnode_t *clause, ResultSetStatistics *stats) {
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
		// Prepare the variables for populating the Argument ops we will build.
		arguments = _buildArgumentModifiesArray(bound_vars);
	}

	// Convert all the AST data required to populate our operations tree.
	AST_MergeContext merge_ctx = AST_PrepareMergeOp(clause, plan->query_graph, bound_vars);

	// Create a Merge operation. It will store no information at this time except for any graph updates
	// it should make due to ON MATCH SET directives in the query.
	OpBase *merge_op = NewMergeOp(plan, merge_ctx.on_match, stats);

	// Set Merge op as new root and add previously-built ops, if any, as Merge's first stream.
	_ExecutionPlan_UpdateRoot(plan, merge_op);

	// Build the Match stream as a Merge child.
	_buildMergeMatchStream(plan, clause, arguments);

	// Build the Create stream as a Merge child.
	_buildMergeCreateStream(plan, &merge_ctx, stats, arguments);

	if(bound_vars) raxFree(bound_vars);
	array_free(arguments);
}

static inline void _buildUpdateOp(ExecutionPlan *plan, const cypher_astnode_t *clause,
								  ResultSetStatistics *stats) {
	EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(clause);
	OpBase *op = NewUpdateOp(plan, update_exps, stats);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static inline void _buildDeleteOp(ExecutionPlan *plan, const cypher_astnode_t *clause,
								  ResultSetStatistics *stats) {
	AR_ExpNode **exps = AST_PrepareDeleteOp(clause);
	OpBase *op = NewDeleteOp(plan, exps, stats);
	_ExecutionPlan_UpdateRoot(plan, op);
}

static void _ExecutionPlanSegment_ConvertClause(GraphContext *gc, AST *ast,
												ExecutionPlan *plan, ResultSetStatistics *stats, const cypher_astnode_t *clause) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	// Because 't' is set using the offsetof() call, it cannot be used in switch statements.
	if(t == CYPHER_AST_MATCH) {
		// Only add at most one set of traversals per plan. TODO Revisit and improve this logic.
		if(ExecutionPlan_LocateOp(plan->root, OPType_NODE_BY_LABEL_SCAN) ||
		   ExecutionPlan_LocateOp(plan->root, OPType_ALL_NODE_SCAN)) {
			return;
		}
		_ExecutionPlan_ProcessQueryGraph(plan, plan->query_graph, ast, plan->filter_tree);
	} else if(t == CYPHER_AST_CALL) {
		_buildCallOp(ast, plan, clause);
	} else if(t == CYPHER_AST_CREATE) {
		// Only add at most one Create op per plan. TODO Revisit and improve this logic.
		if(ExecutionPlan_LocateOp(plan->root, OPType_CREATE)) return;
		_buildCreateOp(gc, ast, plan, stats);
	} else if(t == CYPHER_AST_UNWIND) {
		_buildUnwindOp(plan, clause);
	} else if(t == CYPHER_AST_MERGE) {
		_buildMergeOp(gc, ast, plan, clause, stats);
	} else if(t == CYPHER_AST_SET) {
		_buildUpdateOp(plan, clause, stats);
	} else if(t == CYPHER_AST_DELETE) {
		_buildDeleteOp(plan, clause, stats);
	} else if(t == CYPHER_AST_RETURN) {
		// Converting a RETURN clause can create multiple operations.
		_buildReturnOps(plan, clause);
	} else if(t == CYPHER_AST_WITH) {
		// Converting a WITH clause can create multiple operations.
		_buildWithOps(plan, clause);
	}
}

// Build a tree of operations that performs all the worked required by the clauses of the current AST.
static void _PopulateExecutionPlan(ExecutionPlan *plan, ResultSet *result_set) {
	AST *ast = QueryCtx_GetAST();
	GraphContext *gc = QueryCtx_GetGraphCtx();
	plan->result_set = result_set;

	// Initialize the plan's record mapping if necessary.
	// It will already be set if this ExecutionPlan has been created to populate a single stream.
	if(plan->record_map == NULL) plan->record_map = raxNew();

	// Build query graph
	plan->query_graph = BuildQueryGraph(gc, ast);

	// Build filter tree
	plan->filter_tree = AST_BuildFilterTree(ast);

	// If we are in a querying context, retrieve a pointer to the statistics for operations
	// like DELETE that only produce metadata.
	ResultSetStatistics *stats = (result_set) ? &result_set->stats : NULL;

	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		// Build the appropriate operation(s) for each clause in the query.
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		_ExecutionPlanSegment_ConvertClause(gc, ast, plan, stats, clause);
	}

	if(plan->filter_tree) _ExecutionPlan_PlaceFilterOps(plan);
}

ExecutionPlan *ExecutionPlan_UnionPlans(ResultSet *result_set, AST *ast) {
	uint end_offset = 0;
	uint start_offset = 0;
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	uint *union_indices = AST_GetClauseIndices(ast, CYPHER_AST_UNION);
	union_indices = array_append(union_indices, clause_count);
	int union_count = array_len(union_indices);
	assert(union_count > 1);

	/* Placeholder for each execution plan, these all will be joined
	 * via a single UNION operation. */
	ExecutionPlan **plans = rm_malloc(union_count * sizeof(ExecutionPlan *));

	for(int i = 0; i < union_count; i++) {
		// Create an AST segment from which we will build an execution plan.
		end_offset = union_indices[i];
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);
		plans[i] = NewExecutionPlan(result_set);
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
	plan->segment_count = union_count;
	plan->query_graph = NULL;
	plan->record_map = raxNew();
	plan->result_set = result_set;
	plan->connected_components = NULL;

	OpBase *results_op = NewResultsOp(plan, result_set);
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

		// Set plan's root to NULL, to avoid freeing its operations.
		sub_plan->root = NULL;
	}

	return plan;
}

ExecutionPlan *NewExecutionPlan(ResultSet *result_set) {
	AST *ast = QueryCtx_GetAST();
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	/* Handle UNION if there are any. */
	if(AST_ContainsClause(ast, CYPHER_AST_UNION)) {
		return ExecutionPlan_UnionPlans(result_set, ast);
	}

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
	ExecutionPlan **segments = rm_malloc(segment_count * sizeof(ExecutionPlan *));
	start_offset = 0;
	for(int i = 0; i < segment_count; i++) {
		uint end_offset = segment_indices[i];
		// Slice the AST to only include the clauses in the current segment.
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);

		// Construct a new ExecutionPlanSegment.
		ExecutionPlan *segment = _NewEmptyExecutionPlan();
		_PopulateExecutionPlan(segment, result_set);
		AST_Free(ast_segment); // Free the AST segment.

		segments[i] = segment;
		start_offset = end_offset;
	}

	QueryCtx_SetAST(ast); // AST segments have been freed, set master AST in QueryCtx.

	OpBase *connecting_op = NULL;
	// Merge segments.
	for(int i = 1; i < segment_count; i++) {
		ExecutionPlan *prev_segment = segments[i - 1];
		ExecutionPlan *current_segment = segments[i];

		OpBase *prev_root = prev_segment->root;
		connecting_op = ExecutionPlan_LocateOp(current_segment->root, OPType_PROJECT | OPType_AGGREGATE);
		assert(connecting_op->childCount == 0);

		ExecutionPlan_AddOp(connecting_op, prev_root);
	}

	array_free(segment_indices);

	ExecutionPlan *plan = segments[segment_count - 1];

	// Check to see if we've encountered an error while constructing the execution-plan.
	if(QueryCtx_EncounteredError()) {
		// A compile time error was encountered.
		ExecutionPlan_Free(plan);
		QueryCtx_EmitException();
		return NULL;
	}

	// The root operation is OpResults only if the query culminates in a RETURN or CALL clause.
	if(query_has_return) {
		if(!connecting_op) {
			// Set the connecting op if our query is just a RETURN.
			assert(segment_count == 1);
			connecting_op = ExecutionPlan_LocateOp(plan->root, OPType_PROJECT | OPType_AGGREGATE);
		}

		// Prepare column names for the ResultSet.
		if(result_set) result_set->columns = AST_BuildColumnNames(last_clause);

		OpBase *results_op = NewResultsOp(plan, result_set);
		_ExecutionPlan_UpdateRoot(plan, results_op);
	} else if(last_clause_type == CYPHER_AST_CALL) {
		assert(plan->root->type == OPType_PROC_CALL);
		OpProcCall *last_op = (OpProcCall *)plan->root;
		// Prepare column names for the ResultSet.
		if(result_set) array_clone(result_set->columns, last_op->output);

		OpBase *results_op = NewResultsOp(plan, result_set);
		_ExecutionPlan_UpdateRoot(plan, results_op);
	}


	// Optimize the operations in the ExecutionPlan.
	optimizePlan(plan);

	// Disregard self.
	plan->segment_count = segment_count - 1;
	plan->segments = segments;

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
	ExecutionPlan_Init(plan);

	/* Set an exception-handling breakpoint to capture run-time errors.
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	int encountered_error = SET_EXCEPTION_HANDLER();

	if(encountered_error) {
		// Encountered a run-time error; return immediately.
		return plan->result_set;
	}

	Record r = NULL;
	// Execute the root operation and free the processed Record until the data stream is depleted.
	while((r = OpBase_Consume(plan->root)) != NULL) Record_Free(r);

	// Return the result set.
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

static void _ExecutionPlan_FreeOperations(OpBase *op) {
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_FreeOperations(op->children[i]);
	}
	OpBase_Free(op);
}

static void _ExecutionPlan_FreeSubPlan(ExecutionPlan *plan) {
	if(plan == NULL) return;

	for(int i = 0; i < plan->segment_count; i++) _ExecutionPlan_FreeSubPlan(plan->segments[i]);
	if(plan->segments) rm_free(plan->segments);

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
		plan->connected_components = NULL;
	}

	QueryGraph_Free(plan->query_graph);
	raxFree(plan->record_map);
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
	for(int i = 0; i < plan->segment_count; i++) _ExecutionPlan_FreeSubPlan(plan->segments[i]);
	if(plan->segments) rm_free(plan->segments);

	if(plan->connected_components) {
		uint connected_component_count = array_len(plan->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) QueryGraph_Free(plan->connected_components[i]);
		array_free(plan->connected_components);
		plan->connected_components = NULL;
	}

	QueryGraph_Free(plan->query_graph);
	if(plan->record_map) raxFree(plan->record_map);
	rm_free(plan);
}

