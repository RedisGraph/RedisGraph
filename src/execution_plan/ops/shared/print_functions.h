/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../op.h"
#include "../../runtimes/interpreted/ops/op.h"
#include "../../../arithmetic/algebraic_expression.h"

void TraversalToString(const RT_OpBase *op, sds *buf, AlgebraicExpression *ae);

void ScanToString(const RT_OpBase *op, sds *buf, const char *alias, const char *label);

