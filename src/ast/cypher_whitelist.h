/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "ast.h"

// check if any entity in the AST is not in the RedisGraph supported whitelist
AST_Validation CypherWhitelist_ValidateQuery
(
    const cypher_astnode_t *root
);
