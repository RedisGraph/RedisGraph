/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stddef.h>
#include "rax.h"
#include "value.h"
#include "cypher-parser.h"

// Report an error in filter placement with the first unresolved entity.
void Error_InvalidFilterPlacement(rax *entitiesRax);

// Report an error when an SIValue resolves to an unhandled type.
void Error_SITypeMismatch(SIValue received, SIType expected);

// Report an error on receiving an unhandled AST node type.
void Error_UnsupportedASTNodeType(const cypher_astnode_t *node);

// Report an error on receiving an unhandled AST operator.
void Error_UnsupportedASTOperator(const cypher_operator_t *op);

// Report an error on trying to assign a complex type to a property.
void Error_InvalidPropertyValue(void);
