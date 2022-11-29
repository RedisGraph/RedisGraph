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

// concatenate labels of src or dest of ae to buf, according to the labels in ae
// if src is set, the labels of the source-node are printed, and dest otherwise
static void print_ae_labels_cat
(
	sds *buf,                 // output buffer (concatenated)
	AlgebraicExpression *ae,  // AlgebraicExpression from which to take the labels
	bool src                  // src or dest of ae
) {
	AlgebraicExpression *clone = AlgebraicExpression_Clone(ae);
	const char *alias = src ? AlgebraicExpression_Src(clone) : AlgebraicExpression_Dest(clone);
	*buf = sdscatprintf(*buf, "(%s", alias);
	AlgebraicExpression *ae_src = (AlgebraicExpression *) AlgebraicExpression_SrcOperand(clone);
	while(ae_src->type == AL_OPERAND && ae_src->operand.diagonal) {
		ae_src = AlgebraicExpression_RemoveSource(&clone);
		*buf = sdscatprintf(*buf, ":%s", ae_src->operand.label);
		if(clone) ae_src = (AlgebraicExpression *) AlgebraicExpression_SrcOperand(clone);
		else break;
	}
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
	
	print_ae_labels_cat(buf, AlgebraicExpression_Clone(ae), true);

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
	print_ae_labels_cat(buf, AlgebraicExpression_Clone(ae), false);
}

void ScanToString(const OpBase *op, sds *buf, const char *alias, const char *label) {
	*buf = sdscatprintf(*buf, "%s | ", op->name);
	_NodeToString(buf, alias, label);
}

