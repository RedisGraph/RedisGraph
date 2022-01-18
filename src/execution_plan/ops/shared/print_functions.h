/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../op.h"
#include "../../../arithmetic/algebraic_expression.h"

void TraversalToString(const OpBase *op, sds *buf, AlgebraicExpression *ae);

void ScanToString(const OpBase *op, sds *buf, const char *alias, const char *label);

