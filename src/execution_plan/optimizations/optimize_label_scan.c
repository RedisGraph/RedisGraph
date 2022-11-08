/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../../query_ctx.h"
#include "../ops/op_node_by_label_scan.h"
#include "../ops/op_conditional_traverse.h"
#include "../execution_plan_build/execution_plan_modify.h"
#include "../../arithmetic/algebraic_expression/utils.h"

// this optimization scans through each label-scan operation
// in case the node being scanned is associated with multiple labels
// e.g. MATCH (n:A:B) RETURN n
// we will prefer to first scan through the label with the least amount of nodes
// for the above example if NNZ(A) < NNZ(B) we will want to iterate over A first.
//
// in-case this optimization changed the label scanned e.g. from A to B
// it will have to patch the following conditional traversal removing B operand
// and adding A back
//
// consider MATCH (n:A:B)-[:R]->(m) RETURN m
// 
// Scan(A)
// Traverse B*R
//
// if we switch from Scan(A) to Scan(B)
// we will have to update the traversal pattern from B*R to A*R
//
// Scan(B)
// Traverse A*R

static void _optimizeLabelScan(NodeByLabelScan *scan) {
	ASSERT(scan != NULL);	

	OpBase      *op  =  (OpBase*)scan;
	Graph       *g   =  QueryCtx_GetGraph();
	QueryGraph  *qg  =  op->plan->query_graph;

	// see if scanned node has multiple labels
	const char *node_alias = scan->n.alias;
	QGNode *qn = QueryGraph_GetNodeByAlias(qg, node_alias);
	ASSERT(qn != NULL);

	// return if node has only one label
	uint label_count = QGNode_LabelCount(qn);
	ASSERT(label_count >= 1);
	if(label_count == 1) return;

	// node has multiple labels
	// find label with minimum entities
	uint64_t    min_nnz       = UINT64_MAX; // tracks min entries
	int         min_label_id  = 0;          // tracks min label ID
	const char *min_label_str = NULL;       // tracks min label name
	
	// Is scan->n.label optional
	bool optional, found;
	for (uint i = 0; i < label_count; i++){
		found = QGNode_GetLabel(qn, i) == scan->n.label;
		if (found){
			optional = qn->optional[i];
			break;
		}
	}
	ASSERT(found == true);
	// Don't perform the optimization on a plan where the label is optional.
	if (optional) return;

	for(uint i = 0; i < label_count; i++) {
		// Don't consider optional labels.
		if (QGNode_IsOptional(qn, i)) continue;
		uint64_t nnz;
		int label_id = QGNode_GetLabelID(qn, i);
		nnz = Graph_LabeledNodeCount(g, label_id);
		if(min_nnz > nnz) {
			// update minimum
			min_nnz        =  nnz;
			min_label_id   =  label_id;
			min_label_str  =  QGNode_GetLabel(qn, i);
		}
	}

	// in-case label_id is unknown at time it was created
	// check if we can resolve it now
	if(scan->n.label_id == GRAPH_UNKNOWN_LABEL) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		ASSERT(gc != NULL);

		// get schema by name
		Schema *s = GraphContext_GetSchema(gc, scan->n.label, SCHEMA_NODE);
		if(s != NULL) {
			scan->n.label_id = Schema_GetID(s);
		}
	}

	// scanned label has the minimum number of entries
	// no switching required
	if(min_label_id == scan->n.label_id) return;

	// swap current label with minimum label
	scan->n.label     =  min_label_str;
	scan->n.label_id  =  min_label_id;

	// patch following traversal, skip filters
	OpBase *parent = op->parent;
	while(OpBase_Type(parent) == OPType_FILTER) parent = parent->parent;
	ASSERT(OpBase_Type(parent) == OPType_CONDITIONAL_TRAVERSE);

	OpCondTraverse *op_traverse = (OpCondTraverse*)parent;
	AlgebraicExpression *ae = op_traverse->ae;
	AlgebraicExpression *operand;

	const char *row_domain = scan->n.alias;
	const char *column_domain = scan->n.alias;

	found = AlgebraicExpression_LocateOperand(ae, &operand, NULL,
			row_domain, column_domain, NULL, min_label_str);
	ASSERT(found == true);

	AlgebraicExpression *replacement = AlgebraicExpression_NewOperand(NULL,
			true, AlgebraicExpression_Src(operand),
			AlgebraicExpression_Dest(operand), NULL, min_label_str);

	_AlgebraicExpression_InplaceRepurpose(operand, replacement);
}

void optimizeLabelScan(ExecutionPlan *plan) {
	ASSERT(plan != NULL);

	// collect all label scan operations
	OPType t = OPType_NODE_BY_LABEL_SCAN;
	OpBase **label_scan_ops = ExecutionPlan_CollectOpsMatchingType(plan->root,
			&t ,1);

	// for each label scan operation try to optimize scanned label
	uint op_count = array_len(label_scan_ops);
	for(uint i = 0; i < op_count; i++) {
		NodeByLabelScan *label_scan = (NodeByLabelScan*)label_scan_ops[i];
		_optimizeLabelScan(label_scan);
	}
	array_free(label_scan_ops);
}

