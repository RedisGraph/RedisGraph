/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "apply_ops_utils.h"

Record ApplyOpUtils_PullFromStream(OpBase *branch) {
	return OpBase_Consume(branch);
}

bool ApplyOpUtils_IsBoundBranch(OpBase *branch_root) {
	OpBase *arg = ExecutionPlan_LocateOp(branch_root, OPType_ARGUMENT);
	if(!arg || arg == branch_root) return true;
	return false;
}