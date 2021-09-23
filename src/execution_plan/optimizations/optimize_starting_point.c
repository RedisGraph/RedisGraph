/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../ops/op_node_by_label_scan.h"

// this optimization scans through each label-scan operation
// in case the node being scaned is associated with multiple labels
// e.g. MATCH (n:A:B) RETURN n
// we will prefer to scan through the label with the least number of nodes
// for the above example if NNZ(A) < NNZ(B) we will want to iterate over A
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

void optimizeLabelScan(NodeByLabelScan *op) {
	ASSERT(op != NULL);	

	// see if scanned node has multiple labels
	QueryGraph *qg = op->plan->query_graph;
	const char *node_alias = op->n->alias;

	QGNode *qn = QueryGraph_GetNodeByAlias(qg, alias);
	ASSERT(qn != NULL);

	// return if node has only one label
	uint label_count = QGNode_LabelCount(qn);
	if(label_count == 1) return;

	// collect NNZ for each label
	uint64_t NNZ[label_count];
	for(uint i = 0; i < label_count; i++) {
		int label = QGNode_GetLabelID(qn, i);
		NNZ[i] = Graph
	}
}

void optimizeLabelScan(ExecutionPlan *plan) {
	ASSERT(plan != NULL);

	// collect all label scan operations
	OpBase **label_scan_ops = ExecutionPlan_CollectOpsMatchingType(plan->root,
			OPType_NODE_BY_LABEL_SCAN ,1);

	uint op_count = array_len(label_scan_ops);
	for(uint i = 0; i < op_count; i++) {
		NodeByLabelScan *label_scan = (NodeByLabelScan*)label_scan_ops[i];
		_optimizeLabelScan(label_scan);
	}
}

