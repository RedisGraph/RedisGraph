/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

// forward declarations
GrB_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
);

GrB_Matrix _Eval_Mul
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
);

GrB_Matrix _Eval_Add
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
);

static GrB_Matrix _Eval_Transpose
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	// this function is currently unused
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) == 1);

	AlgebraicExpression *child = FIRST_CHILD(exp);
	ASSERT(child->type == AL_OPERAND);
	ASSERT(child->operand.type == AL_GrB_MAT);
	GrB_Info info = GrB_transpose(res, NULL, NULL, child->operand.grb_matrix, NULL);
	ASSERT(info == GrB_SUCCESS);
	return res;
}

GrB_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp);

	// perform operation
	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_MUL:
			res = _Eval_Mul(exp, res);
			break;

		case AL_EXP_ADD:
			res = _Eval_Add(exp, res);
			break;

		case AL_EXP_TRANSPOSE:
			res = _Eval_Transpose(exp, res);
			break;

		default:
			ASSERT("Unknown algebraic expression operation" && false);
		}
		break;
	case AL_OPERAND:
		ASSERT(exp->operand.type == AL_GrB_MAT);
		res = exp->operand.grb_matrix;
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
	}

	return res;
}

void AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp != NULL);
	_AlgebraicExpression_Eval(exp, res);
}

