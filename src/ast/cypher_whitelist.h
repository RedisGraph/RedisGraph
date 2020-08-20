/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

// Check if any entity in the AST is not in the RedisGraph supported whitelist.
AST_Validation CypherWhitelist_ValidateQuery(const cypher_astnode_t *root);

// Construct a whitelist of all currently-supported Cypher clauses, expressions, and operators.
void CypherWhitelist_Build(void);

