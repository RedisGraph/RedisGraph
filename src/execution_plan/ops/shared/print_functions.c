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

// return the labels of the src of ae splitted by ':'
static char* ae_labels_src
(
	AlgebraicExpression **ae  // AlgebraicExpression from which to take the labels
) {
	sds labels_str = sdsempty();
	AlgebraicExpression *operand_ae = (AlgebraicExpression *) AlgebraicExpression_SrcOperand(*ae);
	while(*ae && operand_ae->type == AL_OPERAND && operand_ae->operand.diagonal) {
		operand_ae = AlgebraicExpression_RemoveSource(ae);
		labels_str = sdscatprintf(labels_str, ":%s", operand_ae->operand.label);
		AlgebraicExpression_Free(operand_ae);
		if(*ae) operand_ae = (AlgebraicExpression *) AlgebraicExpression_SrcOperand(*ae);
	}

	return labels_str;
}

// return the labels of the dest of ae splitted by ':'
static char* ae_labels_dest
(
	AlgebraicExpression **ae  // AlgebraicExpression from which to take the labels
) {
	sds labels_str = sdsempty();
	AlgebraicExpression *operand_ae = (AlgebraicExpression *) AlgebraicExpression_DestOperand(*ae);
	while(*ae && operand_ae->type == AL_OPERAND && operand_ae->operand.diagonal) {
		operand_ae = AlgebraicExpression_RemoveDest(ae);
		labels_str = sdscatprintf(labels_str, ":%s", operand_ae->operand.label);
		AlgebraicExpression_Free(operand_ae);
		if(*ae) operand_ae = (AlgebraicExpression *) AlgebraicExpression_DestOperand(*ae);
	}

	return labels_str;
}

void TraversalToString(const OpBase *op, sds *buf, AlgebraicExpression *ae) {
	ASSERT(ae != NULL);

	*buf = sdscatprintf(*buf, "%s | ", op->name);
	// This edge should be printed right-to-left if the edge matrix is transposed.
	const char *edge = AlgebraicExpression_Edge(ae);
	bool transpose = (edge && AlgebraicExpression_Transposed(ae));
	AlgebraicExpression *clone = AlgebraicExpression_Clone(ae);

	// print source labels
	const char *src_alias = AlgebraicExpression_Src(clone);
	*buf = sdscatprintf(*buf, "(%s", src_alias);
	bool same_alias = !strcmp(AlgebraicExpression_Src(clone),
							  AlgebraicExpression_Dest(clone));
	char *src_labels = ae_labels_src(&clone);
	*buf = sdscatprintf(*buf, "%s", src_labels);
	if(!same_alias) sdsfree(src_labels);
	*buf = sdscatprintf(*buf, ")");

	QGEdge *e = (edge) ? QueryGraph_GetEdgeByAlias(op->plan->query_graph, edge) : NULL;
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
	// print dest labels
	if(same_alias) {
		*buf = sdscatprintf(*buf, "(%s", src_alias);
		*buf = sdscatprintf(*buf, "%s", src_labels);
		sdsfree(src_labels);
	}
	else {
		*buf = sdscatprintf(*buf, "(%s", AlgebraicExpression_Dest(clone));
		char *dest_labels = ae_labels_dest(&clone);
		*buf = sdscatprintf(*buf, "%s", dest_labels);
		sdsfree(dest_labels);
	}
	*buf = sdscatprintf(*buf, ")");

	// if clone is yet to be free'd, free it
	if(clone) AlgebraicExpression_Free(clone);
}

void ScanToString(const OpBase *op, sds *buf, const char *alias, const char *label) {
	*buf = sdscatprintf(*buf, "%s | ", op->name);
	_NodeToString(buf, alias, label);
}

