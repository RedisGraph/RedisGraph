/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply_multiplexer.h"

static Record _OpApplyMultiplexed_OrApplyLogic(OpApplyMultiplexer *apply_multiplexer) {

}

static Record _OpApplyMultiplexed_AndApplyLogic(OpApplyMultiplexer *apply_multiplexer) {

}

OpBase *NewApplyMultiplexerOp(ExecutionPlan *plan, AST_Operator boolean_operator) {

	OpApplyMultiplexer *apply_multiplexer = rm_malloc(sizeof(OpApplyMultiplexer));
	apply_multiplexer->boolean_operator = boolean_operator;
	apply_multiplexer->filter = NULL;
	const char *name;
	if(boolean_operator == OR) {
		name = "Anti Semi Apply";
		op->apply_func = _OpSemiApply_AntiSemiApplyLogic;
	} else {
		name = "Semi Apply";
	}

}
