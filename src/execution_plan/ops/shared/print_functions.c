/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "print_functions.h"

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

// int TraversalToString(const OpBase *op, char *buf, uint buf_len, const char *src, const char *dest,
// const char *edge, bool transpose) {
int TraversalToString(const OpBase *op, char *buf, uint buf_len, AlgebraicExpression *ae) {
	bool transpose = (ae->edge) ? _expressionContainsTranspose(ae) : false;
	switch(ae->edge == NULL) {
	case true:
		switch(transpose) {
		case true:
			return snprintf(buf, buf_len, "%s | (%s)<-(%s)", op->name, ae->src, ae->dest);
		case false:
			return snprintf(buf, buf_len, "%s | (%s)->(%s)", op->name, ae->src, ae->dest);
		}
	case false:
		switch(transpose) {
		case true:
			return snprintf(buf, buf_len, "%s | (%s)<-[%s]-(%s)", op->name, ae->src, ae->edge, ae->dest);
		case false:
			return snprintf(buf, buf_len, "%s | (%s)-[%s]->(%s)", op->name, ae->src, ae->edge, ae->dest);
		}
	}
	return 0;
}

int ScanToString(const OpBase *op, char *buf, uint buf_len, const char *alias, const char *label) {
	switch(label == NULL) {
	case true:
		return snprintf(buf, buf_len, "%s | (%s)", op->name, alias);
	case false:
		return snprintf(buf, buf_len, "%s | (%s:%s)", op->name, alias, label);
	}
	return 0;
}

