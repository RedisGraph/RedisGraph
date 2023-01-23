/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "execution_plan_construct.h"
#include "execution_plan_modify.h"
#include "../ops/ops.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../execution_plan.h"
#include "../../util/rax_extensions.h"
#include "../optimizations/optimizations.h"
#include "../../ast/ast_build_filter_tree.h"

static void _ExecutionPlan_ProcessQueryGraph
(
	ExecutionPlan *plan,
	QueryGraph *qg,
	AST *ast
) {
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// build the full FilterTree for this AST
	// so that we can order traversals properly
	FT_FilterNode *ft = AST_BuildFilterTree(ast);
	QueryGraph **connectedComponents = QueryGraph_ConnectedComponents(qg);
	plan->connected_components = connectedComponents;
	// if we have already constructed any ops
	// the plan's record map contains all variables bound at this time
	uint connectedComponentsCount = array_len(connectedComponents);
	rax *bound_vars = plan->record_map;

	// if we have multiple graph components
	// the root operation is a cartesian product
	// each chain of traversals will be a child of this op
	OpBase *cartesianProduct = NULL;
	if(connectedComponentsCount > 1) {
		cartesianProduct = NewCartesianProductOp(plan);
		ExecutionPlan_UpdateRoot(plan, cartesianProduct);
	}

	// keep track after all traversal operations along a pattern
	for(uint i = 0; i < connectedComponentsCount; i++) {
		QueryGraph *cc = connectedComponents[i];
		uint edge_count = array_len(cc->edges);
		OpBase *root = NULL; // the root of the traversal chain will be added to the ExecutionPlan
		OpBase *tail = NULL;

		if(edge_count == 0) {
			// if there are no edges in the component, we only need a node scan
			QGNode *n = cc->nodes[0];
			if(raxFind(bound_vars, (unsigned char *)n->alias, strlen(n->alias))
					!= raxNotFound) {
				continue;
			}
		}

		AlgebraicExpression **exps = AlgebraicExpression_FromQueryGraph(cc);
		uint expCount = array_len(exps);

		// Reorder exps, to the most performant arrangement of evaluation.
		orderExpressions(qg, exps, &expCount, ft, bound_vars);

		// Create the SCAN operation that will be the tail of the traversal chain.
		QGNode *src = QueryGraph_GetNodeByAlias(qg, AlgebraicExpression_Src(exps[0]));

		uint label_count = QGNode_LabelCount(src);
		if(label_count > 0) {
			AlgebraicExpression *ae_src = AlgebraicExpression_RemoveSource(&exps[0]);
			ASSERT(AlgebraicExpression_DiagonalOperand(ae_src, 0));

			const char *label = AlgebraicExpression_Label(ae_src);
			const char *alias = AlgebraicExpression_Src(ae_src);
			ASSERT(label != NULL);
			ASSERT(alias != NULL);

			int label_id = GRAPH_UNKNOWN_LABEL;
			Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(s != NULL) label_id = Schema_GetID(s);

			// resolve source node by performing label scan
			NodeScanCtx ctx = NODE_CTX_NEW(alias, label, label_id);
			root = tail = NewNodeByLabelScanOp(plan, ctx);

			// first operand has been converted into a label scan op
			AlgebraicExpression_Free(ae_src);
		} else {
			root = tail = NewAllNodeScanOp(plan, src->alias);
			// free expression source
			// in-case there are additional patterns to traverse
			if(array_len(cc->edges) == 0) {
				AlgebraicExpression_Free(
						AlgebraicExpression_RemoveSource(&exps[0]));
			}
		}

		// for each expression, build the appropriate traversal operation
		for(int j = 0; j < expCount; j++) {
			AlgebraicExpression *exp = exps[j];
			// Empty expression, already freed.
			if(AlgebraicExpression_OperandCount(exp) == 0) continue;

			QGEdge *edge = NULL;
			if(AlgebraicExpression_Edge(exp)) {
				edge =
					QueryGraph_GetEdgeByAlias(qg, AlgebraicExpression_Edge(exp));
			}

			if(edge && (QGEdge_VariableLength(edge) || !QGEdge_SingleHop(edge))) {
				// edge is part of a shortest-path
				// MATCH allShortestPaths((a)-[*..]->(b))
				// validate both edge ends are bounded
				if(QGEdge_IsShortestPath(edge)) {
					const char *src_alias  = QGNode_Alias(QGEdge_Src(edge));
					const char *dest_alias = QGNode_Alias(QGEdge_Dest(edge));
					bool src_bounded =
						raxFind(bound_vars, (unsigned char *)src_alias,
								strlen(src_alias)) != raxNotFound;
					bool dest_bounded =
						raxFind(bound_vars, (unsigned char *)dest_alias,
								strlen(dest_alias)) != raxNotFound;

					// TODO: would be great if we can perform this validation
					// at AST validation time
					if(!src_bounded || !dest_bounded) {
						ErrorCtx_SetError("Source and destination must already be resolved to call allShortestPaths");
					}
				}
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
	rax *bound_vars = NULL;

	// The root will be non-null unless the first clause is an OPTIONAL MATCH.
	if(plan->root) {
		// Collect the variables that are bound at this point.
		bound_vars = raxNew();
		// Rather than cloning the record map, collect the bound variables along with their
		// parser-generated constant strings.
		ExecutionPlan_BoundVariables(plan->root, bound_vars);
		// Collect the variable names from bound_vars to populate the Argument op we will build.
		arguments = (const char **)raxValues(bound_vars);
		raxFree(bound_vars);
	}

	// Build the new Match stream and add it to the Optional stream.
	OpBase *match_stream = ExecutionPlan_BuildOpsFromPath(plan, arguments, clause);
	array_free(arguments);
	ExecutionPlan_AddOp(optional, match_stream);

	// The root will be non-null unless the first clause is an OPTIONAL MATCH.
	if(plan->root) {
		// Create an Apply operator and make it the new root.
		OpBase *apply_op = NewApplyOp(plan);
		ExecutionPlan_UpdateRoot(plan, apply_op);

		// Create an Optional op and add it as an Apply child as a right-hand stream.
		ExecutionPlan_AddOp(apply_op, optional);
	} else {
		// If no root has been set (OPTIONAL was the first clause), set it to the Optional op.
		ExecutionPlan_UpdateRoot(plan, optional);
	}
}

void buildMatchOpTree(ExecutionPlan *plan, AST *ast, const cypher_astnode_t *clause) {
	if(cypher_ast_match_is_optional(clause)) {
		_buildOptionalMatchOps(plan, ast, clause);
		return;
	}

	// only add at most one set of traversals per plan
	// TODO Revisit and improve this logic
	if(plan->root && ExecutionPlan_LocateOpMatchingType(plan->root, SCAN_OPS, SCAN_OP_COUNT)) {
		return;
	}

	//--------------------------------------------------------------------------
	// Extract mandatory patterns
	//--------------------------------------------------------------------------

	uint mandatory_match_count = 0; // Number of mandatory patterns
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	uint match_clause_count = array_len(match_clauses);
	const cypher_astnode_t *patterns[match_clause_count];
	const cypher_astnode_t *mandatory_matches[match_clause_count];

	for(uint i = 0; i < match_clause_count; i++) {
		const cypher_astnode_t *match_clause = match_clauses[i];
		if(cypher_ast_match_is_optional(match_clause)) continue;
		mandatory_matches[mandatory_match_count] = match_clause;
		patterns[mandatory_match_count] = cypher_ast_match_get_pattern(match_clause);
		mandatory_match_count++;
	}

	// collect the QueryGraph entities referenced in the clauses being converted
	QueryGraph *qg = plan->query_graph;
	QueryGraph *sub_qg = QueryGraph_ExtractPatterns(qg, patterns,
			mandatory_match_count);

	_ExecutionPlan_ProcessQueryGraph(plan, sub_qg, ast);
	if(ErrorCtx_EncounteredError()) goto cleanup;

	// Build the FilterTree to model any WHERE predicates on these clauses and place ops appropriately.
	FT_FilterNode *sub_ft = AST_BuildFilterTreeFromClauses(ast, mandatory_matches,
														   mandatory_match_count);
	ExecutionPlan_PlaceFilterOps(plan, plan->root, NULL, sub_ft);

	// Clean up
cleanup:
	QueryGraph_Free(sub_qg);
	array_free(match_clauses);
}

