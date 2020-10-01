#include "execution_plan_construct.h"
#include "execution_plan_modify.h"
#include "../execution_plan.h"
#include "../ops/ops.h"
#include "../optimizations/traverse_order.h"
#include "../../query_ctx.h"
#include "../../util/rax_extensions.h"
#include "../../ast/ast_build_filter_tree.h"

static void _ExecutionPlan_ProcessQueryGraph(ExecutionPlan *plan, QueryGraph *qg,
											 AST *ast) {
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Build the full FilterTree for this AST so that we can order traversals properly.
	FT_FilterNode *ft = AST_BuildFilterTree(ast);
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
		ExecutionPlan_UpdateRoot(plan, cartesianProduct);
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
			if(n->labelID != GRAPH_NO_LABEL) {
				NodeScanCtx ctx = NODE_CTX_NEW(n->alias, n->label, n->labelID);
				root = NewNodeByLabelScanOp(plan, ctx);
			} else {
				root = NewAllNodeScanOp(plan, n->alias);
			}
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
					AlgebraicExpression_Free(AlgebraicExpression_RemoveSource(&exps[0]));
				}
				NodeScanCtx ctx = NODE_CTX_NEW(src->alias, src->label, src->labelID);
				root = tail = NewNodeByLabelScanOp(plan, ctx);
			} else {
				root = tail = NewAllNodeScanOp(plan, src->alias);
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
			ExecutionPlan_UpdateRoot(plan, root);
		}
	}
	FilterTree_Free(ft);
}

static void _buildOptionalMatchOps(ExecutionPlan *plan, AST *ast, const cypher_astnode_t *clause) {
	const char **arguments = NULL;
	OpBase *optional = NewOptionalOp(plan);

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
		ExecutionPlan_UpdateRoot(plan, apply_op);

		// Create an Optional op and add it as an Apply child as a right-hand stream.
		ExecutionPlan_AddOp(apply_op, optional);
	}

	// Build the new Match stream and add it to the Optional stream.
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, clause);
	ExecutionPlan_AddOp(optional, match_stream);

	// Build the FilterTree to model any WHERE predicates on this clause and place ops appropriately.
	FT_FilterNode *sub_ft = AST_BuildFilterTreeFromClauses(ast, &clause, 1);
	if(sub_ft) ExecutionPlan_PlaceFilterOps(plan, match_stream, NULL, sub_ft);
	// If no root has been set (OPTIONAL was the first clause), set it to the Optional op.
	if(!plan->root) ExecutionPlan_UpdateRoot(plan, optional);

	array_free(arguments);
}


void buildMatchOpTree(ExecutionPlan *plan, AST *ast, const cypher_astnode_t *clause) {
	if(cypher_ast_match_is_optional(clause)) {
		_buildOptionalMatchOps(plan, ast, clause);
		return;
	}

	/* Only add at most one set of traversals per plan.
	 * TODO Revisit and improve this logic. */
	if(plan->root && ExecutionPlan_LocateOpMatchingType(plan->root, SCAN_OPS, SCAN_OP_COUNT)) {
		return;
	}

	//--------------------------------------------------------------------------
	// Extract mandatory patterns
	//--------------------------------------------------------------------------

	uint n = 0; // Number of mandatory patterns
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	uint match_clause_count = array_len(match_clauses);
	const cypher_astnode_t *patterns[match_clause_count];
	const cypher_astnode_t *mandatory_matches[match_clause_count];

	for(uint i = 0; i < match_clause_count; i++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		if(cypher_ast_match_is_optional(match_clause)) continue;
		mandatory_matches[n] = match_clauses[i];
		patterns[n] = cypher_ast_match_get_pattern(match_clause);
		n++;
	}

	// Collect the QueryGraph entities referenced in the clauses being converted.
	QueryGraph *qg = plan->query_graph;
	QueryGraph *sub_qg = QueryGraph_ExtractPatterns(qg, patterns, n);

	_ExecutionPlan_ProcessQueryGraph(plan, sub_qg, ast);

	// Build the FilterTree to model any WHERE predicates on these clauses and place ops appropriately.
	FT_FilterNode *sub_ft = AST_BuildFilterTreeFromClauses(ast, mandatory_matches, n);
	ExecutionPlan_PlaceFilterOps(plan, plan->root, NULL, sub_ft);

	// Clean up
	QueryGraph_Free(sub_qg);
	array_free(match_clauses);
}

