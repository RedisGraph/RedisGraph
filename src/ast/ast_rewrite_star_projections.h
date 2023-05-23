/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "ast.h"

// collect aliases defined in a scope bounded by scope_start and scope_end
void collect_aliases_in_scope
(
	const cypher_astnode_t *root,  // the query root
	uint scope_start,              // start index of scope
	uint scope_end,                // end index of scope
	rax *identifiers               // rax to populate with identifiers
);

// rewrite WITH/RETURN * clauses in query to project explicit identifiers,
// returning true if a rewrite has been performed
bool AST_RewriteStarProjections
(
    const cypher_astnode_t *root  // root for which to rewrite star projections
);
