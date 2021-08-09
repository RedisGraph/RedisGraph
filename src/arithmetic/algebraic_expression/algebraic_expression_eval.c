/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

// forward declarations
RG_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	RG_Matrix res
);

RG_Matrix _Eval_Mul
(
	const AlgebraicExpression *exp,
	RG_Matrix res
);

RG_Matrix _Eval_Add
(
	const AlgebraicExpression *exp,
	RG_Matrix res
);

RG_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	RG_Matrix res
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
			ASSERT("transpose should have been applied prior to evaluation");
			break;

		default:
			ASSERT("Unknown algebraic expression operation" && false);
		}
		break;
	case AL_OPERAND:
		res = exp->operand.matrix;
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
	}

	return res;
}

RG_Matrix AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	RG_Matrix res
) {
	ASSERT(exp != NULL);
	return _AlgebraicExpression_Eval(exp, res);
}

