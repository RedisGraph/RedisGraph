/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../op.h"
#include "../../../arithmetic/algebraic_expression.h"

int TraversalToString(const OpBase *op, char *buf, uint buf_len, AlgebraicExpression *ae);

int ScanToString(const OpBase *op, char *buf, uint buf_len, const char *alias, const char *label);

