/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "print_functions.h"
#include "../../execution_plan.h"
#include "../op_node_by_label_scan.h"

static inline void _NodeToString(sds *buf, const char *alias, const char *label) {
	// Print a representation of a node with an optional alias and optional label.
	*buf = sdscatprintf(*buf, "(");
	if(alias) *buf = sdscatprintf(*buf, "%s", alias);
	if(label) *buf = sdscatprintf(*buf, ":%s", label);
	*buf = sdscatprintf(*buf, ")");
}

void TraversalToString(const OpBase *op, sds *buf, AlgebraicExpression *ae) {
	ASSERT(ae != NULL);

	*buf = sdscatprintf(*buf, "%s | ", op->name);
	// This edge should be printed right-to-left if the edge matrix is transposed.
	const char *edge = AlgebraicExpression_Edge(ae);
	bool transpose = (edge && AlgebraicExpression_Transposed(ae));

	// Retrieve QueryGraph entities.
	QGNode *src = QueryGraph_GetNodeByAlias(op->plan->query_graph, AlgebraicExpression_Src(ae));
	QGNode *dest = QueryGraph_GetNodeByAlias(op->plan->query_graph,
											 AlgebraicExpression_Dest(ae));
	QGEdge *e = (edge) ? QueryGraph_GetEdgeByAlias(op->plan->query_graph, edge) : NULL;

	// ignore child label if child is a label-scan.
	bool ignore_label = (op->type == OPType_CONDITIONAL_TRAVERSE || op->type == OPType_EXPAND_INTO)
						&& op->children[0]->type == OPType_NODE_BY_LABEL_SCAN;
	char *label_to_ignore = NULL;
	if(ignore_label) label_to_ignore = ((NodeByLabelScan *)op->children[0])->n.label;
	QGNode_ToString(src, buf, label_to_ignore);
	if(e) {
		if(transpose) {
			*buf = sdscatprintf(*buf, "<-");
			QGEdge_ToString(e, buf);
			*buf = sdscatprintf(*buf, "-");
		} else {
			*buf = sdscatprintf(*buf, "-");
			QGEdge_ToString(e, buf);
			*buf = sdscatprintf(*buf, "->");
		}
	} else {
		*buf = sdscatprintf(*buf, "->");
	}
	QGNode_ToString(dest, buf, label_to_ignore);
}

void ScanToString(const OpBase *op, sds *buf, const char *alias, const char *label) {
	*buf = sdscatprintf(*buf, "%s | ", op->name);
	_NodeToString(buf, alias, label);
}

