/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "optimizations_shared.h"

AST_Operator Optimizer_ReverseOp(AST_Operator op) {
	switch(op) {
	case OP_LT:
		return OP_GT;
	case OP_LE:
		return OP_GE;
	case OP_GT:
		return OP_LT;
	case OP_GE:
		return OP_LE;
	default:
		return op;
	}
}