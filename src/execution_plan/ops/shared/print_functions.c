/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "print_functions.h"
#include "../../execution_plan.h"

static inline int _NodeToString(char *buf, uint buf_len, const char *alias, const char *label) {
	// Print a representation of a node with an optional alias and optional label.
	int offset = snprintf(buf, buf_len, "(");
	if(alias) offset += snprintf(buf + offset, buf_len - offset, "%s", alias);
	if(label) offset += snprintf(buf + offset, buf_len - offset, ":%s", label);
	offset += snprintf(buf + offset, buf_len - offset, ")");
	return offset;
}

int TraversalToString(const OpBase *op, char *buf, uint buf_len, AlgebraicExpression *ae) {
	uint offset = 0;
	if(!ae) {
		offset += snprintf(buf, buf_len, "%s", op->name);
		return offset;
	}

	offset += snprintf(buf, buf_len, "%s | ", op->name);
	// This edge should be printed right-to-left if the edge matrix is transposed.
	const char *edge = AlgebraicExpression_Edge(ae);
	bool transpose = (edge && AlgebraicExpression_Transposed(ae));

	// Retrieve QueryGraph entities.
	QGNode *src = QueryGraph_GetNodeByAlias(op->plan->query_graph, AlgebraicExpression_Source(ae));
	QGNode *dest = QueryGraph_GetNodeByAlias(op->plan->query_graph,
											 AlgebraicExpression_Destination(ae));
	QGEdge *e = (edge) ? QueryGraph_GetEdgeByAlias(op->plan->query_graph, edge) : NULL;

	offset += QGNode_ToString(src, buf + offset, buf_len - offset);
	if(e) {
		if(transpose) {
			offset += snprintf(buf + offset, buf_len - offset, "<-");
			offset += QGEdge_ToString(e, buf + offset, buf_len - offset);
			offset += snprintf(buf + offset, buf_len - offset, "-");
		} else {
			offset += snprintf(buf + offset, buf_len - offset, "-");
			offset += QGEdge_ToString(e, buf + offset, buf_len - offset);
			offset += snprintf(buf + offset, buf_len - offset, "->");
		}
	} else {
		offset += snprintf(buf + offset, buf_len - offset, "->");
	}
	offset += QGNode_ToString(dest, buf + offset, buf_len - offset);
	return offset;
}

int ScanToString(const OpBase *op, char *buf, uint buf_len, const char *alias, const char *label) {
	int offset = snprintf(buf, buf_len, "%s | ", op->name);
	buf += offset;
	buf_len -= offset;
	offset += _NodeToString(buf, buf_len, alias, label);
	return offset;
}

