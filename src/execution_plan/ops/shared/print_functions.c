/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "print_functions.h"
#include "../../execution_plan.h"

/* Given an AlgebraicExpression with a populated edge, determine whether we're traversing a
 * transposed edge matrix. The edge matrix will be either the first or second operand, and is
 * the only operand which can be transposed (as the others are label diagonals). */
static inline bool _expressionContainsTranspose(AlgebraicExpression *exp) {
	for(uint i = 0; i < exp->operand_count; i ++) {
		if(exp->operands[i].transpose) {
			return true;
		}
	}
	return false;
}

int TraversalToString(const OpBase *op, char *buf, uint buf_len, AlgebraicExpression *ae) {
	int offset = 0;
	// This edge should be printed right-to-left if the edge matrix is transposed.
	bool transpose = (ae->edge) ? _expressionContainsTranspose(ae) : false;
	offset += snprintf(buf, buf_len, "%s | ", op->name);

	// Retrieve QueryGraph entities.
	offset += QGNode_ToString(ae->src_node, buf + offset, buf_len - offset);
	if(ae->edge) {
		switch(transpose) {
		case true:
			offset += snprintf(buf + offset, buf_len - offset, "<-");
			offset += QGEdge_ToString(ae->edge, buf + offset, buf_len - offset);
			offset += snprintf(buf + offset, buf_len - offset, "-");
			break;
		case false:
			offset += snprintf(buf + offset, buf_len - offset, "-");
			offset += QGEdge_ToString(ae->edge, buf + offset, buf_len - offset);
			offset += snprintf(buf + offset, buf_len - offset, "->");
			break;
		}
	} else {
		offset += snprintf(buf + offset, buf_len - offset, "->");
	}
	offset += QGNode_ToString(ae->dest_node, buf + offset, buf_len - offset);
	return offset;
}

int ScanToString(const OpBase *op, char *buf, uint buf_len, const QGNode *n) {
	int offset = snprintf(buf, buf_len, "%s | ", op->name);
	offset += QGNode_ToString(n, buf + offset, buf_len - offset);
	return offset;
}

