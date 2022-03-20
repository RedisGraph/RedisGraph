/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "ast.h"

// rewrite WITH/RETURN * clauses in query to project explicit identifiers,
// returning true if a rewrite has been performed
bool AST_RewriteStarProjections(cypher_parse_result_t *result);

