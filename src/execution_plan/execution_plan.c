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
#include "../graph/entities/edge.h"
#include "../ast/ast_build_ar_exp.h"
#include "./optimizations/optimizer.h"
#include "../ast/ast_build_op_contexts.h"
#include "../ast/ast_build_filter_tree.h"
#include "./optimizations/optimizations.h"
#include "../arithmetic/algebraic_expression.h"

#include <assert.h>
#include <setjmp.h>

static inline OpBase *_ExecutionPlan_LocateLeaf(OpBase *root) {
	if(root->childCount == 0) return root;
	return _ExecutionPlan_LocateLeaf(root->children[0]);
}

/* In a query with a RETURN *, build projections for all explicit aliases
 * in previous clauses. */
static AR_ExpNode **_ReturnExpandAll(AST *ast) {
	// Collect all unique aliases
	const char **aliases = AST_CollectElementNames(ast);
	uint count = array_len(aliases);

	// Build an expression for each alias
	AR_ExpNode **return_expressions = array_new(AR_ExpNode *, count);
	for(int i = 0; i < count; i ++) {
		AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(aliases[i], NULL);
		exp->resolved_name = aliases[i];
		return_expressions = array_append(return_expressions, exp);
	}

	array_free(aliases);
	return return_expressions;
}

// Handle ORDER entities
static AR_ExpNode **_BuildOrderExpressions(AR_ExpNode **projections,
										   const cypher_astnode_t *order_clause) {
	uint projection_count = array_len(projections);
	uint count = cypher_ast_order_by_nitems(order_clause);
	AR_ExpNode **order_exps = array_new(AR_ExpNode *, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
		AR_ExpNode *exp = NULL;
		cypher_astnode_type_t type = cypher_astnode_type(ast_exp);
		if(type == CYPHER_AST_IDENTIFIER) {
			// Order expression is a reference to an alias in the query
			const char *alias = cypher_ast_identifier_get_name(ast_exp);
			for(uint j = 0; j < projection_count; j ++) {
				AR_ExpNode *projection = projections[j];
				if(!strcmp(projection->resolved_name, alias)) {
					// The projection must be cloned to avoid a double free
					exp = AR_EXP_Clone(projection);
					break;
				}
			}
			if(exp == NULL) {
				// We didn't match any previous projections. This can occur when we're
				// ordering by a projected WITH entity that is not also being returned.
				exp = AR_EXP_FromExpression(ast_exp);
				AR_EXP_BuildResolvedName(exp);
			}
			// } else if(type == CYPHER_AST_PROPERTY_OPERATOR) {
			// TODO can capture things like RETURN e.name ORDER by e.name here
			// const cypher_astnode_t *entity = cypher_ast_property_operator_get_expression(ast_exp);
			// if(cypher_astnode_type(entity) == CYPHER_AST_IDENTIFIER) {
			// // Order expression is a reference to an alias in the query
			// const char *alias = cypher_ast_identifier_get_name(ast_exp);
			// }
		} else {
			// Independent operator like:
			// ORDER BY COUNT(a)
			exp = AR_EXP_FromExpression(ast_exp);
			AR_EXP_BuildResolvedName(exp);
		}

		order_exps = array_append(order_exps, exp);
		// TODO direction should be specifiable per order entity
		// ascending = cypher_ast_sort_item_is_ascending(item);
	}

	return order_exps;
}

// Handle RETURN entities
// (This function is not static because it is relied upon by unit tests)
AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause, AST *ast) {
	// Query is of type "RETURN *",
	// collect all defined identifiers and create return elements for them
	if(cypher_ast_return_has_include_existing(ret_clause)) return _ReturnExpandAll(ast);

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
		// AR_ExpNode *arg = AR_EXP_FromExpression(record_map, ast_exp);
		// SIValue si_arg = AR_EXP_Evaluate(arg, NULL);
	}

	return arguments;
}

static inline void _ExecutionPlan_UpdateRoot(ExecutionPlan *plan, OpBase *new_root) {
	if(plan->root) ExecutionPlan_NewRoot(plan->root, new_root);
	plan->root = new_root;
}

static void _ExecutionPlan_ProcessQueryGraph(ExecutionPlan *plan, QueryGraph *qg, AST *ast,
											 FT_FilterNode *ft) {
	GraphContext *gc = QueryCtx_GetGraphCtx();

	QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
	uint connectedComponentsCount = array_len(connectedComponents);
	plan->connected_components = connectedComponents;

	/* If we have multiple graph components, we'll join each chain of traversals
	 * under a Cartesian Product root operation. */
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
			orderExpressions(exps, expCount, ft);

			AlgebraicExpression *exp = exps[0];
			selectEntryPoint(exp, ft);

			OpBase *tail = NULL;
			/* Create the SCAN operation that will be the tail of the traversal chain. */
			if(exp->src_node->label) {
				/* Resolve source node by performing label scan,
				 * in which case if the first algebraic expression operand
				 * is a label matrix (diagonal) remove it, otherwise
				 * the label matrix associated with source's label is located
				 * within another traversal operation, for the timebeing do not
				 * try to locate and remove it, there's no real harm except some performace hit
				 * in keeping that label matrix. */
				if(exp->operands[0].diagonal) AlgebraicExpression_RemoveTerm(exp, 0, NULL);
				tail = NewNodeByLabelScanOp(plan, exp->src_node);
			} else {
				tail = NewAllNodeScanOp(plan, gc->g, exp->src_node);
			}

			/* For each expression, build the appropriate traversal operation. */
			for(int j = 0; j < expCount; j++) {
				exp = exps[j];
				if(exp->operand_count == 0) continue;

				if(exp->edge && QGEdge_VariableLength(exp->edge)) {
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

		if(connectedComponentsCount > 1) {
			// Add this traversal chain as a child under the Cartesian Product.
			ExecutionPlan_AddOp(cartesianProduct, root);
		} else {
			// We've built the only necessary traversal chain; add it directly to the ExecutionPlan.
			if(plan->root) ExecutionPlan_AddOp(plan->root, root);
			else _ExecutionPlan_UpdateRoot(plan, root);
		}
	}
}

static ExecutionPlan *_NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, AST *ast,
										ResultSet *result_set) {
	// Allocate a new segment
	ExecutionPlan *plan = rm_calloc(1, sizeof(ExecutionPlan));
	plan->record_map = raxNew();
	plan->projection_map = raxNew(); // TODO not always a necessary alloc
	plan->result_set = result_set;
	plan->connected_components = NULL;

	/* Build projections from this AST's WITH, RETURN, and ORDER clauses. */
	// Retrieve a RETURN clause if one is specified in this AST's range
	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	// Retrieve a WITH clause if one is specified in this AST
	const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
	// We cannot have both a RETURN and WITH clause
	assert(!(ret_clause && with_clause));

	AR_ExpNode **projections = NULL;
	const cypher_astnode_t *order_clause = NULL;
	if(ret_clause) {
		projections = _BuildReturnExpressions(ret_clause, ast);
		order_clause = cypher_ast_return_get_order_by(ret_clause);
	} else if(with_clause) {
		projections = _BuildWithExpressions(with_clause);
		order_clause = cypher_ast_with_get_order_by(with_clause);
	}

	AR_ExpNode **order_expressions = NULL;
	if(order_clause) order_expressions = _BuildOrderExpressions(projections, order_clause);

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
			// yields = array_append(yields, yield_exps[i]->resolved_name);
			yields = array_append(yields, yield_exps[i]->operand.variadic.entity_alias);
		}
		array_free(yield_exps);
		OpBase *opProcCall = NewProcCallOp(plan, proc_name, arguments, yields);
		Vector_Push(ops, opProcCall);
	}

	// Build traversal operations for every connected component in the QueryGraph
	if(AST_ContainsClause(ast, CYPHER_AST_MATCH) || AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
		_ExecutionPlan_ProcessQueryGraph(plan, qg, ast, filter_tree);
	}
	raxFree(modifies_ids);

	return modifies;
}

// Build an aggregate or project operation and any required modifying operations.
// This logic applies for both WITH and RETURN projections.
static inline void _buildProjectionOps(ExecutionPlanSegment *segment, AR_ExpNode **order_exps,
									   uint skip, uint limit, uint sort, bool aggregate, bool distinct) {
	uint *modifies = _buildModifiesArray(segment->projections);

	// Our fundamental operation will be a projection or aggregation.
	OpBase *op;
	if(aggregate) {
		op = NewAggregateOp(segment->projections, modifies);
	} else {
		op = NewProjectOp(segment->projections, modifies);
	}
	_ExecutionPlan_UpdateRoot(segment, op);

	/* Add modifier operations in order such that the final execution plan will follow the sequence:
	 * Limit -> Skip -> Sort -> Distinct -> Project/Aggregate */
	if(distinct) {
		OpBase *op = NewDistinctOp();
		_ExecutionPlan_UpdateRoot(segment, op);
	}

	if(sort) {
		// The sort operation will obey a specified limit, but must account for skipped records
		uint sort_limit = (limit != RESULTSET_UNLIMITED) ? limit + skip : 0;
		OpBase *op = NewSortOp(order_exps, sort, sort_limit);
		_ExecutionPlan_UpdateRoot(segment, op);
	}

	if(skip) {
		OpBase *op = NewSkipOp(skip);
		_ExecutionPlan_UpdateRoot(segment, op);
	}

	if(limit != RESULTSET_UNLIMITED) {
		OpBase *op = NewLimitOp(limit);
		_ExecutionPlan_UpdateRoot(segment, op);
	}
}

// The RETURN logic is identical to WITH-culminating segments,
// though a different set of API endpoints must be used for each.
static inline void _buildReturnOps(ExecutionPlanSegment *segment, const cypher_astnode_t *clause) {
	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_return_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(clause);
	const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(clause);

	uint skip = 0;
	uint limit = RESULTSET_UNLIMITED;
	if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
	if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

	int sort_direction = 0;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(clause);
	if(order_clause) {
		sort_direction = AST_PrepareSortOp(order_clause);
		order_exps = _BuildOrderExpressions(segment->record_map, segment->projections, order_clause);
	}
	_buildProjectionOps(segment, order_exps, skip, limit, sort_direction, aggregate, distinct);
}

static inline void _buildWithOps(ExecutionPlanSegment *segment, const cypher_astnode_t *clause) {
	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_with_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(clause);
	const cypher_astnode_t *limit_clause = cypher_ast_with_get_limit(clause);

	uint skip = 0;
	uint limit = RESULTSET_UNLIMITED;
	if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
	if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

	int sort_direction = 0;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(clause);
	if(order_clause) {
		sort_direction = AST_PrepareSortOp(order_clause);
		order_exps = _BuildOrderExpressions(segment->record_map, segment->projections, order_clause);
	}
	_buildProjectionOps(segment, order_exps, skip, limit, sort_direction, aggregate, distinct);
}

// Convert a CALL clause into a procedure call oepration.
static inline void _buildCallOp(AST *ast, ExecutionPlanSegment *segment,
								const cypher_astnode_t *call_clause) {
	// A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
	const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
	const char **arguments = _BuildCallArguments(segment->record_map, call_clause);
	AR_ExpNode **yield_exps = _BuildCallProjections(segment->record_map, call_clause, ast);
	uint yield_count = array_len(yield_exps);
	const char **yields = array_new(const char *, yield_count);

	uint *call_modifies = array_new(uint, yield_count);
	for(uint i = 0; i < yield_count; i ++) {
		// Track the names of yielded variables.
		// yields = array_append(yields, yield_exps[i]->resolved_name);
		yields = array_append(yields, yield_exps[i]->operand.variadic.entity_alias);
		// Track which variables are modified by this operation.
		call_modifies = array_append(call_modifies, RecordMap_LookupAlias(segment->record_map,
																		  yield_exps[i]->resolved_name));
	}

	if(segment->projections == NULL) {
		segment->projections = array_new(AR_ExpNode *, yield_count);
		for(uint i = 0; i < yield_count; i ++) {
			// TODO revisit this logic, room for improvement
			// Add yielded expressions to segment projections.
			segment->projections = array_append(segment->projections, yield_exps[i]);
		}
	}

	array_free(yield_exps);

	OpBase *op =  NewProcCallOp(proc_name, arguments, yields, call_modifies);
	_ExecutionPlan_UpdateRoot(segment, op);
}

static inline void _buildCreateOp(GraphContext *gc, AST *ast, ExecutionPlanSegment *segment,
								  ResultSetStatistics *stats) {
	AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(gc, segment->record_map, ast,
														   segment->query_graph);
	OpBase *op = NewCreateOp(stats, create_ast_ctx.nodes_to_create, create_ast_ctx.edges_to_create);
	_ExecutionPlan_UpdateRoot(segment, op);

}

static inline void _buildUnwindOp(ExecutionPlanSegment *segment, const cypher_astnode_t *clause) {
	AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(clause, segment->record_map);
	OpBase *op = NewUnwindOp(unwind_ast_ctx.record_idx, unwind_ast_ctx.exp);
	_ExecutionPlan_UpdateRoot(segment, op);
}

static inline void _buildMergeOp(GraphContext *gc, AST *ast, ExecutionPlanSegment *segment,
								 const cypher_astnode_t *clause, ResultSetStatistics *stats) {
	// A merge clause provides a single path that must exist or be created.
	// As with paths in a MATCH query, build the appropriate traversal operations
	// and append them to the set of ops.
	AST_MergeContext merge_ast_ctx = AST_PrepareMergeOp(gc, segment->record_map, ast, clause,
														segment->query_graph);
	OpBase *op = NewMergeOp(stats, merge_ast_ctx.nodes_to_merge, merge_ast_ctx.edges_to_merge);
	_ExecutionPlan_UpdateRoot(segment, op);
}

static inline void _buildUpdateOp(GraphContext *gc, ExecutionPlanSegment *segment,
								  const cypher_astnode_t *clause, ResultSetStatistics *stats) {
	uint nitems;
	EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(clause, segment->record_map, &nitems);
	OpBase *op = NewUpdateOp(gc, update_exps, nitems, stats);
	_ExecutionPlan_UpdateRoot(segment, op);
}

static inline void _buildDeleteOp(ExecutionPlanSegment *segment, const cypher_astnode_t *clause,
								  ResultSetStatistics *stats) {
	uint *nodes_ref;
	uint *edges_ref;
	AST_PrepareDeleteOp(clause, segment->query_graph, segment->record_map, &nodes_ref, &edges_ref);
	OpBase *op = NewDeleteOp(nodes_ref, edges_ref, stats);
	_ExecutionPlan_UpdateRoot(segment, op);
}

static void _ExecutionPlanSegment_ConvertClause(GraphContext *gc, AST *ast,
												ExecutionPlanSegment *segment, ResultSetStatistics *stats, const cypher_astnode_t *clause) {
	cypher_astnode_type_t t = cypher_astnode_type(clause);
	// Because 't' is set using the offsetof() call, it cannot be used in switch statements.
	if(t == CYPHER_AST_MATCH || t == CYPHER_AST_CALL) {
		// MATCH and CALL clauses are processed before looping over all clauses.
		// TODO This should be changed later
		return;
	} else if(t == CYPHER_AST_CREATE) {
		// Only add at most one Create op per segment. TODO Revisit and improve this logic.
		if(ExecutionPlan_LocateOp(segment->root, OPType_CREATE)) return;
		_buildCreateOp(gc, ast, segment, stats);
	} else if(t == CYPHER_AST_UNWIND) {
		_buildUnwindOp(segment, clause);
	} else if(t == CYPHER_AST_MERGE) {
		_buildMergeOp(gc, ast, segment, clause, stats);
	} else if(t == CYPHER_AST_SET) {
		_buildUpdateOp(gc, segment, clause, stats);
	} else if(t == CYPHER_AST_DELETE) {
		_buildDeleteOp(segment, clause, stats);
	} else if(t == CYPHER_AST_RETURN) {
		// Converting a RETURN clause can create multiple operations.
		_buildReturnOps(segment, clause);
	} else if(t == CYPHER_AST_WITH) {
		// Converting a WITH clause can create multiple operations.
		_buildWithOps(segment, clause);
	}
}

static void _ExecutionPlan_PlaceFilterOps(ExecutionPlan *plan) {
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
		raxFree(references);
	}
	Vector_Free(sub_trees);
}

static void _NewExecutionPlanSegment(RedisModuleCtx *ctx, GraphContext *gc,
									 ExecutionPlanSegment *segment, AST *ast, ResultSet *result_set,
									 AR_ExpNode **prev_projections, OpBase *prev_op) {
	// Initialize map of Record IDs
	RecordMap *record_map = RecordMap_New();
	segment->record_map = record_map;

	if(prev_projections) {
		// We have an array of identifiers provided by a prior WITH clause -
		// these will correspond to our first Record entities
		uint projection_count = array_len(prev_projections);
		for(uint i = 0; i < projection_count; i++) {
			AR_ExpNode *projection = prev_projections[i];
			RecordMap_FindOrAddAlias(record_map, projection->resolved_name);
		}
	}

	/* Build projections from this AST's WITH, RETURN, and ORDER clauses. */
	_ExecutionPlanSegment_BuildProjections(ast, segment, prev_projections);

	// Extend the RecordMap to include references from clauses that do not form projections
	// (SET, CREATE, DELETE)
	_ExecutionPlanSegment_MapReferences(segment, ast);

	// Build query graph
	QueryGraph *qg = BuildQueryGraph(gc, ast);
	segment->query_graph = qg;

	// Build filter tree
	FT_FilterNode *filter_tree = AST_BuildFilterTree(ast, record_map);
	segment->filter_tree = filter_tree;

	// Instantiate the array that will hold all constructed operations.
	// TODO This array could be removed with some logical changes.
	uint clause_count = cypher_ast_query_nclauses(ast->root);

	/* If we have a procedure call, build an op for it first.
	 * This is currently necessary to extend the segment's projections if necesary
	 * and in order to build the plan for queries of forms like CALL..MATCH properly.
	 * TODO Fix this so that CALL clauses are handled in the ConvertClause loop. */
	const cypher_astnode_t *call_clause = AST_GetClause(ast, CYPHER_AST_CALL);
	if(call_clause) _buildCallOp(ast, segment, call_clause);

	// Build traversal operations for every connected component in the QueryGraph
	if(AST_ContainsClause(ast, CYPHER_AST_MATCH) || AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
		_ExecutionPlanSegment_ProcessQueryGraph(segment, qg, ast, filter_tree);
	}

	// If we are in a querying context, retrieve a pointer to the statistics for operations
	// like DELETE that only produce metadata.
	ResultSetStatistics *stats = (result_set) ? &result_set->stats : NULL;

	for(uint i = 0; i < clause_count; i ++) {
		// Build the appropriate operation(s) for each clause in the query.
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);
		_ExecutionPlanSegment_ConvertClause(gc, ast, segment, stats, clause);
	}

	// The root operation is OpResults only if the query culminates in a RETURN or CALL clause.
	cypher_astnode_type_t last_clause_type = cypher_astnode_type(cypher_ast_query_get_clause(ast->root,
																 clause_count - 1));
	if(last_clause_type == CYPHER_AST_RETURN || last_clause_type == CYPHER_AST_CALL) {
		OpBase *results_op = NewResultsOp(result_set);
		_ExecutionPlan_UpdateRoot(segment, results_op);
	}

	// If this is not the first segment to be constructed, connect this segment's
	// operations tree with the previous one.
	if(prev_op) {
		OpBase *leaf = _ExecutionPlan_LocateLeaf(segment->root);
		_ExecutionPlan_ConnectSegmentOps(leaf, prev_op, prev_projections);
	}

	if(plan->filter_tree) _ExecutionPlanSegment_PlaceFilterOps(plan);


	return plan;
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, ResultSet *result_set) {
	AST *ast = QueryCtx_GetAST();

	/* Execution plans are created in 1 or more segments. Every WITH clause demarcates the end of
	 * a segment, and the next clause begins a new one. */
	uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
	bool query_has_return = AST_ContainsClause(ast, CYPHER_AST_RETURN);
	uint segment_count = with_clause_count + query_has_return + 1;

	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	if(query_has_return) {
		segment_indices = array_append(segment_indices, cypher_ast_query_nclauses(ast->root) - 1);
	}
	segment_indices = array_append(segment_indices, cypher_ast_query_nclauses(ast->root)); // TODO ?

	uint start_offset = 0;
	ExecutionPlan *segments[segment_count];

	for(int i = 0; i < segment_count; i++) {
		/* A segment needs to know about it referenced entities. Those entities are known only when projection
		 * operation executes over the records of the current segment. For example:
		 * "Match (a)-[b]->(c) return a,b" only references a and b.
		 * Since project operation is a tap it cannot be built as the closing operation of a current segment,
		 * but in order for the information about the segment to be complete, the current segment AST will
		 * hold its projection information, if valid.
		 * This is relevant for all non-last segments. */
		// uint end_offset = (i == segment_count - 1) ? segment_indices[i] : segment_indices[i] + 1;
		uint end_offset = segment_indices[i];
		// Slice the AST to only include the clauses in the current segment.
		AST *ast_segment = AST_NewSegment(ast, start_offset, end_offset);

		// Construct a new ExecutionPlanSegment.
		ExecutionPlan *segment = _NewExecutionPlan(ctx, gc, ast_segment, result_set);

		// Free the AST segment.
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

	// Free current AST segment if it has been constructed here.
	// if(ast_segment != ast) AST_Free(ast_segment); // TODO leak?

	// Check to see if we've encountered an error while constructing the execution-plan.
	if(QueryCtx_EncounteredError()) {
		// A compile time error was encountered.
		ExecutionPlan_Free(plan);
		QueryCtx_EmitException();
		return NULL;
	}

	// Optimize the operations in the ExecutionPlan.
	optimizePlan(gc, plan);

	// plan->segment_count = segment_count;
	// Prepare column names for the ResultSet if this query contains data in addition to statistics.
	// if(result_set) ResultSet_BuildColumns(result_set, plan->projections);

	return plan;
}

OpBase *ExecutionPlan_LocateLeaf(OpBase *root) {
	if(root->childCount == 0) return root;
	return ExecutionPlan_LocateLeaf(root->children[0]);
}

rax *ExecutionPlan_GetMappings(const ExecutionPlan *plan) {
	assert(plan && plan->record_map);
	return plan->record_map;
}

rax *ExecutionPlan_GetProjectionMap(const ExecutionPlan *plan) {
	assert(plan && plan->projection_map); // TODO could allocate here if plan weren't const
	return plan->projection_map;
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

void ExecutionPlanInit(ExecutionPlan *plan) {
	if(!plan) return;
	_ExecutionPlanInit(plan->root);
	// for(int i = 0; i < plan->segment_count; i ++) {
	// _ExecutionPlanInit(plan->segments[i]->root);
	// }
}

ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan) {
	Record r = NULL;
	ExecutionPlanInit(plan);

	/* Set an exception-handling breakpoint to capture run-time errors.
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	int encountered_error = SET_EXCEPTION_HANDLER();

	if(encountered_error) {
		// Encountered a run-time error; return immediately.
		if(r) Record_Free(r);
		return plan->result_set;
	}

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
	raxFree(plan->record_map);
	raxFree(plan->projection_map);
	rm_free(plan);
}

