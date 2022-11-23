/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "ast.h"

// rewrite sequence of CREATE clauses in query to 1 clause,
// returning true if a rewrite has been performed
bool AST_RewriteSameClauses
(
    cypher_parse_result_t *result
);
