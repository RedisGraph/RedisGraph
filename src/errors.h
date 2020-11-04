/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stddef.h>
#include "rax.h"

// Report an error in filter placement with the first unresolved entity.
void Error_InvalidFilterPlacement(rax *entitiesRax);

// Report an error when an SIValue resolves to an unhandled type.
void Error_SITypeMismatch(const char *received, const char *expected);

// Report an error on receiving an unhandled AST node type.
void Error_UnsupportedType(const char *type);
