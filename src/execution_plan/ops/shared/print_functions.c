/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "print_functions.h"
#include "../../execution_plan.h"

static inline void _NodeToString(sds *buf, const char *alias, const char *label) {
	// Print a representation of a node with an optional alias and optional label.
	*buf = sdscatprintf(*buf, "(");
	if(alias) *buf = sdscatprintf(*buf, "%s", alias);
	if(label) *buf = sdscatprintf(*buf, ":%s", label);
	*buf = sdscatprintf(*buf, ")");
}

void TraversalToString(const OpBase *op, sds *buf, AlgebraicExpression *ae) {
	if(!ae) *buf = sdscatprintf(*buf, "%s", op->name);

	*buf = sdscatprintf(*buf, "%s | ", op->name);
	// This edge should be printed right-to-left if the edge matrix is transposed.
	const char *edge = AlgebraicExpression_Edge(ae);
	bool transpose = (edge && AlgebraicExpression_Transposed(ae));

	// Retrieve QueryGraph entities.
	QGNode *src = QueryGraph_GetNodeByAlias(op->plan->query_graph, AlgebraicExpression_Src(ae));
	QGNode *dest = QueryGraph_GetNodeByAlias(op->plan->query_graph,
											 AlgebraicExpression_Dest(ae));
	QGEdge *e = (edge) ? QueryGraph_GetEdgeByAlias(op->plan->query_graph, edge) : NULL;

	QGNode_ToString(src, buf);
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
	QGNode_ToString(dest, buf);
}

void ScanToString(const OpBase *op, sds *buf, const char *alias, const char *label) {
	*buf = sdscatprintf(*buf, "%s | ", op->name);
	_NodeToString(buf, alias, label);
}

