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

// Associate each operation in the chain with the provided RecordMap.
static void _associateRecordMap(OpBase *root, RecordMap *record_map) {
	// If this op has already been initialized,
	// we don't need to recurse further.
	if(root->record_map != NULL) return;

	// Share this ExecutionPlanSegment's record map with the operation.
	root->record_map = record_map;

	for(int i = 0; i < root->childCount; i++) {
		_associateRecordMap(root->children[i], record_map);
	}
}

/* In a query with a RETURN *, build projections for all explicit aliases
 * in previous clauses. */
static void _ReturnExpandAll(ExecutionPlanSegment *segment, AST *ast) {
	// Collect all unique aliases
	const char **aliases = AST_CollectElementNames(ast);
	uint count = array_len(aliases);

	// Build an expression for each alias
	segment->projections = array_new(AR_ExpNode *, count);
	for(int i = 0; i < count; i ++) {
		AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(segment->record_map, aliases[i], NULL);
		exp->resolved_name = aliases[i];
		segment->projections = array_append(segment->projections, exp);
	}

	array_free(aliases);
}

// Handle ORDER entities
static AR_ExpNode **_BuildOrderExpressions(RecordMap *record_map, AR_ExpNode **projections,
										   const cypher_astnode_t *order_clause) {
	uint projection_count = array_len(projections);
	uint count = cypher_ast_order_by_nitems(order_clause);
	AR_ExpNode **order_exps = array_new(AR_ExpNode *, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
		AR_ExpNode *exp = NULL;
		if(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER) {
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
				exp = AR_EXP_FromExpression(record_map, ast_exp);
			}
		} else {
			// Independent operator like:
			// ORDER BY COUNT(a)
			exp = AR_EXP_FromExpression(record_map, ast_exp);
		}

		order_exps = array_append(order_exps, exp);
		// TODO direction should be specifiable per order entity
		// ascending = cypher_ast_sort_item_is_ascending(item);
	}

	return order_exps;
}

// Handle RETURN entities
// (This function is not static because it is relied upon by unit tests)
void _BuildReturnExpressions(ExecutionPlanSegment *segment, const cypher_astnode_t *ret_clause,
							 AST *ast) {
	RecordMap *record_map = segment->record_map;
	// Query is of type "RETURN *",
	// collect all defined identifiers and create return elements for them
	if(cypher_ast_return_has_include_existing(ret_clause)) {
		_ReturnExpandAll(segment, ast);
		return;
	}

	uint count = cypher_ast_return_nprojections(ret_clause);
	segment->projections = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
		// The AST expression can be an identifier, function call, or constant
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this return entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

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
		segment->projections = array_append(segment->projections, exp);
	}
}

static AR_ExpNode **_BuildWithExpressions(RecordMap *record_map,
										  const cypher_astnode_t *with_clause) {
	uint count = cypher_ast_with_nprojections(with_clause);
	AR_ExpNode **with_expressions = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

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
static AR_ExpNode **_BuildCallProjections(RecordMap *record_map,
										  const cypher_astnode_t *call_clause, AST *ast) {
	// Handle yield entities
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	AR_ExpNode **expressions = array_new(AR_ExpNode *, yield_count);

	for(uint i = 0; i < yield_count; i ++) {
		const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(record_map, ast_exp);

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

			// TODO the 'name' variable doesn't have an AST ID, so an assertion in
			// AR_EXP_NewVariableOperandNode() fails without this call.
			// Consider alternatives, because this is an annoying kludge.
			ASTMap_FindOrAddAlias(ast, name, IDENTIFIER_NOT_FOUND);
			AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(record_map, name, NULL);
			exp->resolved_name = name;
			expressions = array_append(expressions, exp);
		}
		Proc_Free(proc);
	}

	return expressions;
}

/* Strings enclosed in the parentheses of a CALL clause represent the arguments to the procedure.
 * _BuildCallArguments creates a string array holding all of these arguments. */
static const char **_BuildCallArguments(RecordMap *record_map,
										const cypher_astnode_t *call_clause) {
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

static void _ExecutionPlanSegment_ProcessQueryGraph(ExecutionPlanSegment *segment, QueryGraph *qg,
													AST *ast, FT_FilterNode *ft, Vector *ops) {
	GraphContext *gc = QueryCtx_GetGraphCtx();

	QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
	uint connectedComponentsCount = array_len(connectedComponents);
	segment->connected_components = connectedComponents;

	/* For every connected component.
	 * Incase we're dealing with multiple components
	 * we'll simply join them all together with a join operation. */
	OpBase *cartesianProduct = NULL;
	if(connectedComponentsCount > 1) {
		cartesianProduct = NewCartesianProductOp();
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
			uint rec_idx = RecordMap_FindOrAddID(segment->record_map, n->id);
			if(n->labelID != GRAPH_NO_LABEL) op = NewNodeByLabelScanOp(n, rec_idx);
			else op = NewAllNodeScanOp(gc->g, n, rec_idx);
			Vector_Push(traversals, op);
		} else {
			size_t expCount = 0;
			AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc, segment->record_map, &expCount);

			// Reorder exps, to the most performant arrangement of evaluation.
			orderExpressions(exps, expCount, segment->record_map, ft);

			AlgebraicExpression *exp = exps[0];
			selectEntryPoint(exp, segment->record_map, ft);

			// Retrieve the AST ID for the source node
			uint ast_id = exp->src_node->id;
			// Convert to a Record ID
			uint record_id = RecordMap_FindOrAddID(segment->record_map, ast_id);

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
				op = NewNodeByLabelScanOp(exp->src_node, record_id);
			} else {
				op = NewAllNodeScanOp(gc->g, exp->src_node, record_id);
			}
			Vector_Push(traversals, op);

			for(int i = 0; i < expCount; i++) {
				exp = exps[i];
				if(exp->operand_count == 0) continue;

				if(exp->edge && QGEdge_VariableLength(exp->edge)) {
					op = NewCondVarLenTraverseOp(gc->g, segment->record_map, exp);
				} else {
					op = NewCondTraverseOp(gc->g, segment->record_map, exp, TraverseRecordCap(ast));
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

static void _ExecutionPlanSegment_MapAliasesInExpression(ExecutionPlanSegment *segment,
														 const cypher_astnode_t *expr) {
	if(!expr) return;

	if(cypher_astnode_type(expr) == CYPHER_AST_IDENTIFIER) {
		// Encountered an alias - verify that it is represented in the RecordMap.
		const char *alias = cypher_ast_identifier_get_name(expr);
		RecordMap_FindOrAddAlias(segment->record_map, alias);
	}

	uint child_count = cypher_astnode_nchildren(expr);
	for(uint i = 0; i < child_count; i ++) {
		_ExecutionPlanSegment_MapAliasesInExpression(segment, cypher_astnode_get_child(expr, i));
	}
}

// Ensure that aliases that are referred to in a pattern but not projected still get added to the Record.
static void _ExecutionPlanSegment_MapAliasesInPattern(ExecutionPlanSegment *segment,
													  const cypher_astnode_t *pattern) {
	uint npaths = cypher_ast_pattern_npaths(pattern);

	for(uint i = 0; i < npaths; i++) {
		const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
		uint path_elem_count = cypher_ast_pattern_path_nelements(path);
		for(uint j = 0; j < path_elem_count; j ++) {
			const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, j);
			const cypher_astnode_t *ast_props = (j % 2) ? cypher_ast_rel_pattern_get_properties(elem) :
												cypher_ast_node_pattern_get_properties(elem);

			if(ast_props) {
				assert(cypher_astnode_type(ast_props) == CYPHER_AST_MAP &&
					   "parameters are not currently supported");
				uint value_count = cypher_ast_map_nentries(ast_props);
				for(uint k = 0; k < value_count; k ++) {
					const cypher_astnode_t *value = cypher_ast_map_get_value(ast_props, k);
					_ExecutionPlanSegment_MapAliasesInExpression(segment, value);
				}
			}

		}
	}
}

// Map the AST entities referred to in SET, CREATE, and DELETE clauses.
// This is necessary so that edge references will be constructed prior to forming AlgebraicExpressions.
static void _ExecutionPlanSegment_MapReferences(ExecutionPlanSegment *segment, AST *ast) {
	const cypher_astnode_t **set_clauses = AST_GetClauses(ast, CYPHER_AST_SET);
	if(set_clauses) {
		uint set_count = array_len(set_clauses);
		for(uint i = 0; i < set_count; i ++) {
			const cypher_astnode_t *set_clause = set_clauses[i];
			uint nitems = cypher_ast_set_nitems(set_clause);
			for(uint j = 0; j < nitems; j++) {
				const cypher_astnode_t *set_item = cypher_ast_set_get_item(set_clause, j);
				const cypher_astnode_t *key_to_set = cypher_ast_set_property_get_property(
														 set_item); // type == CYPHER_AST_PROPERTY_OPERATOR
				const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(key_to_set);
				assert(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
				const char *alias = cypher_ast_identifier_get_name(prop_expr);
				RecordMap_FindOrAddAlias(segment->record_map, alias);
			}
		}
		array_free(set_clauses);
	}

	const cypher_astnode_t **create_clauses = AST_GetClauses(ast, CYPHER_AST_CREATE);
	if(create_clauses) {
		uint create_count = array_len(create_clauses);
		for(uint i = 0; i < create_count; i ++) {
			const cypher_astnode_t *create_pattern = cypher_ast_create_get_pattern(create_clauses[i]);
			_ExecutionPlanSegment_MapAliasesInPattern(segment, create_pattern);
		}
		array_free(create_clauses);
	}

	const cypher_astnode_t **delete_clauses = AST_GetClauses(ast, CYPHER_AST_DELETE);
	if(delete_clauses) {
		uint delete_count = array_len(delete_clauses);
		for(uint i = 0; i < delete_count; i ++) {
			const cypher_astnode_t *delete_clause = delete_clauses[i];
			uint nitems = cypher_ast_delete_nexpressions(delete_clause);
			for(uint j = 0; j < nitems; j++) {
				const cypher_astnode_t *ast_expr = cypher_ast_delete_get_expression(delete_clause, j);
				assert(cypher_astnode_type(ast_expr) == CYPHER_AST_IDENTIFIER);
				const char *alias = cypher_ast_identifier_get_name(ast_expr);
				RecordMap_FindOrAddAlias(segment->record_map, alias);
			}
		}
		array_free(delete_clauses);
	}
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
	// Retrieve a RETURN clause if one is specified in this AST's range
	const cypher_astnode_t *ret_clause = AST_GetClause(ast, CYPHER_AST_RETURN);
	// Retrieve a WITH clause if one is specified in this AST's range
	const cypher_astnode_t *with_clause = AST_GetClause(ast, CYPHER_AST_WITH);
	// We cannot have both a RETURN and WITH clause
	assert(!(ret_clause && with_clause));
	segment->projections = NULL;

	const cypher_astnode_t *order_clause = NULL;
	if(ret_clause) {
		_BuildReturnExpressions(segment, ret_clause, ast);
		// If we have a RETURN * and previous WITH projections, include those projections.
		if(cypher_ast_return_has_include_existing(ret_clause) && prev_projections) {
			uint prev_projection_count = array_len(prev_projections);
			for(uint i = 0; i < prev_projection_count; i++) {
				// Build a new projection that's just the WITH identifier
				AR_ExpNode *projection = AR_EXP_NewVariableOperandNode(record_map,
																	   prev_projections[i]->resolved_name, NULL);
				projection->resolved_name = prev_projections[i]->resolved_name;
				segment->projections = array_append(segment->projections, projection);
			}
		}
		order_clause = cypher_ast_return_get_order_by(ret_clause);
	} else if(with_clause) {
		segment->projections = _BuildWithExpressions(segment->record_map, with_clause);
		order_clause = cypher_ast_with_get_order_by(with_clause);
	}

	AR_ExpNode **order_expressions = NULL;
	if(order_clause) order_expressions = _BuildOrderExpressions(segment->record_map,
																	segment->projections, order_clause);

	// Extend the RecordMap to include references from clauses that do not form projections
	// (SET, CREATE, DELETE)
	_ExecutionPlanSegment_MapReferences(segment, ast);

	Vector *ops = NewVector(OpBase *, 1);

	// Build query graph
	QueryGraph *qg = BuildQueryGraph(gc, ast);
	segment->query_graph = qg;

	// Build filter tree
	FT_FilterNode *filter_tree = AST_BuildFilterTree(ast, record_map);
	segment->filter_tree = filter_tree;

	const cypher_astnode_t *call_clause = AST_GetClause(ast, CYPHER_AST_CALL);
	if(call_clause) {
		// A call clause has a procedure name, 0+ arguments (parenthesized expressions), and a projection if YIELD is included
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		const char **arguments = _BuildCallArguments(record_map, call_clause);
		AR_ExpNode **yield_exps = _BuildCallProjections(record_map, call_clause, ast);
		uint yield_count = array_len(yield_exps);
		const char **yields = array_new(const char *, yield_count);

		uint *call_modifies = array_new(uint, yield_count);
		for(uint i = 0; i < yield_count; i ++) {
			// Track the names of yielded variables.
			// yields = array_append(yields, yield_exps[i]->resolved_name);
			yields = array_append(yields, yield_exps[i]->operand.variadic.entity_alias);
			// Track which variables are modified by this operation.
			call_modifies = array_append(call_modifies, RecordMap_LookupAlias(record_map,
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

		OpBase *opProcCall = NewProcCallOp(proc_name, arguments, yields, call_modifies);
		Vector_Push(ops, opProcCall);
	}

	// Build traversal operations for every connected component in the QueryGraph
	if(AST_ContainsClause(ast, CYPHER_AST_MATCH) || AST_ContainsClause(ast, CYPHER_AST_MERGE)) {
		_ExecutionPlanSegment_ProcessQueryGraph(segment, qg, ast, filter_tree, ops);
	}

	// If we are in a querying context, retrieve a pointer to the statistics for operations
	// like DELETE that only produce metadata.
	ResultSetStatistics *stats = (result_set) ? &result_set->stats : NULL;

	// Set root operation
	const cypher_astnode_t *unwind_clause = AST_GetClause(ast, CYPHER_AST_UNWIND);
	if(unwind_clause) {
		AST_UnwindContext unwind_ast_ctx = AST_PrepareUnwindOp(unwind_clause, record_map);

		OpBase *opUnwind = NewUnwindOp(unwind_ast_ctx.record_idx, unwind_ast_ctx.exp);
		Vector_Push(ops, opUnwind);
	}

	bool create_clause = AST_ContainsClause(ast, CYPHER_AST_CREATE);
	if(create_clause) {
		AST_CreateContext create_ast_ctx = AST_PrepareCreateOp(gc, record_map, ast, qg);
		OpBase *opCreate = NewCreateOp(stats,
									   create_ast_ctx.nodes_to_create,
									   create_ast_ctx.edges_to_create);
		Vector_Push(ops, opCreate);
	}

	const cypher_astnode_t *merge_clause = AST_GetClause(ast, CYPHER_AST_MERGE);
	if(merge_clause) {
		// A merge clause provides a single path that must exist or be created.
		// As with paths in a MATCH query, build the appropriate traversal operations
		// and append them to the set of ops.
		AST_MergeContext merge_ast_ctx = AST_PrepareMergeOp(gc, record_map, ast, merge_clause, qg);

		// Append a merge operation
		OpBase *opMerge = NewMergeOp(stats,
									 merge_ast_ctx.nodes_to_merge,
									 merge_ast_ctx.edges_to_merge);
		Vector_Push(ops, opMerge);
	}

	const cypher_astnode_t *delete_clause = AST_GetClause(ast, CYPHER_AST_DELETE);
	if(delete_clause) {
		uint *nodes_ref;
		uint *edges_ref;
		AST_PrepareDeleteOp(delete_clause, qg, record_map, &nodes_ref, &edges_ref);
		OpBase *opDelete = NewDeleteOp(nodes_ref, edges_ref, stats);
		Vector_Push(ops, opDelete);
	}

	const cypher_astnode_t *set_clause = AST_GetClause(ast, CYPHER_AST_SET);
	if(set_clause) {
		// Create a context for each update expression.
		uint nitems;
		EntityUpdateEvalCtx *update_exps = AST_PrepareUpdateOp(set_clause, record_map, &nitems);
		OpBase *op_update = NewUpdateOp(gc, update_exps, nitems, stats);
		Vector_Push(ops, op_update);
	}

	assert(!(with_clause && ret_clause));

	uint *modifies = NULL;

	// WITH/RETURN projections have already been constructed from the AST
	AR_ExpNode **projections = segment->projections;

	if(with_clause || ret_clause || call_clause) {
		// TODO improve interface, maybe CollectEntityIDs variant that builds an array
		rax *modifies_ids = raxNew();
		uint exp_count = array_len(projections);
		for(uint i = 0; i < exp_count; i ++) {
			AR_ExpNode *exp = projections[i];
			AR_EXP_CollectEntityIDs(exp, modifies_ids);
		}

		modifies = array_new(uint, raxSize(modifies_ids));
		raxIterator iter;
		raxStart(&iter, modifies_ids);
		raxSeek(&iter, ">=", (unsigned char *)"", 0);
		while(raxNext(&iter)) {
			modifies = array_append(modifies, *(uint *)iter.key);
		}
		raxFree(modifies_ids);
	}

	OpBase *op;

	if(with_clause) {
		// uint *with_projections = AST_WithClauseModifies(ast, with_clause);
		if(AST_ClauseContainsAggregation(with_clause)) {
			op = NewAggregateOp(projections, modifies);
		} else {
			op = NewProjectOp(projections, modifies);
		}
		Vector_Push(ops, op);

		if(cypher_ast_with_is_distinct(with_clause)) {
			op = NewDistinctOp();
			Vector_Push(ops, op);
		}

		const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(with_clause);
		const cypher_astnode_t *limit_clause = cypher_ast_with_get_limit(with_clause);

		uint skip = 0;
		uint limit = 0;
		if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
		if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

		if(order_expressions) {
			const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(with_clause);
			int direction = AST_PrepareSortOp(order_clause);
			// The sort operation will obey a specified limit, but must account for skipped records
			uint sort_limit = (limit > 0) ? limit + skip : 0;
			op = NewSortOp(order_expressions, direction, sort_limit);
			Vector_Push(ops, op);
		}

		if(skip_clause) {
			OpBase *op_skip = NewSkipOp(skip);
			Vector_Push(ops, op_skip);
		}

		if(limit_clause) {
			OpBase *op_limit = NewLimitOp(limit);
			Vector_Push(ops, op_limit);
		}
	} else if(ret_clause) {
		// TODO we may not need a new project op if the query is something like:
		// MATCH (a) WITH a.val AS val RETURN val
		// Though we would still need a new projection (barring later optimizations) for:
		// MATCH (a) WITH a.val AS val RETURN val AS e
		if(AST_ClauseContainsAggregation(ret_clause)) {
			op = NewAggregateOp(projections, modifies);
		} else {
			op = NewProjectOp(projections, modifies);
		}
		Vector_Push(ops, op);

		if(cypher_ast_return_is_distinct(ret_clause)) {
			op = NewDistinctOp();
			Vector_Push(ops, op);
		}

		const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(ret_clause);
		const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(ret_clause);
		const cypher_astnode_t *limit_clause = cypher_ast_return_get_limit(ret_clause);

		uint skip = 0;
		uint limit = 0;
		if(skip_clause) skip = AST_ParseIntegerNode(skip_clause);
		if(limit_clause) limit = AST_ParseIntegerNode(limit_clause);

		if(order_expressions) {
			int direction = AST_PrepareSortOp(order_clause);
			// The sort operation will obey a specified limit, but must account for skipped records
			uint sort_limit = (limit > 0) ? limit + skip : 0;
			op = NewSortOp(order_expressions, direction, sort_limit);
			Vector_Push(ops, op);
		}

		if(skip_clause) {
			OpBase *op_skip = NewSkipOp(skip);
			Vector_Push(ops, op_skip);
		}

		if(limit_clause) {
			OpBase *op_limit = NewLimitOp(limit);
			Vector_Push(ops, op_limit);
		}

		op = NewResultsOp(result_set, qg);
		Vector_Push(ops, op);
	} else if(call_clause) {
		op = NewResultsOp(result_set, qg);
		Vector_Push(ops, op);
	}

	OpBase *parent_op;
	OpBase *child_op;
	Vector_Pop(ops, &parent_op);
	segment->root = parent_op;

	while(Vector_Pop(ops, &child_op)) {
		ExecutionPlan_AddOp(parent_op, child_op);
		parent_op = child_op;
	}

	Vector_Free(ops);

	if(prev_op) {
		// Need to connect this segment to the previous one.
		// If the last operation of this segment is a potential data producer, join them
		// under a Cartesian Product operation. (This could be an Apply op if preferred.)
		if(parent_op->type & OP_TAPS) {
			OpBase *op_cp = NewCartesianProductOp();
			uint prev_projection_count = array_len(prev_projections);
			// The Apply op must be associated with the IDs projected from the previous segment
			// in case we're placing filter operations.
			// (although this is a rather ugly way to accomplish that.)
			op_cp->modifies = array_new(uint, prev_projection_count);
			for(uint i = 0; i < prev_projection_count; i ++) {
				op_cp->modifies = array_append(op_cp->modifies, i);
			}

			ExecutionPlan_PushBelow(parent_op, op_cp);
			ExecutionPlan_AddOp(op_cp, prev_op);
		} else {
			// All operations can be connected in a single chain.
			ExecutionPlan_AddOp(parent_op, prev_op);
		}
	}

	if(segment->filter_tree) {
		Vector *sub_trees = FilterTree_SubTrees(segment->filter_tree);

		/* For each filter tree find the earliest position along the execution
		 * after which the filter tree can be applied. */
		for(int i = 0; i < Vector_Size(sub_trees); i++) {
			FT_FilterNode *tree;
			Vector_Get(sub_trees, i, &tree);
			rax *references = FilterTree_CollectModified(tree);

			if(raxSize(references) > 0) {
				/* Scan execution segment, locate the earliest position where all
				 * references been resolved. */
				op = ExecutionPlan_LocateReferences(segment->root, references);
				assert(op);
			} else {
				/* The filter tree does not contain references, like:
				 * WHERE 1=1
				 * TODO This logic is inadequate. For now, we'll place the op
				 * directly below the first projection (hopefully there is one!). */
				op = segment->root;
				while(op->childCount > 0 && op->type != OPType_PROJECT && op->type != OPType_AGGREGATE) {
					op = op->children[0];
				}
			}

			/* Create filter node.
			 * Introduce filter op right below located op. */
			OpBase *filter_op = NewFilterOp(tree);
			ExecutionPlan_PushBelow(op, filter_op);
			raxFree(references);
		}
		Vector_Free(sub_trees);
	}

	_associateRecordMap(segment->root, record_map);
}

ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, GraphContext *gc, ResultSet *result_set) {
	AST *ast = QueryCtx_GetAST();

	ExecutionPlan *plan = rm_calloc(1, sizeof(ExecutionPlan));

	plan->result_set = result_set;

	/* Execution plans are created in 1 or more segments. Every WITH clause demarcates the end of
	 * a segment, and the next clause begins a new one. */
	uint with_clause_count = AST_GetClauseCount(ast, CYPHER_AST_WITH);
	plan->segment_count = with_clause_count + 1;

	plan->segments = rm_malloc(plan->segment_count * sizeof(ExecutionPlanSegment));

	// Retrieve the indices of each WITH clause to properly set the bounds of each segment.
	uint *segment_indices = NULL;
	if(with_clause_count > 0) segment_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);

	/* Set an exception-handling breakpoint to capture compile-time errors.
	 * We might encounter a compile-time error while attempting to reduce expression trees to scalars.
	 *
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	bool free_exception_cause = true; // The cause of a compile-time error will be freed on occurrence.
	int encountered_error = SET_EXCEPTION_HANDLER(free_exception_cause);

	if(encountered_error) {
		// Encountered a compile-time error.
		char *err = QueryCtx_GetError();
		// The reply arrays have not been instantiated at this point, so we'll return a Redis-level error.
		RedisModule_ReplyWithError(ctx, err);
		ExecutionPlan_Free(plan); // Free the partial execution plan.
		return NULL;
	}

	uint i = 0;
	uint end_offset;
	uint start_offset = 0;
	OpBase *prev_op = NULL;
	ExecutionPlanSegment *segment = NULL;
	AR_ExpNode **input_projections = NULL;

	// The original AST does not need to be modified if our query only has one segment
	AST *ast_segment = ast;
	if(with_clause_count > 0) {
		// Construct all segments except the final one in a loop.
		for(i = 0; i < with_clause_count; i++) {
			end_offset = segment_indices[i] + 1; // Switching from index to bound, so add 1
			// Slice the AST to only include the clauses in the current segment.
			ast_segment = AST_NewSegment(ast, start_offset, end_offset);

			// Construct a new ExecutionPlanSegment.
			segment = rm_calloc(1, sizeof(ExecutionPlanSegment));
			plan->segments[i] = segment; // Add the segment now in case of an exception during construction.
			_NewExecutionPlanSegment(ctx, gc, segment, ast_segment, plan->result_set, input_projections,
									 prev_op);

			AST_Free(ast_segment); // Free all AST constructions scoped to this segment

			// Store the root op and the expressions constructed by this segment's
			// WITH projection to pass into the *next* segment
			prev_op = segment->root;
			input_projections = segment->projections;
			start_offset = end_offset;
		}
		// Prepare the last AST segment
		end_offset = cypher_astnode_nchildren(ast->root);
		ast_segment = AST_NewSegment(ast, start_offset, end_offset);
	}

	if(segment_indices) array_free(segment_indices);

	// Construct a new ExecutionPlanSegment.
	segment = rm_calloc(1, sizeof(ExecutionPlanSegment));
	plan->segments[i] = segment; // Add the segment now in case of an exception during construction.
	_NewExecutionPlanSegment(ctx, gc, segment, ast_segment, plan->result_set, input_projections,
							 prev_op);

	plan->root = plan->segments[i]->root;

	// Optimize the operations in the ExecutionPlan.
	optimizePlan(gc, plan);

	// Prepare column names for the ResultSet if this query contains data in addition to statistics.
	if(result_set) ResultSet_BuildColumns(result_set, segment->projections);


	// Free current AST segment if it has been constructed here.
	if(ast_segment != ast) AST_Free(ast_segment);

	return plan;
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

void _ExecutionPlanInit(OpBase *root, RecordMap *record_map) {
	// Share this ExecutionPlanSegment's record map with the operation.
	// This will already have been set unless the operation was introduced by an optimization.
	if(root->record_map == NULL) root->record_map = record_map;

	// Initialize the operation if necesary.
	if(!root->op_initialized && root->init) {
		root->init(root);
		root->op_initialized = true;
	}

	// Continue initializing downstream operations.
	for(int i = 0; i < root->childCount; i++) {
		_ExecutionPlanInit(root->children[i], record_map);
	}
}

void ExecutionPlanInit(ExecutionPlan *plan) {
	if(!plan) return;
	for(int i = 0; i < plan->segment_count; i ++) {
		RecordMap *segment_map = plan->segments[i]->record_map;
		_ExecutionPlanInit(plan->segments[i]->root, segment_map);
	}
}

ResultSet *ExecutionPlan_Execute(ExecutionPlan *plan) {
	Record r = NULL;
	ExecutionPlanInit(plan);

	bool free_exception_cause = false; // The causes of run-time errors will be freed in cleanup.

	/* Set an exception-handling breakpoint to capture run-time errors.
	 * encountered_error will be set to 0 when setjmp is invoked, and will be nonzero if
	 * a downstream exception returns us to this breakpoint. */
	int encountered_error = SET_EXCEPTION_HANDLER(false);

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

void _ExecutionPlanSegment_Free(ExecutionPlanSegment *segment) {
	if(segment->record_map) RecordMap_Free(segment->record_map);

	if(segment->connected_components) {
		uint connected_component_count = array_len(segment->connected_components);
		for(uint i = 0; i < connected_component_count; i ++) {
			QueryGraph_Free(segment->connected_components[i]);
		}
		array_free(segment->connected_components);
	}

	if(segment->query_graph) QueryGraph_Free(segment->query_graph);

	if(segment->projections) {
		uint projection_count = array_len(segment->projections);
		for(uint i = 0; i < projection_count; i ++) {
			AR_EXP_Free(segment->projections[i]);
		}
		array_free(segment->projections);
	}

	rm_free(segment);
}

void ExecutionPlan_Free(ExecutionPlan *plan) {
	if(plan == NULL) return;
	if(plan->root) _ExecutionPlan_FreeOperations(plan->root);

	if(plan->segments) {
		for(uint i = 0; i < plan->segment_count; i ++) {
			if(plan->segments[i]) _ExecutionPlanSegment_Free(plan->segments[i]);
		}
		rm_free(plan->segments);
	}

	rm_free(plan);
}

