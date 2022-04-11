/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

bool AST_Rewrite(cypher_parse_result_t *result);

// rewrite WITH/RETURN * clauses in query to project explicit identifiers,
// returning true if a rewrite has been performed
bool AST_RewriteStarProjections(const cypher_astnode_t *root);

// expand fixed-length traversal patterns in query to full patterns
bool AST_ExpandFixedLengthTraversals(const cypher_astnode_t *root);
