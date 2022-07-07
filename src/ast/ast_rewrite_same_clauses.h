/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "ast.h"

// rewrite sequence of CREATE clauses in query to 1 clause,
// returning true if a rewrite has been performed
bool AST_RewriteSameClauses(cypher_parse_result_t *result);

