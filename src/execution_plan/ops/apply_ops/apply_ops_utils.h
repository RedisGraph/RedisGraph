/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../op.h"
#include "../op_argument.h"
#include "../../execution_plan.h"

Record ApplyOpUtils_PullFromStream(OpBase *branch);

bool ApplyOpUtils_IsBoundBranch(OpBase *branch_root);

